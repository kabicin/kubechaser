package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GRoleBinding struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GRoleBinding) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}

	gpod := &entity.Cube{}
	gpod.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{3, 1.5, 1.5}, nil, true)
	gd.object.Init(gpod, t, shaderID, mgl.Vec3{float32(229) / 255, float32(50) / 255, float32(59) / 255}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GRoleBinding) GetResource() GResource {
	return GROLEBINDING
}

func (gd *GRoleBinding) Delete() {

}

func (gd *GRoleBinding) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GRoleBinding) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GRoleBinding) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GRoleBinding) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GRoleBinding) SetDeleting() {

}
