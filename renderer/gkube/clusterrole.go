package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GClusterRole struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name      string
	namespace string

	currentOffset *mgl.Vec3
}

func (gd *GClusterRole) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gpod := &entity.Heptagon{}
	gpod.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	tOffset := mgl.Vec3{offset.X(), offset.Y() + 0.25, offset.Z()}
	t.Init(&tOffset, &mgl.Vec3{3, 1, 3}, nil)
	gd.object.Init(gpod, t, shaderID, mgl.Vec3{float32(179) / 255, float32(50) / 255, float32(109) / 255}, mgl.Vec3{1, 1, 1})

	gd.currentOffset = offset

	gd.object.AddOnClickHandler(gd.OnClick)

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GClusterRole) GetResource() GResource {
	return GCLUSTERROLE
}

func (gd *GClusterRole) Delete() {
}

func (gd *GClusterRole) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GClusterRole) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GClusterRole) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GClusterRole) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GClusterRole) SetDeleting() {
}
