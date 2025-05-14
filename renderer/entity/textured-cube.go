package entity

import (
	"log"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/texture"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type TexturedCube struct {
	VAO          uint32
	NumTriangles int32
	NumTextures  int
	Textures     []*texture.Texture
	vertices     []mgl.Vec3
	triangles    []*TTri
	text         *v41.Text
	textPosition mgl.Vec3
}

func (entity *TexturedCube) InitWithTexture(font *v41.Font, text string, filePath string, totalTextureCount uint32) bool {
	entity.Init(font, text)
	return entity.AddTexture(filePath, totalTextureCount)
}

func (entity *TexturedCube) AddTexture(filePath string, textureIndex uint32) bool {
	tex, err := texture.NewTextureFromFile(filePath, textureIndex)
	if err != nil {
		log.Println(err)
		return false
	}
	entity.Textures = append(entity.Textures, tex)
	entity.NumTextures += 1
	return true
}

func (entity *TexturedCube) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 1.5, 0}
	if font != nil {
		entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.5, 0.6, 0.3}, 0.6)
	}
	entity.NumTextures = 0
	entity.Textures = make([]*texture.Texture, 0)

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
	entity.triangles = make([]*TTri, 0)

	front := CreateTTriPointsFromQuad(frontTopLeft, frontBottomLeft, frontBottomRight, frontTopRight, 1)
	back := CreateTTriPointsFromQuad(backTopLeft, backTopRight, backBottomRight, backBottomLeft, 2)
	left := CreateTTriPointsFromQuad(backTopLeft, backBottomLeft, frontBottomLeft, frontTopLeft, 1)
	right := CreateTTriPointsFromQuad(frontTopRight, frontBottomRight, backBottomRight, backTopRight, 1)
	top := CreateTTriPointsFromQuad(backTopLeft, frontTopLeft, frontTopRight, backTopRight, 1)
	bottom := CreateTTriPointsFromQuad(frontBottomLeft, backBottomLeft, backBottomRight, frontBottomRight, 1)

	entity.triangles = append(entity.triangles, front...)
	entity.triangles = append(entity.triangles, back...)
	entity.triangles = append(entity.triangles, left...)
	entity.triangles = append(entity.triangles, right...)
	entity.triangles = append(entity.triangles, top...)
	entity.triangles = append(entity.triangles, bottom...)

	entity.NumTriangles = int32(len(entity.triangles))
	for _, triangle := range entity.triangles {
		point := triangle.GetPoints()
		points = append(points, point...)
		log.Println("point..")
		log.Println(points)
	}

	entity.VAO = createVAOWithNormalsAndTextures(points)
	log.Printf("created textured cube at VAO: %d\n", entity.VAO)
}

func (entity *TexturedCube) BindTextures() {
	for _, texture := range entity.Textures {
		gl.ActiveTexture(gl.TEXTURE0 + texture.ActiveTextureIndex)
		gl.BindTexture(gl.TEXTURE_2D, texture.TextureID)
	}
}

func (entity *TexturedCube) Draw() {
	gl.BindVertexArray(entity.VAO)
	gl.DrawArrays(gl.TRIANGLES, 0, (entity.NumTriangles-2)*(3+3+2)) // (p-2) * (#vertices + #normal + #texture)
	gl.BindVertexArray(0)
}

func (entity *TexturedCube) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *TexturedCube) GetName() string {
	return "TexturedCube"
}

func (entity *TexturedCube) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	localToWorld := cam.GetModel(camTransform)
	v1 := entity.vertices[0]
	v2 := entity.vertices[7]
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	pv2 := localToWorld.Mul4x1(v2.Vec4(1)).Vec3()
	return utils.RayAABBFromVertices(pv1, pv2, ray)
}
