package entity

import (
	"log"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type Quad struct {
	VAO          uint32
	NumTriangles int32
	vertices     []mgl.Vec3
}

func (entity *Quad) BindTextures() {}

func (entity *Quad) Init(font *v41.Font, text string) {
	entity.vertices = []mgl.Vec3{
		{-0.5, 0.5, 0},  // top-left
		{-0.5, -0.5, 0}, // bottom-left
		{0.5, -0.5, 0},  // bottom-right
		{0.5, 0.5, 0},   // top-right
	}
	points := []float32{
		// bottom-left tri
		-0.5, 0.5, 0, 0, 1, 0, // top-left
		-0.5, -0.5, 0, 0, 1, 0, // bottom-left
		0.5, -0.5, 0, 0, 1, 0, // bottom-right
		// top-right tri
		-0.5, 0.5, 0, 0, 1, 0, // top-left
		0.5, -0.5, 0, 0, 1, 0, // bottom-right
		0.5, 0.5, 0, 0, 1, 0, // top-right
	}
	entity.NumTriangles = 6
	entity.VAO = createVAOWithNormals(points)
	log.Printf("created quad at VAO: %d", entity.VAO)
}

func (entity *Quad) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *Quad) GetName() string {
	return "Quad"
}

func (entity *Quad) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)

	v1 := entity.vertices[0]
	v2 := entity.vertices[1]
	v3 := entity.vertices[2]
	v4 := entity.vertices[3]

	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	pv2 := localToWorld.Mul4x1(v2.Vec4(1)).Vec3()
	pv3 := localToWorld.Mul4x1(v3.Vec4(1)).Vec3()
	pv4 := localToWorld.Mul4x1(v4.Vec4(1)).Vec3()

	if tempT, n, intersects := utils.RayPlane(pv1, pv2, pv3, ray); intersects {
		p := ray.Eye.Add(ray.Direction.Mul(float32(tempT)))
		return utils.RayQuad(pv1, pv2, pv3, pv4, p, *n, tempT)
	}
	return -1, false
}
