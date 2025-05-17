package entity

import (
	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/shader"
)

type Entity interface {
	GetName() string
	Init(*v41.Font, string)
	Draw()
	BindTextures()
	Intersect(*camera.Camera, *camera.Transform3D, *camera.Ray, bool) (float64, bool)
}

type GroupedEntity interface {
	Entity
	DrawMultiple(deltaT float32, transform *camera.Transform3D, program *shader.Program, cam *camera.Camera, lightPos *mgl.Vec3, cameraPos *mgl.Vec3, color mgl.Vec3, onClick bool, onClickColor mgl.Vec3)
}

// A Tri is composed of points p1, p2, p3 in CCW order
type Tri struct {
	P *[]mgl.Vec3
}

// A NTri is composed of points p1, p2, p3 in CCW order with vertex normals N
type NTri struct {
	P *[]mgl.Vec3
	N *[]mgl.Vec3
}

func (triangle *Tri) GetCentroid() *mgl.Vec3 {
	if triangle == nil {
		return nil
	}
	return &mgl.Vec3{
		((*triangle.P)[0].X() + (*triangle.P)[1].X() + (*triangle.P)[2].X()) / 3.0,
		((*triangle.P)[0].Y() + (*triangle.P)[1].Y() + (*triangle.P)[2].Y()) / 3.0,
		((*triangle.P)[0].Z() + (*triangle.P)[1].Z() + (*triangle.P)[2].Z()) / 3.0,
	}
}

func ConvertNTrisToTris(inputTriangles []*NTri) []*Tri {
	out := []*Tri{}
	for _, triangle := range inputTriangles {
		out = append(out, &Tri{
			P: triangle.P,
		})
	}
	return out
}

func (t *NTri) GetPoints() []float32 {
	points := []float32{}
	for i := 0; i < 3; i++ {
		points = append(points, (*t.P)[i].X())
		points = append(points, (*t.P)[i].Y())
		points = append(points, (*t.P)[i].Z())
		points = append(points, (*t.N)[i].X())
		points = append(points, (*t.N)[i].Y())
		points = append(points, (*t.N)[i].Z())
	}
	return points
}

// A TTri is composed of points p1, p2, p3 in CCW order and T1,T2 representing the texture coordinate of p1 and p3
type TTri struct {
	Tri
	TextureCoordinates *[]mgl.Vec2
}

// Get float32 points from a Tri with normal facing outwards from the CCW drawn Tri
func (t *Tri) GetPoints() []float32 {
	points := []float32{}
	normal := t.GetNormal()
	for i := 0; i < 3; i++ {
		points = append(points, (*t.P)[i].X())
		points = append(points, (*t.P)[i].Y())
		points = append(points, (*t.P)[i].Z())
		points = append(points, normal.X())
		points = append(points, normal.Y())
		points = append(points, normal.Z())
	}
	return points
}

func (t *TTri) GetPoints() []float32 {
	points := []float32{}
	normal := t.GetNormal()
	for i := 0; i < 3; i++ {
		points = append(points, (*t.P)[i].X())
		points = append(points, (*t.P)[i].Y())
		points = append(points, (*t.P)[i].Z())
		points = append(points, normal.X())
		points = append(points, normal.Y())
		points = append(points, normal.Z())
		points = append(points, (*t.TextureCoordinates)[i].X())
		points = append(points, (*t.TextureCoordinates)[i].Y())
	}
	return points
}

// Get the normal from a Tri with points p1, p2, p3 by obtaining the cross product between line segments p2p3 X p2p1
func (t *Tri) GetNormal() mgl.Vec3 {
	r1 := (*t.P)[2].Sub((*t.P)[1])
	r2 := (*t.P)[0].Sub((*t.P)[1])
	return r1.Cross(r2).Normalize()
}

// creates a border where
// v0,v1,v2,v3 form the outer quad
// vi0, vi1, vi2, vi3 form the inner quad
// in CCW direction
func CreateTriPointsFromBorder(v0, v1, v2, v3, vi0, vi1, vi2, vi3 mgl.Vec3) []*Tri {
	tris := make([]*Tri, 0)
	tris1 := CreateTriPointsFromQuad(vi0, v0, v1, vi1)
	tris2 := CreateTriPointsFromQuad(v3, v0, vi0, vi3)
	tris3 := CreateTriPointsFromQuad(v3, vi3, vi2, v2)
	tris4 := CreateTriPointsFromQuad(vi2, vi1, v1, v2)
	tris = append(tris, tris1...)
	tris = append(tris, tris2...)
	tris = append(tris, tris3...)
	tris = append(tris, tris4...)
	return tris
}

func CreateTTriPointsFromQuad(v0, v1, v2, v3 mgl.Vec3, t00 int) []*TTri {
	triPoints := make([]mgl.Vec3, 3)
	triPoints[0] = v0
	triPoints[1] = v1
	triPoints[2] = v2

	vt00 := mgl.Vec2{0, 0}
	vt01 := mgl.Vec2{0, 1}
	vt10 := mgl.Vec2{1, 0}
	vt11 := mgl.Vec2{1, 1}
	texCoordinates := make([]mgl.Vec2, 3)

	//  00      01
	// (v3)    (v2)
	// (v0)    (v1)
	//  10     11

	if t00 == 0 {
		texCoordinates[0] = vt00
		texCoordinates[1] = vt10
		texCoordinates[2] = vt11
	} else if t00 == 1 {
		texCoordinates[0] = vt01
		texCoordinates[1] = vt00
		texCoordinates[2] = vt10
	} else if t00 == 2 {
		texCoordinates[0] = vt11
		texCoordinates[1] = vt01
		texCoordinates[2] = vt00
	} else if t00 == 3 {
		texCoordinates[0] = vt10
		texCoordinates[1] = vt11
		texCoordinates[2] = vt01
	}

	texCoordinates2 := make([]mgl.Vec2, 3)

	triPoints2 := make([]mgl.Vec3, 3)
	triPoints2[0] = v0
	triPoints2[1] = v2
	triPoints2[2] = v3

	//   00     01
	// (v3)    (v2)

	// (v0)    (v1)
	// 10     11

	if t00 == 0 {
		texCoordinates2[0] = vt00
		texCoordinates2[1] = vt11
		texCoordinates2[2] = vt01
	} else if t00 == 1 {
		texCoordinates2[0] = vt01
		texCoordinates2[1] = vt10
		texCoordinates2[2] = vt11
	} else if t00 == 2 {
		texCoordinates2[0] = vt11
		texCoordinates2[1] = vt00
		texCoordinates2[2] = vt10
	} else if t00 == 3 {
		texCoordinates2[0] = vt10
		texCoordinates2[1] = vt01
		texCoordinates2[2] = vt00
	}

	return []*TTri{
		{Tri: Tri{P: &triPoints}, TextureCoordinates: &texCoordinates},
		{Tri: Tri{P: &triPoints2}, TextureCoordinates: &texCoordinates2},
	}
}

func CreateTriPointsFromQuad(v0, v1, v2, v3 mgl.Vec3) []*Tri {
	triPoints := make([]mgl.Vec3, 3)
	triPoints[0] = v0
	triPoints[1] = v1
	triPoints[2] = v2

	triPoints2 := make([]mgl.Vec3, 3)
	triPoints2[0] = v0
	triPoints2[1] = v2
	triPoints2[2] = v3
	return []*Tri{{P: &triPoints}, {P: &triPoints2}}
}

func CreateTriPoints(v0, v1, v2 mgl.Vec3) *Tri {
	triPoints := make([]mgl.Vec3, 3)
	triPoints[0] = v0
	triPoints[1] = v1
	triPoints[2] = v2
	return &Tri{P: &triPoints}
}

func createVAO(points []float32) uint32 {
	var VBO, VAO uint32
	// Create VAO
	gl.GenVertexArrays(1, &VAO)
	gl.BindVertexArray(VAO)

	// Create VBO
	gl.GenBuffers(1, &VBO)
	gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
	gl.BufferData(gl.ARRAY_BUFFER, 4*len(points), gl.Ptr(points), gl.STATIC_DRAW)

	// Position
	gl.EnableVertexAttribArray(0)
	gl.VertexAttribPointer(0, 3, gl.FLOAT, false, 0, nil)

	return VAO
}

func createVAOWithNormals(points []float32) uint32 {
	var VBO, VAO uint32
	// Create VAO
	gl.GenVertexArrays(1, &VAO)
	gl.BindVertexArray(VAO)

	// Create VBO
	gl.GenBuffers(1, &VBO)
	gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
	gl.BufferData(gl.ARRAY_BUFFER, 4*len(points), gl.Ptr(points), gl.STATIC_DRAW)

	// Position
	gl.VertexAttribPointerWithOffset(0, 3, gl.FLOAT, false, 6*4, uintptr(0))
	gl.EnableVertexAttribArray(0)

	// Normal
	gl.VertexAttribPointerWithOffset(1, 3, gl.FLOAT, false, 6*4, uintptr(3*4))
	gl.EnableVertexAttribArray(1)

	return VAO
}

func createVAOWithNormalsAndTextures(points []float32) uint32 {
	var VBO, VAO uint32
	// Create VAO
	gl.GenVertexArrays(1, &VAO)
	gl.BindVertexArray(VAO)

	// Create VBO
	gl.GenBuffers(1, &VBO)
	gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
	gl.BufferData(gl.ARRAY_BUFFER, 4*len(points), gl.Ptr(points), gl.STATIC_DRAW)

	// Position
	gl.VertexAttribPointerWithOffset(0, 3, gl.FLOAT, false, (3+3+2)*4, uintptr(0))
	gl.EnableVertexAttribArray(0)

	// Normal
	gl.VertexAttribPointerWithOffset(1, 3, gl.FLOAT, false, (3+3+2)*4, uintptr(3*4))
	gl.EnableVertexAttribArray(1)

	// Texture
	gl.VertexAttribPointerWithOffset(2, 2, gl.FLOAT, false, (3+3+2)*4, uintptr((3+3)*4))
	gl.EnableVertexAttribArray(2)
	return VAO
}

func createVAOWithEBO(points []float32, indices []uint32) uint32 {
	var VBO, VAO, EBO uint32
	// Create VAO
	gl.GenVertexArrays(1, &VAO)
	gl.BindVertexArray(VAO)

	// Create VBO
	gl.GenBuffers(1, &VBO)
	gl.BindBuffer(gl.ARRAY_BUFFER, VBO)
	gl.BufferData(gl.ARRAY_BUFFER, 4*len(points), gl.Ptr(points), gl.STATIC_DRAW)

	// Create EBO
	gl.GenBuffers(1, &EBO)
	gl.BindBuffer(gl.ELEMENT_ARRAY_BUFFER, EBO)
	gl.BufferData(gl.ELEMENT_ARRAY_BUFFER, 4*len(indices), gl.Ptr(indices), gl.STATIC_DRAW)

	// Position
	gl.VertexAttribPointer(0, 3, gl.FLOAT, false, 0, nil)
	gl.EnableVertexAttribArray(0)

	return VAO
}
