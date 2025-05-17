package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GSecret struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GSecret) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gpod := &entity.Heptagon{}
	gpod.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{1.6, 0.5, 1}, nil, true)
	gd.object.Init(gpod, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GSecret) GetResource() GResource {
	return GSECRET
}

func (gd *GSecret) Delete() {

}

func (gd *GSecret) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GSecret) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GSecret) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GSecret) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GSecret) SetDeleting() {

}
