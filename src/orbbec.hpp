#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

/**
 * Wraps the orbbec sdk, specifically for the point cloud format (no RGB data)
 */
class Orbbec : public RefCounted {
	GDCLASS(Orbbec, RefCounted)

protected:
	static void _bind_methods();

public:
	Orbbec() = default;
	~Orbbec() override = default;

  void print_hello() const;
  void get_sensor_from_idx(uint32_t idx);

private:
  ob::Context ob_ctx;
  std::unique_ptr<ob::Pipeline> pipeline;
  std::unique_ptr<ob::PointCloudFilter> point_cloud_filter;
  std::shared_ptr<ob::Device> device;
  std::shared_ptr<ob::FrameSet> data;
};
