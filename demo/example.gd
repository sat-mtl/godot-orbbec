extends Node

func _ready() -> void:
	print(%OrbbecDevices.get_devices_ips())
	%OrbbecPointCloud.set_device_from_ip("10.10.30.182")
	%OrbbecPointCloud.start_stream()
