#include "orbbec.hpp"
#include <libobsensor/ObSensor.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

void Orbbec::_bind_methods() {
  godot::ClassDB::bind_method(D_METHOD("print_hello"), &Orbbec::print_hello);
  ADD_SIGNAL(MethodInfo("point_cloud_frame", PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "new_point_cloud_frame")));
}

void Orbbec::print_hello() {
  std::shared_ptr<ob::DeviceList> devices = ob_ctx.queryDeviceList();
  print_line(devices->getCount());
  print_line("aaaaa");
  print_line(" devices");
  for (uint32_t i = 0; i < devices->getCount(); ++i) {
    print_line(devices->getSerialNumber(i));
    print_line(devices->getConnectionType(i));
    print_line(devices->getIpAddress(i));
    if (std::string(devices->getIpAddress(i)) == "10.10.30.182") {
      device = devices->getDevice(i);
    }
  }
  start_stream();
}

void Orbbec::populate_device_from_idx(uint32_t idx) {
  std::shared_ptr<ob::DeviceList> devices = ob_ctx.queryDeviceList();
  print_line(devices->getSerialNumber(idx));
  print_line(devices->getConnectionType(idx));
  print_line(devices->getIpAddress(idx));
}

void Orbbec::start_stream() {
  print_line("start_stream");
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
      // print_line("emit");
      // emit_signal("point_cloud_frame", point_cloud_data);
      call_deferred("emit_signal", "point_cloud_frame", point_cloud_data);
    });
  }
  catch( const std::exception & ex ) {
    print_line(ex.what());
  }

}
