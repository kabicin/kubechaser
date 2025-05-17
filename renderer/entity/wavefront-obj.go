package entity

import (
	"fmt"
	"log"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type WavefrontOBJ struct {
	VAO          uint32
	NumTriangles int32
	vertices     []mgl.Vec3
	triangles    []*NTri
	text         *v41.Text
	textPosition mgl.Vec3
	BoundingBox  *AABB
	FileName     string
}

func (entity *WavefrontOBJ) BindTextures() {}

func (entity *WavefrontOBJ) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.8, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.35546875, 0.56640625, 0.23046875}, 0.5)

	entity.vertices = []mgl.Vec3{}

	points := []float32{}
	if len(entity.FileName) == 0 {
		log.Println("Could not create WavefrontOBJ file without filename")
		return
	}
	triangles, err := LoadNTrianglesFromOBJ(fmt.Sprintf("./assets/models/%s", entity.FileName))
	if err != nil {
		log.Printf("Could not load triangles from OBJ in %f\n", entity.FileName)
		log.Printf("%+v\n", err)
	}
	entity.triangles = triangles

	entity.NumTriangles = int32(len(entity.triangles))
	fmt.Printf("WavefrontOBJ num triangles: %d\n", entity.NumTriangles)
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
	}

	entity.VAO = createVAOWithNormals(points)
	// log.Printf("created cube at VAO: %d\n", entity.VAO)

	// convert NTris into Tris to generate AABB
	trianglePoints := ConvertNTrisToTris(triangles)
	wavefrontLeaf := BuildLeafAABB(trianglePoints...)
	entity.BoundingBox = BuildAABB(wavefrontLeaf)
}

func (entity *WavefrontOBJ) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3)) // (p-2) * (#vertices + #normal)
	gl.BindVertexArray(0)
}

func (entity *WavefrontOBJ) GetName() string {
	return "WavefrontOBJ"
}

func (entity *WavefrontOBJ) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *WavefrontOBJ) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	return RayAABB(&localToWorld, entity.BoundingBox, ray)
}
