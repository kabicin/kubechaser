package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GNamespaceObjectFrame struct {
	parent *GCluster
	object *scene.SceneObject

	children []*GObject

	name      string
	namespace string

	font          *v41.Font
	shaderID      uint32
	currentOffset *mgl.Vec3

	isObjectFrameCreated bool
}

func (gd *GNamespaceObjectFrame) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent

	gd.object = &scene.SceneObject{}
	// color := mgl.Vec3{0, 1, 0}
	onClickColor := mgl.Vec3{0, 1, 1}
	gd.object.OnClickColor = onClickColor

	// objFrame := &entity.ObjectFrame{}
	// objFrame.Init(gd.font, "")
	// t := &camera.Transform3D{}
	// t.Init(&mgl.Vec3{0, 0, 0}, &mgl.Vec3{1, 1, 1}, nil)
	// gd.object.Init(objFrame, t, gd.shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})

	gd.font = font
	gd.shaderID = shaderID

	gd.children = make([]*GObject, 0)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GNamespaceObjectFrame) GetResource() GResource {
	return GNAMESPACEOBJECTFRAME
}

func (gd *GNamespaceObjectFrame) Delete() {

}

func (gd *GNamespaceObjectFrame) SetObjectFrame(center, bounds mgl.Vec3, onPostInitCallback func()) {
	defer onPostInitCallback()
	objFrame := &entity.ObjectFrame{}
	objFrame.SetObjectFrame(bounds.X(), bounds.Y(), bounds.Z(), 0.5)
	objFrame.Init(gd.font, gd.name)
	t := &camera.Transform3D{}
	t.Init(&center, &mgl.Vec3{1, 1, 1}, nil)
	gd.object.Init(objFrame, t, gd.shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)
	gd.isObjectFrameCreated = true
}

func (gd *GNamespaceObjectFrame) UpdateObjectFrame(center, bounds mgl.Vec3, onPostInitCallback func()) {
	defer onPostInitCallback()
	if !gd.isObjectFrameCreated {
		gd.SetObjectFrame(center, bounds, func() {})
	} else {
		gd.object.Object.(*entity.ObjectFrame).SetObjectFrame(bounds.X(), bounds.Y(), bounds.Z(), 0.5)
		gd.object.Transform.SetTranslate(&center)
	}
}

func (gd *GNamespaceObjectFrame) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GNamespaceObjectFrame) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GNamespaceObjectFrame) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GNamespaceObjectFrame) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GNamespaceObjectFrame) SetDeleting() {

}
