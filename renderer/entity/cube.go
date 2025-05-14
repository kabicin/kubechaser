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

type Cube struct {
	VAO          uint32
	NumTriangles int32
	vertices     []mgl.Vec3
	triangles    []*Tri
	text         *v41.Text
	textPosition mgl.Vec3
	BoundingBox  *AABB
}

func (entity *Cube) BindTextures() {}

func (entity *Cube) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.8, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.35546875, 0.56640625, 0.23046875}, 0.5)

	frontTopLeft := mgl.Vec3{-0.5, 0.5, 0.5}
	frontTopRight := mgl.Vec3{0.5, 0.5, 0.5}
	frontBottomLeft := mgl.Vec3{-0.5, -0.5, 0.5}
	frontBottomRight := mgl.Vec3{0.5, -0.5, 0.5}
	backTopLeft := mgl.Vec3{-0.5, 0.5, -0.5}
	backTopRight := mgl.Vec3{0.5, 0.5, -0.5}
	backBottomLeft := mgl.Vec3{-0.5, -0.5, -0.5}
	backBottomRight := mgl.Vec3{0.5, -0.5, -0.5}

	entity.vertices = []mgl.Vec3{
		frontTopLeft,     // front-top-left
		frontBottomLeft,  // front-bottom-left
		frontTopRight,    // front-top-right
		frontBottomRight, // front-bottom-right
		backTopLeft,      // back-top-left
		backBottomLeft,   // back-bottom-left
		backTopRight,     // back-top-right
		backBottomRight,  // back-bottom-right
	}

	points := []float32{}
	entity.triangles = make([]*Tri, 0)

	front := CreateTriPointsFromQuad(frontTopLeft, frontBottomLeft, frontBottomRight, frontTopRight)
	back := CreateTriPointsFromQuad(backTopLeft, backTopRight, backBottomRight, backBottomLeft)
	left := CreateTriPointsFromQuad(backTopLeft, backBottomLeft, frontBottomLeft, frontTopLeft)
	right := CreateTriPointsFromQuad(frontTopRight, frontBottomRight, backBottomRight, backTopRight)
	top := CreateTriPointsFromQuad(backTopLeft, frontTopLeft, frontTopRight, backTopRight)
	bottom := CreateTriPointsFromQuad(frontBottomLeft, backBottomLeft, backBottomRight, frontBottomRight)

	entity.triangles = append(entity.triangles, front...)
	entity.triangles = append(entity.triangles, back...)
	entity.triangles = append(entity.triangles, left...)
	entity.triangles = append(entity.triangles, right...)
	entity.triangles = append(entity.triangles, top...)
	entity.triangles = append(entity.triangles, bottom...)

	cubeLeaf := BuildLeafAABB(
		front[0], front[1],
		left[0], left[1],
		right[0], right[1],
		back[0], back[1],
		top[0], top[1],
		bottom[0], bottom[1])
	entity.BoundingBox = BuildAABB(cubeLeaf)

	entity.NumTriangles = int32(len(entity.triangles))
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
	}

	entity.VAO = createVAOWithNormals(points)
	// log.Printf("created cube at VAO: %d\n", entity.VAO)
}

func (entity *Cube) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *Cube) GetName() string {
	return "Cube"
}

func (entity *Cube) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *Cube) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
