#include "orbbec.hpp"
#include <libobsensor/ObSensor.hpp>

void Orbbec::_bind_methods() {
  godot::ClassDB::bind_method(D_METHOD("print_hello"), &Orbbec::print_hello);
}

void Orbbec::print_hello() const {
  ob::Context context;
  print_line("hello");
}

void Orbbec::populate_device_from_idx(uint32_t idx) {
  std::shared_ptr<DeviceList> devices = ob_ctx.queryDeviceList();
  device = devices->getDevice(idx);
}

void Orbbec::start_stream() {
  point_cloud_filter->setCreatePointFormat(OB_FORMAT_POINT);
  pipeline = std::make_unique<ob::Pipeline>();


}
