#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/rid.hpp"
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
 * unused for now.
 */
enum DepthCamType {
  ORBBEC_FEMTO_MEGA
};

/**
 * Base orbbec device class. Manages connection to devices.
 */
class OrbbecPointCloudBase: public Node {
 GDCLASS(OrbbecPointCloudBase, Node)
 public:
  /**
   * we only support orbbec femto megas for now.
   */
  DepthCamType device_type = DepthCamType::ORBBEC_FEMTO_MEGA;
  void get_sensor_from_idx(uint32_t idx);
  virtual void start_stream(int xres, int yres, int framerate) {};
  void stop_stream();
  PackedStringArray get_device_stream_formats();
  void set_device_from_ip(String ip);
  void set_device_from_serial_number(String serial_number);
  static void _bind_methods();

 protected:
  std::unique_ptr<ob::Pipeline> pipeline;
  std::unique_ptr<ob::PointCloudFilter> point_cloud_filter = std::make_unique<ob::PointCloudFilter>();
  std::shared_ptr<ob::Device> device;
  std::shared_ptr<ob::FrameSet> data;
  std::shared_ptr<ob::Config> config;
  using predicate_type = std::function<bool(std::shared_ptr<ob::DeviceList>, uint32_t)>;
  void set_device_from_predicate(predicate_type predicate);
  void populate_device_from_idx(uint32_t idx);
};

/**
 * Node that gets pointcloud data from an orbbec camera. Use the OrbbecDevices node to query available nodes.
 * Preprocess the data and passes it as PackedFloat32Array and PackedVector3Array. The preprocessing is done on
 * the CPU and can be slow.
 */
class OrbbecPointCloud : public OrbbecPointCloudBase{
  GDCLASS(OrbbecPointCloud, OrbbecPointCloudBase)

protected:
  static void _bind_methods();

public:
  OrbbecPointCloud();
  ~OrbbecPointCloud() override = default;
  void set_thinning(float thinning);
  float get_thinning();
  void start_stream(int xres, int yres, int framerate) override;
private:
  float thinning = 0.5;
  /**
   * used to create raw multimesh buffers from point cloud data
   */
  const Transform3D identity_transform{};
  static constexpr size_t thinning_mask_size = 100000;
  static constexpr size_t floats_per_raw_point = 12;
  std::array<float, thinning_mask_size> thinning_mask;

};

constexpr uint32_t floats_per_points = 3;
constexpr uint32_t bytes_per_float = 4;

/**
 * Upload point cloud data to a GPU SSBO without any preprocessing. You can use this assigning the RID of
 * the gpu buffer to a compute shader and call that compute shader from the point_cloud_frame signal.
 * You are responsible for the thinning and preprocessing.
 *
 * Make sure to call
 */
class OrbbecPointCloudGPU : public OrbbecPointCloudBase {
  GDCLASS(OrbbecPointCloudGPU, OrbbecPointCloudBase)

protected:
  static void _bind_methods();

public:
  ~OrbbecPointCloudGPU() override = default;
  /**
   * Allocate a new RID for the GPU buffer on _enter_tree
   */
  void _enter_tree() override;
  /**
   * Frees the GPU buffer on _exit_tree
   */
  void _exit_tree() override;
  /**
   * returns the current GPU buffer's RID.
   * Do not store the RID, call this every time. if your node exists and reenters the tree
   * the *RID* will have changed and its not like copying a RID is expensive.
   */
  RID get_point_buffer_rid();
  /**
   * gives the number of points that are uploaded to the gpu in the point_cloud SSBO.
   */
  uint32_t get_num_points();
  void set_rendering_device(RenderingDevice* rd);
  RenderingDevice* get_rendering_device();
  void start_stream(int xres, int yres, int framerate) override;
  void update_point_cloud_buffer();
private:
  void allocate_point_cloud_buffer();
  RID point_buffer{};
  PackedByteArray point_bytes{};
  RenderingDevice* rd = nullptr;
  float thinning = 0.5;
  uint32_t xres = 1024;
  uint32_t yres = 1024;
  /**
   * used to create raw multimesh buffers from point cloud data
   */
  const Transform3D identity_transform{};
  static constexpr size_t thinning_mask_size = 100000;
  static constexpr size_t floats_per_raw_point = 12;
  std::array<float, thinning_mask_size> thinning_mask;
};
