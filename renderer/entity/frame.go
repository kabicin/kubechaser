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

type Frame struct {
	VAO          uint32
	NumTriangles int32
	triangles    []*Tri
	// vertices       []mgl.Vec3
	text           *v41.Text
	textPosition   mgl.Vec3
	thicknessRatio float64
	BoundingBox    *AABB
}

func (entity *Frame) BindTextures() {}

func (entity *Frame) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.8, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.35546875, 0.56640625, 0.23046875}, 0.5)

	entity.thicknessRatio = 10
	maxWidth := 1.0
	offset := float32(maxWidth / entity.thicknessRatio)

	frontTopLeft := mgl.Vec3{-0.5, 0.5, 0.5}
	frontBottomLeft := mgl.Vec3{-0.5, -0.5, 0.5}
	frontTopRight := mgl.Vec3{0.5, 0.5, 0.5}
	frontBottomRight := mgl.Vec3{0.5, -0.5, 0.5}
	backTopLeft := mgl.Vec3{-0.5, 0.5, -0.5}
	backBottomLeft := mgl.Vec3{-0.5, -0.5, -0.5}
	backTopRight := mgl.Vec3{0.5, 0.5, -0.5}
	backBottomRight := mgl.Vec3{0.5, -0.5, -0.5}

	// top face
	topDownLeftInner := frontTopLeft.Add(mgl.Vec3{offset, 0, -offset})
	topDownRightInner := frontTopRight.Add(mgl.Vec3{-offset, 0, -offset})
	topUpLeftInner := backTopLeft.Add(mgl.Vec3{offset, 0, offset})
	topUpRightInner := backTopRight.Add(mgl.Vec3{-offset, 0, offset})

	// bottom face
	// bottomUpLeftInner := frontBottomLeft.Add(mgl.Vec3{offset, 0, -offset})    // +x,-z
	// bottomUpRightInner := frontBottomRight.Add(mgl.Vec3{-offset, 0, -offset}) // -x,-z
	// bottomDownLeftInner := backBottomLeft.Add(mgl.Vec3{offset, 0, offset})    // +x,+z
	// bottomDownRightInner := backBottomRight.Add(mgl.Vec3{-offset, 0, offset}) // -x,+z

	// left face
	leftUpLeftInner := backTopLeft.Add(mgl.Vec3{0, -offset, offset})        // -y, z
	leftUpRightInner := frontTopLeft.Add(mgl.Vec3{0, -offset, -offset})     // -y,-z
	leftDownLeftInner := backBottomLeft.Add(mgl.Vec3{0, offset, offset})    // +y,+z
	leftDownRightInner := frontBottomLeft.Add(mgl.Vec3{0, offset, -offset}) // +y,-z

	// right face
	rightUpRightInner := backTopRight.Add(mgl.Vec3{0, -offset, offset})      // -y, z
	rightUpLeftInner := frontTopRight.Add(mgl.Vec3{0, -offset, -offset})     // -y,-z
	rightDownLeftInner := frontBottomRight.Add(mgl.Vec3{0, offset, -offset}) // +y,-z
	rightDownRightInner := backBottomRight.Add(mgl.Vec3{0, offset, offset})  // +y,+z

	// front face
	frontUpLeftInner := frontTopLeft.Add(mgl.Vec3{offset, -offset, 0})        // +x,-y
	frontUpRightInner := frontTopRight.Add(mgl.Vec3{-offset, -offset, 0})     // -x,-y
	frontDownLeftInner := frontBottomLeft.Add(mgl.Vec3{offset, offset, 0})    // +x,+y
	frontDownRightInner := frontBottomRight.Add(mgl.Vec3{-offset, offset, 0}) // -x,+y

	// back face
	backUpLeftInner := backTopLeft.Add(mgl.Vec3{offset, -offset, 0})        // +x,-y
	backUpRightInner := backTopRight.Add(mgl.Vec3{-offset, -offset, 0})     // -x,-y
	backDownLeftInner := backBottomLeft.Add(mgl.Vec3{offset, offset, 0})    // +x,+y
	backDownRightInner := backBottomRight.Add(mgl.Vec3{-offset, offset, 0}) // -x,+y

	points := []float32{}
	entity.triangles = make([]*Tri, 0)

	// top
	top := CreateTriPointsFromBorder(
		backTopLeft, frontTopLeft, frontTopRight, backTopRight,
		topUpLeftInner, topDownLeftInner, topDownRightInner, topUpRightInner)
	entity.triangles = append(entity.triangles, top...)

	// bottom
	// bottom := CreateTriPointsFromBorder(
	// 	frontBottomLeft, backBottomLeft, backBottomRight, frontBottomRight,
	// 	bottomUpLeftInner, bottomDownLeftInner, bottomDownRightInner, bottomUpRightInner)
	// entity.triangles = append(entity.triangles, bottom...)

	// left
	left := CreateTriPointsFromBorder(
		frontTopLeft, backTopLeft, backBottomLeft, frontBottomLeft,
		leftUpRightInner, leftUpLeftInner, leftDownLeftInner, leftDownRightInner)
	entity.triangles = append(entity.triangles, left...)

	// right
	right := CreateTriPointsFromBorder(
		backTopRight, frontTopRight, frontBottomRight, backBottomRight,
		rightUpRightInner, rightUpLeftInner, rightDownLeftInner, rightDownRightInner)
	entity.triangles = append(entity.triangles, right...)

	// front
	front := CreateTriPointsFromBorder(
		frontTopLeft, frontBottomLeft, frontBottomRight, frontTopRight,
		frontUpLeftInner, frontDownLeftInner, frontDownRightInner, frontUpRightInner)
	entity.triangles = append(entity.triangles, front...)

	// back
	back := CreateTriPointsFromBorder(
		backTopLeft, backTopRight, backBottomRight, backBottomLeft,
		backUpLeftInner, backUpRightInner, backDownRightInner, backDownLeftInner)
	entity.triangles = append(entity.triangles, back...)

	frameLeaf := BuildLeafAABB(entity.triangles...)
	entity.BoundingBox = BuildAABB(frameLeaf)

	entity.NumTriangles = int32(len(entity.triangles))
	log.Printf("this many triangels!!! %d\n", entity.NumTriangles)
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
	}

	entity.VAO = createVAOWithNormals(points)
	log.Printf("created frame at VAO: %d\n", entity.VAO)
}

func (entity *Frame) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *Frame) GetName() string {
	return "Frame"
}

func (entity *Frame) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

// func vec3ToString(v mgl.Vec3) string {
// 	return fmt.Sprintf("v: (%f, %f, %f)", v.X(), v.Y(), v.Z())
// }

func (entity *Frame) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
