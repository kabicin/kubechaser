package entity

import (
	"log"
	"math"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type Heptagon struct {
	VAO          uint32
	NumTriangles int32
	triangles    []*Tri
	BoundingBox  *AABB
	text         *v41.Text
	textPosition mgl.Vec3
}

func (entity *Heptagon) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 1.5, 0}
	if len(text) > 0 {
		entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.5, 0.6, 0.3}, 0.6)
	}

	// heptagon 1
	p0 := mgl.Vec3{0, 1, 0.0}
	p1 := mgl.Vec3{0.780624749799799775730861640117474307634119997248957038556621491, 0.625, 0.0}       // Point{sqrt(39)/8, 5/8}
	p2 := mgl.Vec3{0.9757809372497497196635770501468428845426499965611962981957768641, -0.21875, 0.0}   // Point{5*sqrt(39)/32, -7/32}
	p3 := mgl.Vec3{0.4391014217623873738486096725660792980441924984525383341880995888, -0.8984375, 0.0} // Point{9*sqrt(39)/128, -115/128}
	p4 := mgl.Vec3{-p3.X(), p3.Y(), 0.0}                                                                // Point{-9*sqrt(39)/128, -115/128}
	p5 := mgl.Vec3{-p2.X(), p2.Y(), 0.0}                                                                // Point{-5*sqrt(39)/32, -7/32}
	p6 := mgl.Vec3{-p1.X(), p1.Y(), 0.0}                                                                // Point{-sqrt(39)/8, 5/8}

	// rotation
	theta := float64(mgl.DegToRad(90))
	Ry := mgl.Mat3{
		float32(math.Cos(theta)), 0, float32(math.Sin(theta)),
		0, 1, 0,
		-float32(math.Sin(theta)), 0, float32(math.Cos(theta))}
	p := []mgl.Vec3{p0, p1, p2, p3, p4, p5, p6}

	// heptagon 2
	for i := 0; i < 7; i++ {
		p = append(p, Ry.Mul3x1(p[i]))
	}

	points := []float32{}
	entity.triangles = make([]*Tri, 0)
	// top, triangular prism
	// top1 := CreateTriPoints(p[0], p[6], p[13])
	// top2 := CreateTriPoints(p[0], p[13], p[1])
	// top3 := CreateTriPoints(p[0], p[1], p[8])
	// top4 := CreateTriPoints(p[0], p[8], p[6])
	top1 := CreateTriPoints(p[0], p[13], p[6])
	top2 := CreateTriPoints(p[0], p[1], p[13])
	top3 := CreateTriPoints(p[0], p[8], p[1])
	top4 := CreateTriPoints(p[0], p[6], p[8])
	entity.triangles = append(entity.triangles, top1)
	entity.triangles = append(entity.triangles, top2)
	entity.triangles = append(entity.triangles, top3)
	entity.triangles = append(entity.triangles, top4)
	// mid, quads
	// mid1 := CreateTriPointsFromQuad(p[6], p[5], p[12], p[13])
	// mid2 := CreateTriPointsFromQuad(p[13], p[12], p[2], p[1])
	// mid3 := CreateTriPointsFromQuad(p[1], p[2], p[9], p[8])
	// mid4 := CreateTriPointsFromQuad(p[8], p[9], p[5], p[6])
	// mid5 := CreateTriPointsFromQuad(p[5], p[4], p[11], p[12])
	// mid6 := CreateTriPointsFromQuad(p[12], p[11], p[3], p[2])
	// mid7 := CreateTriPointsFromQuad(p[2], p[3], p[10], p[9])
	// mid8 := CreateTriPointsFromQuad(p[9], p[10], p[4], p[5])
	mid1 := CreateTriPointsFromQuad(p[6], p[13], p[12], p[5])
	mid2 := CreateTriPointsFromQuad(p[13], p[1], p[2], p[12])
	mid3 := CreateTriPointsFromQuad(p[1], p[8], p[9], p[2])
	mid4 := CreateTriPointsFromQuad(p[8], p[6], p[5], p[9])
	mid5 := CreateTriPointsFromQuad(p[5], p[12], p[11], p[4])
	mid6 := CreateTriPointsFromQuad(p[12], p[2], p[3], p[11])
	mid7 := CreateTriPointsFromQuad(p[2], p[9], p[10], p[3])
	mid8 := CreateTriPointsFromQuad(p[9], p[5], p[4], p[10])
	entity.triangles = append(entity.triangles, mid1...)
	entity.triangles = append(entity.triangles, mid2...)
	entity.triangles = append(entity.triangles, mid3...)
	entity.triangles = append(entity.triangles, mid4...)
	entity.triangles = append(entity.triangles, mid5...)
	entity.triangles = append(entity.triangles, mid6...)
	entity.triangles = append(entity.triangles, mid7...)
	entity.triangles = append(entity.triangles, mid8...)
	// bottom, quad
	bottom := CreateTriPointsFromQuad(p[11], p[3], p[10], p[4])
	entity.triangles = append(entity.triangles, bottom...)

	topLeaf := BuildLeafAABB(top1, top2, top3, top4)
	midLeaf := BuildLeafAABB(
		mid1[0], mid1[1],
		mid2[0], mid2[1],
		mid3[0], mid3[1],
		mid4[0], mid4[1],
		mid5[0], mid5[1],
		mid6[0], mid6[1],
		mid7[0], mid7[1],
		mid8[0], mid8[1],
	)
	bottomLeaf := BuildLeafAABB(bottom[0], bottom[1])
	entity.BoundingBox = BuildAABB(topLeaf, midLeaf, bottomLeaf)

	// for i := range len(p) - 2 {
	// 	triPoints := make([]mgl.Vec3, 3)
	// 	triPoints[0] = p[0]
	// 	triPoints[1] = p[i+2]
	// 	triPoints[2] = p[i+1]
	// 	log.Printf("v: (%f, %f, %f)", triPoints[1].X(), triPoints[1].Y(), triPoints[1].Z())
	// 	triangles = append(triangles, Tri{P: &triPoints})
	// }
	//
	// log.Printf("There are %d triangles\n", len(triangles))
	entity.NumTriangles = int32(len(entity.triangles))
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
		// for j := 0; j < 3; j++ {
		// 	log.Printf("v(%f, %f, %f) and n(%f, %f, %f)\n", point[6*j], point[6*j+1], point[6*j+2], point[6*j+3], point[6*j+4], point[6*j+5])
		// }
	}

	entity.VAO = createVAOWithNormals(points)
	// log.Printf("created heptagon at VAO: %d\n", entity.VAO)
}

func (entity *Heptagon) BindTextures() {}

func (entity *Heptagon) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *Heptagon) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)

}

func (entity *Heptagon) GetName() string {
	return "Heptagon"
}

func (entity *Heptagon) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
