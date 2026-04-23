extends Node

func _ready() -> void:
	var ips = %OrbbecDevices.get_devices_ips()
	ips.sort()
	print(ips)
	%OrbbecPointCloud.set_device_from_ip(ips[0])
	var res_avail = %OrbbecPointCloud.get_device_stream_formats()
	print(res_avail)
	
	## select 512x512 resolution at 30 FPS 
	var res_select = res_avail[1]
	var xyres = res_select.split(" ")[0].split("x")

	%OrbbecPointCloud.start_stream(int(xyres[0]), int(xyres[1]), 30)
