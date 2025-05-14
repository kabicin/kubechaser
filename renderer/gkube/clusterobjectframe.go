package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GClusterObjectFrame struct {
	parent *GCluster
	object *scene.SceneObject

	children []*GObject

	name      string
	namespace string

	font     *v41.Font
	shaderID uint32

	currentOffset *mgl.Vec3
}

func (gd *GClusterObjectFrame) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent

	gd.object = &scene.SceneObject{}
	gd.object.OnClickColor = mgl.Vec3{0, 1, 1}

	gd.font = font
	gd.shaderID = shaderID

	gd.currentOffset = offset
	gd.children = make([]*GObject, 0)

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GClusterObjectFrame) GetResource() GResource {
	return GCLUSTEROBJECTFRAME
}

func (gd *GClusterObjectFrame) Delete() {

}

func (gd *GClusterObjectFrame) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GClusterObjectFrame) SetObjectFrame(center, bounds mgl.Vec3, onFinishCallback func()) {
	defer onFinishCallback()
	objFrame := &entity.ObjectFrame{}
	objFrame.SetObjectFrame(bounds.X(), bounds.Y(), bounds.Z(), 0.5)
	objFrame.Init(gd.font, "")
	t := &camera.Transform3D{}
	t.Init(&center, &mgl.Vec3{1, 1, 1}, nil)
	gd.object.Init(objFrame, t, gd.shaderID, mgl.Vec3{0, 0, 0}, mgl.Vec3{111, 111, 111})
	gd.object.AddOnClickHandler(gd.OnClick)
}

func (gd *GClusterObjectFrame) UpdateObjectFrame(center, bounds mgl.Vec3, onFinishCallback func()) {
	defer onFinishCallback()
	gd.object.Object.(*entity.ObjectFrame).SetObjectFrame(bounds.X(), bounds.Y(), bounds.Z(), 0.5)
	gd.object.Transform.SetTranslate(&center)
}

func (gd *GClusterObjectFrame) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GClusterObjectFrame) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GClusterObjectFrame) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GClusterObjectFrame) SetDeleting() {

}
