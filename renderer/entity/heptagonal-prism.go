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

type HeptagonalPrism struct {
	VAO          uint32
	NumTriangles int32
	triangles    []*Tri
	BoundingBox  *AABB
	text         *v41.Text
	textPosition mgl.Vec3
}

func (entity *HeptagonalPrism) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 1.5, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.5, 0.6, 0.3}, 0.6)

	// heptagon on xz plane
	p0 := mgl.Vec3{0, 0.0, -1}
	p1 := mgl.Vec3{0.780624749799799775730861640117474307634119997248957038556621491, 0.0, -0.625}     // Point{sqrt(39)/8, 5/8}
	p2 := mgl.Vec3{0.9757809372497497196635770501468428845426499965611962981957768641, 0.0, 0.21875}   // Point{5*sqrt(39)/32, -7/32}
	p3 := mgl.Vec3{0.4391014217623873738486096725660792980441924984525383341880995888, 0.0, 0.8984375} // Point{9*sqrt(39)/128, -115/128}
	p4 := mgl.Vec3{-p3.X(), 0, p3.Z()}                                                                 // Point{-9*sqrt(39)/128, -115/128}
	p5 := mgl.Vec3{-p2.X(), 0, p2.Z()}                                                                 // Point{-5*sqrt(39)/32, -7/32}
	p6 := mgl.Vec3{-p1.X(), 0, p1.Z()}                                                                 // Point{-sqrt(39)/8, 5/8}

	p := []mgl.Vec3{p0, p1, p2, p3, p4, p5, p6}
	ptop := make([]mgl.Vec3, len(p))
	pbottom := make([]mgl.Vec3, len(p))
	yOffset := mgl.Vec3{0, .5, 0}
	for i := 0; i < len(p); i++ {
		ptop[i] = p[i].Add(yOffset)
		pbottom[i] = p[i].Sub(yOffset)
	}

	points := []float32{}
	entity.triangles = make([]*Tri, 0)

	top1 := CreateTriPoints(ptop[0], ptop[6], ptop[5])
	top2 := CreateTriPoints(ptop[0], ptop[5], ptop[4])
	top3 := CreateTriPoints(ptop[0], ptop[4], ptop[3])
	top4 := CreateTriPoints(ptop[0], ptop[3], ptop[2])
	top5 := CreateTriPoints(ptop[0], ptop[2], ptop[1])
	entity.triangles = append(entity.triangles, top1, top2, top3, top4, top5)

	bottom1 := CreateTriPoints(pbottom[0], pbottom[1], pbottom[2])
	bottom2 := CreateTriPoints(pbottom[0], pbottom[2], pbottom[3])
	bottom3 := CreateTriPoints(pbottom[0], pbottom[3], pbottom[4])
	bottom4 := CreateTriPoints(pbottom[0], pbottom[4], pbottom[5])
	bottom5 := CreateTriPoints(pbottom[0], pbottom[5], pbottom[6])
	entity.triangles = append(entity.triangles, bottom1, bottom2, bottom3, bottom4, bottom5)

	// mid
	mid1 := CreateTriPointsFromQuad(ptop[6], pbottom[6], pbottom[5], ptop[5])
	mid2 := CreateTriPointsFromQuad(ptop[5], pbottom[5], pbottom[4], ptop[4])
	mid3 := CreateTriPointsFromQuad(ptop[4], pbottom[4], pbottom[3], ptop[3])
	mid4 := CreateTriPointsFromQuad(ptop[3], pbottom[3], pbottom[2], ptop[2])
	mid5 := CreateTriPointsFromQuad(ptop[2], pbottom[2], pbottom[1], ptop[1])
	mid6 := CreateTriPointsFromQuad(ptop[1], pbottom[1], pbottom[0], ptop[0])
	mid7 := CreateTriPointsFromQuad(ptop[0], pbottom[0], pbottom[6], ptop[6])
	entity.triangles = append(entity.triangles, mid1...)
	entity.triangles = append(entity.triangles, mid2...)
	entity.triangles = append(entity.triangles, mid3...)
	entity.triangles = append(entity.triangles, mid4...)
	entity.triangles = append(entity.triangles, mid5...)
	entity.triangles = append(entity.triangles, mid6...)
	entity.triangles = append(entity.triangles, mid7...)

	topLeaf := BuildLeafAABB(top1, top2, top3, top4, top5)
	midLeaf := BuildLeafAABB(
		mid1[0], mid1[1],
		mid2[0], mid2[1],
		mid3[0], mid3[1],
		mid4[0], mid4[1],
		mid5[0], mid5[1],
		mid6[0], mid6[1],
		mid7[0], mid7[1],
	)
	bottomLeaf := BuildLeafAABB(bottom1, bottom2, bottom3, bottom4, bottom5)
	entity.BoundingBox = BuildAABB(topLeaf, midLeaf, bottomLeaf)

	entity.NumTriangles = int32(len(entity.triangles))
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
	}

	entity.VAO = createVAOWithNormals(points)
	// log.Printf("created heptagonalprism at VAO: %d\n", entity.VAO)
}

func (entity *HeptagonalPrism) BindTextures() {}

func (entity *HeptagonalPrism) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *HeptagonalPrism) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *HeptagonalPrism) GetName() string {
	return "HeptagonalPrism"
}

func (entity *HeptagonalPrism) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
