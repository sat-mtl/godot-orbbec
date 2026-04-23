extends MultiMeshInstance3D

var points: PackedVector3Array
var raw_points: PackedFloat32Array
var redraw:bool = false

func _ready():
	multimesh = MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D

	var pmesh := PointMesh.new()
	var material := StandardMaterial3D.new()
	material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	material.albedo_color=Color(1,0,1)
	material.point_size=10
	pmesh.material=material
	multimesh.mesh=pmesh

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if redraw:
		multimesh.instance_count = len(points)
		multimesh.buffer = raw_points
		redraw = false 

func _on_orbbec_point_cloud_frame(new_point_cloud_frame: PackedVector3Array, raw_buffer: PackedFloat32Array) -> void:
	points = new_point_cloud_frame
	raw_points = raw_buffer
	redraw = true
