package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GDaemonSet struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GDaemonSet) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gd.object.Color = mgl.Vec3{0.05, 0.05, 0.05}
	gdaemonset := &entity.WavefrontOBJ{FileName: "daemonset.obj"}
	gdaemonset.Init(font, "")
	t := &camera.Transform3D{}
	tOffset := mgl.Vec3{offset.X(), offset.Y(), offset.Z()}
	t.Init(&tOffset, &mgl.Vec3{1, 1, 1}, nil, true)
	gd.object.Init(gdaemonset, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)
	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GDaemonSet) GetResource() GResource {
	return GDAEMONSET
}

func (gd *GDaemonSet) Delete() {
}

func (gd *GDaemonSet) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GDaemonSet) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GDaemonSet) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GDaemonSet) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GDaemonSet) SetDeleting() {
}
