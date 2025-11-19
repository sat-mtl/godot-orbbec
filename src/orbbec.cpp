#include "orbbec.hpp"
#include <libobsensor/ObSensor.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

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
  emit_signal("point_cloud_frame", this);
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

void OrbbecPointCloud::_bind_methods() {
  godot::ClassDB::bind_method(D_METHOD("start_stream"), &OrbbecPointCloud::start_stream);
  godot::ClassDB::bind_method(D_METHOD("set_device_from_ip", "ip"), &OrbbecPointCloud::set_device_from_ip);
  godot::ClassDB::bind_method(D_METHOD("set_device_from_serial_number", "serial_number"), &OrbbecPointCloud::set_device_from_serial_number);
  ADD_SIGNAL(MethodInfo("point_cloud_frame", PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "new_point_cloud_frame")));
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

void OrbbecPointCloud::start_stream() {
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
    config->enableVideoStream(OB_STREAM_DEPTH, OB_WIDTH_ANY, OB_HEIGHT_ANY, OB_FPS_ANY, OB_FORMAT_ANY);
    config->enableVideoStream(OB_STREAM_COLOR, OB_WIDTH_ANY, OB_HEIGHT_ANY, OB_FPS_ANY, OB_FORMAT_RGB);
    // set frame aggregate output mode to all type frame require. therefor, the output frameset will contain all type of frames
    config->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);
    // 4.Start the pipeline with config and callback.
    pipeline->start(config, [&](std::shared_ptr<ob::FrameSet> frameSet) {
      // print_line("got frameset");
      auto frame = point_cloud_filter->process(frameSet)->as<ob::PointsFrame>();
      uint32_t width  = frame->getWidth();
      uint32_t height = frame->getHeight();

      PackedVector3Array point_cloud_data;
      point_cloud_data.resize(width*height);
      const uint8_t* data = frame->getData();
      uint32_t real_size = 0;
      OBPoint *points = reinterpret_cast<OBPoint *>(const_cast<uint8_t *>(data));
      for(uint32_t y = 0; y < height; ++y) {
        for(uint32_t x = 0; x < width; ++x) {
          int idx = y * width + x;
          const auto &pt = points[idx];
          if(std::fabs(pt.z) >= min_point_value) {
            point_cloud_data[real_size] = Vector3(pt.x/1000.0f, pt.y/1000.0f, pt.z/1000.0f);
            real_size+=1;
          }
        }
      }
      point_cloud_data.resize(real_size);
      // need to call_deferred because this code ends up being called outside of the engine thread.
      call_deferred("emit_signal", "point_cloud_frame", point_cloud_data);
    });
  }
  catch( const std::exception & ex ) {
    print_line(ex.what());
  }
}
