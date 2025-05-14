package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GService struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GService) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}

	gservice := &entity.WavefrontOBJ{FileName: "service.obj"}
	gservice.Init(font, "")

	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{1, 1, 1}, nil)
	gd.object.Init(gservice, t, shaderID, mgl.Vec3{float32(229) / 255, float32(175) / 255, float32(50) / 255}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GService) GetResource() GResource {
	return GSERVICE
}

func (gd *GService) Delete() {
}

func (gd *GService) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GService) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GService) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GService) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GService) SetDeleting() {
}
