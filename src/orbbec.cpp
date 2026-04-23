#include "orbbec.hpp"
#include <libobsensor/ObSensor.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <stdlib.h>

void OrbbecDevices::_bind_methods() {
  godot::ClassDB::bind_method(D_METHOD("refresh_device_list"), &OrbbecDevices::refresh_device_list);
  godot::ClassDB::bind_method(D_METHOD("get_devices_ips"), &OrbbecDevices::get_devices_ips);
  godot::ClassDB::bind_method(D_METHOD("get_devices_serial_numbers"), &OrbbecDevices::get_devices_serial_numbers);
  ADD_SIGNAL(MethodInfo("device_list_refreshed", PropertyInfo(Variant::OBJECT, "node")));
}

void OrbbecDevices::refresh_device_list() {
  devices = ob_ctx.queryDeviceList();
}

void OrbbecDevices::_ready() {
  refresh_device_list();
}

PackedStringArray OrbbecDevices::get_devices_ips() {
  PackedStringArray ips;
  if (!devices) {
    return ips;
  }
  for (uint32_t i = 0; i < devices->getCount(); ++i) {
    ips.push_back(devices->getIpAddress(i));
  }
  return ips;
}

PackedStringArray OrbbecDevices::get_devices_serial_numbers() {
  PackedStringArray serials;
  if (!devices) {
    return serials;
  }
  for (uint32_t i = 0; i < devices->getCount(); ++i) {
    serials.push_back(devices->getSerialNumber(i));
  }
  return serials;
};

OrbbecPointCloud::OrbbecPointCloud() {
  // create a random thinning mask.
  for (int i=0; i < thinning_mask_size; ++i) {
    thinning_mask[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }
}

void OrbbecPointCloud::set_thinning(float thin) {
  thinning = thin;
}

float OrbbecPointCloud::get_thinning() {
  return thinning;
}

PackedStringArray OrbbecPointCloud::get_device_stream_formats() {
  // TODO: make this dynamic when we support more camera types.
  // I tried to make this a static const inline and it made godot crash everytime.
  // This will have to be good enough.
  return PackedStringArray {"1024x1024 (WFOV Unbinned)", "512x512 (WFOV Binned)", "640x576 (NFOV Unbinned)", "320x288 (NFOV Binned)"};
}

void OrbbecPointCloud::_bind_methods() {
  godot::ClassDB::bind_method(D_METHOD("start_stream", "xres", "yres", "framerate"), &OrbbecPointCloud::start_stream);
  godot::ClassDB::bind_method(D_METHOD("stop_stream"), &OrbbecPointCloud::stop_stream);
  godot::ClassDB::bind_method(D_METHOD("set_device_from_ip", "ip"), &OrbbecPointCloud::set_device_from_ip);
  godot::ClassDB::bind_method(D_METHOD("set_device_from_serial_number", "serial_number"), &OrbbecPointCloud::set_device_from_serial_number);
  godot::ClassDB::bind_method(D_METHOD("get_thinning"), &OrbbecPointCloud::get_thinning);
  godot::ClassDB::bind_method(D_METHOD("set_thinning", "p_thinning"), &OrbbecPointCloud::set_thinning);
  godot::ClassDB::bind_method(D_METHOD("get_device_stream_formats"), &OrbbecPointCloud::get_device_stream_formats);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "thinning"), "set_thinning", "get_thinning");
  ADD_SIGNAL(MethodInfo("point_cloud_frame", PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "points"), PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "raw_buffer")));
}

void OrbbecPointCloud::set_device_from_predicate(predicate_type predicate) {
  std::shared_ptr<ob::DeviceList> devices = ob_ctx.queryDeviceList();
  for (uint32_t i = 0; i < devices->getCount(); ++i) {
    if (predicate(devices, i)) {
      try {
        device = devices->getDevice(i);
      } catch (std::exception & ex) {
        // TODO: throw a real godot error here.
        print_line("couldn't open device: ");
        print_line(ex.what());
      }
      return;
    }
  }
}

void OrbbecPointCloud::set_device_from_ip(String ip) {
  set_device_from_predicate([&](std::shared_ptr<ob::DeviceList> devices, uint32_t idx) {
    return devices->getIpAddress(idx) == ip;
  });
}

void OrbbecPointCloud::set_device_from_serial_number(String serial_number) {
  set_device_from_predicate([&](std::shared_ptr<ob::DeviceList> devices, uint32_t idx) {
    return devices->getSerialNumber(idx) == serial_number;
  });
}

void OrbbecPointCloud::stop_stream() {
  // deleting the pipeline makes the stream stop.
  pipeline.reset();
  // probably don't need to do this but it can't hurt too much
  config.reset();
}

void OrbbecPointCloud::start_stream(int xres, int yres, int framerate) {
  if (!device) {
    print_line("Not starting stream, please set a device.");
    return;
  }
  try {
    // stolen from the savePointCloudToPly function of the orbbec sdk. I assume this value filters out irrelevant points ?
    // populate_device_from_idx(idx);
    constexpr auto min_point_value = 1e-6f;
    point_cloud_filter->setCreatePointFormat(OB_FORMAT_POINT);
    pipeline = std::make_unique<ob::Pipeline>(device);
    pipeline->enableFrameSync();
    config = std::make_shared<ob::Config>();
    config->enableVideoStream(OB_STREAM_DEPTH, xres, yres, framerate, OB_FORMAT_ANY);
    // set frame aggregate output mode to all type frame require. therefor, the output frameset will contain all type of frames
    config->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);
    // 4.Start the pipeline with config and callback.
    pipeline->start(config, [&, this](std::shared_ptr<ob::FrameSet> frameSet) {
      auto frame = point_cloud_filter->process(frameSet)->as<ob::PointsFrame>();
      uint32_t width  = frame->getWidth();
      uint32_t height = frame->getHeight();

      PackedVector3Array point_cloud_data;
      PackedFloat32Array point_cloud_raw_buffer;
      point_cloud_data.resize(width*height);
      point_cloud_raw_buffer.resize(width*height*floats_per_raw_point);
      const uint8_t* data = frame->getData();
      uint32_t real_size = 0;
      OBPoint *points = reinterpret_cast<OBPoint *>(const_cast<uint8_t *>(data));
      const auto& basis = identity_transform.basis.rows;
      for(uint32_t y = 0; y < height; ++y) {
        for(uint32_t x = 0; x < width; ++x) {
          int idx = y * width + x;
          const auto &pt = points[idx];
          if (thinning_mask[idx%thinning_mask_size] > thinning && std::fabs(pt.z) >= min_point_value) {
            float x = pt.x/1000.0f;
            float y = pt.y/1000.0f;
            float z = pt.z/1000.0f;

            point_cloud_data[real_size] = Vector3(x, y, z);

            // doc for this order is here : https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer
            point_cloud_raw_buffer[real_size * floats_per_raw_point] = basis[0][0];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 1] = basis[1][0];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 2] = basis[2][0];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 3] = x;
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 4] = basis[0][1];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 5] = basis[1][1];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 6] = basis[2][1];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 7] = y;
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 8] = basis[0][2];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 9] = basis[1][2];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 10] = basis[2][2];
            point_cloud_raw_buffer[real_size * floats_per_raw_point + 11] = z;

            real_size+=1;
          }
        }
      }
      point_cloud_data.resize(real_size);
      point_cloud_raw_buffer.resize(real_size*floats_per_raw_point);
      // need to call_deferred because this code ends up being called outside of the engine thread.
      call_deferred("emit_signal", "point_cloud_frame", point_cloud_data, point_cloud_raw_buffer);
    });
  }
  catch( const std::exception & ex ) {
    print_line(ex.what());
  }
}
