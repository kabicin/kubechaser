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

type Pyramid struct {
	VAO          uint32
	NumTriangles int32
	// vertices     []mgl.Vec3
	triangles    []*Tri
	text         *v41.Text
	textPosition mgl.Vec3
	BoundingBox  *AABB
}

func (entity *Pyramid) BindTextures() {}

func (entity *Pyramid) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.8, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.35546875, 0.56640625, 0.23046875}, 0.5)

	top := mgl.Vec3{0, 0.5, 0}
	frontLeft := mgl.Vec3{-0.5, -0.5, 0.5}
	frontRight := mgl.Vec3{0.5, -0.5, 0.5}
	backRight := mgl.Vec3{0.5, -0.5, -0.5}
	backLeft := mgl.Vec3{-0.5, -0.5, -0.5}
	// entity.vertices = []mgl.Vec3{top, frontLeft, frontRight, backRight, backLeft}

	entity.triangles = make([]*Tri, 0)
	entity.triangles = append(entity.triangles, CreateTriPoints(top, frontLeft, frontRight))                            // front tri
	entity.triangles = append(entity.triangles, CreateTriPoints(top, backLeft, frontLeft))                              // left tri
	entity.triangles = append(entity.triangles, CreateTriPoints(top, frontRight, backRight))                            // right tri
	entity.triangles = append(entity.triangles, CreateTriPoints(top, backRight, backLeft))                              // back tri
	entity.triangles = append(entity.triangles, CreateTriPointsFromQuad(frontLeft, backLeft, backRight, frontRight)...) // bottom quad

	pyramidAABB := BuildLeafAABB(entity.triangles...)
	entity.BoundingBox = BuildAABB(pyramidAABB)

	points := []float32{}
	entity.NumTriangles = int32(len(entity.triangles))
	log.Printf("num triangles: %d\n", entity.NumTriangles)
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
		for j := 0; j < 3; j++ {
			log.Printf("v(%f, %f, %f) and n(%f, %f, %f)\n", point[6*j], point[6*j+1], point[6*j+2], point[6*j+3], point[6*j+4], point[6*j+5])
		}
	}
	log.Printf("num points: %d\n", len(points))
	entity.VAO = createVAOWithNormals(points)
	log.Printf("created pyramid at VAO: %d\n", entity.VAO)
}

func (entity *Pyramid) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3))
	gl.BindVertexArray(0)
}

func (entity *Pyramid) GetName() string {
	return "Pyramid"
}

func (entity *Pyramid) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *Pyramid) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
