#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <libobsensor/ObSensor.hpp>

using namespace godot;

/**
 * Wraps the orbbec sdk, specifically for the point cloud format (no RGB data)
 */
class Orbbec : public Node {
	GDCLASS(Orbbec, Node)

protected:
	static void _bind_methods();

public:
	Orbbec() = default;
	~Orbbec() override = default;

  void print_hello();
  void get_sensor_from_idx(uint32_t idx);

private:
  ob::Context ob_ctx;
  std::unique_ptr<ob::Pipeline> pipeline;
  std::unique_ptr<ob::PointCloudFilter> point_cloud_filter = std::make_unique<ob::PointCloudFilter>();
  std::shared_ptr<ob::Device> device;
  std::shared_ptr<ob::FrameSet> data;
  std::shared_ptr<ob::Config> config;
  void populate_device_from_idx(uint32_t idx);
  void start_stream();
};
