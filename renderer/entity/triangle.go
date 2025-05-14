package entity

import (
	"log"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type Triangle struct {
	VAO          uint32
	NumTriangles int32
	vertices     []mgl.Vec3
	text         *v41.Text
	textPosition mgl.Vec3
}

func (entity *Triangle) GetID() uint32 {
	return entity.VAO
}

func (entity *Triangle) BindTextures() {}

func (entity *Triangle) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.75, 0}
	if font != nil {
		entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.5, 0.6, 0.3}, 0.6)
	}

	entity.vertices = []mgl.Vec3{
		{0, 0.5, 0},
		{-0.5, -0.5, 0},
		{0.5, -0.5, 0},
	}
	triangle := []float32{
		0, 0.5, 0, // top
		-0.5, -0.5, 0, // left
		0.5, -0.5, 0, // right
	}
	entity.VAO = createVAO(triangle)
	log.Printf("created triangle at ID: %d", entity.VAO)
}

func (entity *Triangle) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, int32(9/3))
	gl.BindVertexArray(0)
}

func (entity *Triangle) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *Triangle) GetName() string {
	return "Triangle"
}

func (entity *Triangle) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}

	// localToWorld := cam.GetModel(camTransform)
	// v1 := entity.vertices[0]
	// v2 := entity.vertices[1]
	// v3 := entity.vertices[2]

	// pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	// pv2 := localToWorld.Mul4x1(v2.Vec4(1)).Vec3()
	// pv3 := localToWorld.Mul4x1(v3.Vec4(1)).Vec3()

	// if tempT, n, intersects := utils.RayPlane(pv1, pv2, pv3, ray); intersects {
	// 	p := ray.Eye.Add(ray.Direction.Mul(float32(tempT)))
	// 	return utils.RayTriangle(pv1, pv2, pv3, p, *n, tempT)
	// }
	return 0, false
}
