#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/char_string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include <libobsensor/ObSensor.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

using namespace godot;


/**
 * ob_ctx is used for everything related to the orbbec sdk and can be used to do things like
 * multi device synchronization so I think it makes sense to have it be a global variable.
 */
inline ob::Context ob_ctx;

class OrbbecDevices : public Node {
  GDCLASS(OrbbecDevices, Node)

protected:
  static void _bind_methods();

public:
  OrbbecDevices() = default;
  ~OrbbecDevices() override = default;

  /**
   * do a refresh device list on ready.
   */
  void _ready() override;

  /**
   * asks the orbbec SDK what devices are available and store them in `devices`.
   */
  void refresh_device_list();

  PackedStringArray get_devices_ips();
  PackedStringArray get_devices_serial_numbers();

private:
  std::shared_ptr<ob::DeviceList> devices;
  void populate_device_from_idx(uint32_t idx);
};

/**
 * Node that gets pointcloud data from an orbbec camera. Use the OrbbecDevices node to query available nodes.
 */
class OrbbecPointCloud : public Node {
  GDCLASS(OrbbecPointCloud, Node)

protected:
  static void _bind_methods();

public:
  /**
   * unused for now.
   */
  enum DepthCamType {
    ORBBEC_FEMTO_MEGA
  };
  OrbbecPointCloud();
  ~OrbbecPointCloud() override = default;
  void print_hello();
  void get_sensor_from_idx(uint32_t idx);
  void start_stream(int xres, int yres, int framerate);
  void stop_stream();
  PackedStringArray get_device_stream_formats();
  void set_device_from_ip(String ip);
  void set_device_from_serial_number(String serial_number);
  void set_thinning(float thinning);
  float get_thinning();
private:
  float thinning = 0.5;
  /**
   * we only support orbbec femto megas for now.
   */
  DepthCamType device_type = DepthCamType::ORBBEC_FEMTO_MEGA;
  /**
   * used to create raw multimesh buffers from point cloud data
   */
  const Transform3D identity_transform{};
  static constexpr size_t thinning_mask_size = 100000;
  static constexpr size_t floats_per_raw_point = 12;
  std::array<float, thinning_mask_size> thinning_mask;
  std::unique_ptr<ob::Pipeline> pipeline;
  std::unique_ptr<ob::PointCloudFilter> point_cloud_filter = std::make_unique<ob::PointCloudFilter>();
  std::shared_ptr<ob::Device> device;
  std::shared_ptr<ob::FrameSet> data;
  std::shared_ptr<ob::Config> config;
  using predicate_type = std::function<bool(std::shared_ptr<ob::DeviceList>, uint32_t)>;
  void set_device_from_predicate(predicate_type predicate);

  void populate_device_from_idx(uint32_t idx);
};
