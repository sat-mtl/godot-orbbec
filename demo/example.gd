extends Node

func _ready() -> void:
	var ips = %OrbbecDevices.get_devices_ips()
	ips.sort()
	print(ips)
	%OrbbecPointCloud.set_device_from_ip(ips[0])
	%OrbbecPointCloud.start_stream()
