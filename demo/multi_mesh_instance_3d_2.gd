extends MultiMeshInstance3D
@export

var file: String = "res://test_point_cloud.ply.tres"
@export
var color: Color
# Called when the node enters the scene tree for the first time.
var points:Array[Vector3]
func _ready():
	var f = FileAccess.open(file, FileAccess.READ)
	var content = f.get_as_text()
	var lines = content.split("\n").slice(7, -1)
	
	for line in lines:
		var coords = line.replace("\r", "").strip_edges().split(" ")
		var vec = Vector3()
		for i in range(3):
			vec[i] = float(coords[i])
		# sample file has a ton of point at 0.0, ignore them
		if vec.length() != 0:
			points.append(vec)
	draw()
	

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass
	
func draw():
	multimesh = MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D
		
	var pmesh := PointMesh.new()
	var material := StandardMaterial3D.new()
	material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	material.albedo_color=color
	material.point_size=1
	pmesh.material=material    
	
	multimesh.mesh=pmesh
	multimesh.instance_count = len(points)
	
	for i in multimesh.instance_count:
		multimesh.set_instance_transform(i, Transform3D(Basis(), points[i]))    
