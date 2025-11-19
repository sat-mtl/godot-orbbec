extends MultiMeshInstance3D

var points: PackedVector3Array
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
		# TODO: there is a way to set a whole PackedFloat32Array as the multimesh's buffer.
		# This may be faster than iterating and creating a transform3D.
		# see https://docs.godotengine.org/en/stable/classes/class_renderingserver.html#class-renderingserver-method-multimesh-set-buffer
		# it references rendering server but it works in gdscript too. We could even generate the packed array in C++ directly.
		for i in multimesh.instance_count:
			multimesh.set_instance_transform(i, Transform3D(Basis(), points[i]))   
		redraw = false 

func _on_orbbec_point_cloud_frame(new_point_cloud_frame: PackedVector3Array) -> void:
	points = new_point_cloud_frame
	redraw = true
