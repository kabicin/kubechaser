package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GPod struct {
	parent    *GCluster
	object    *scene.SceneObject
	state     State
	kubeState map[string]interface{}

	name      string
	namespace string

	currentOffset *mgl.Vec3
}

func (gd *GPod) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	color := mgl.Vec3{0.19607843137, 0.42352941176, 0.89803921568}
	onClickColor := mgl.Vec3{0.19607843137, 0.42352941176, 0.89803921568}

	gpod := &entity.WavefrontOBJ{FileName: "pod.obj"}
	gpod.Init(font, name)

	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{2, 2, 2}, nil, true)
	gd.object.Init(gpod, t, shaderID, color, onClickColor)
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GPod) SetKubeState(kubeState map[string]interface{}) {
	gd.kubeState = kubeState
}

func (gd *GPod) GetResource() GResource {
	return GPOD
}

// removes self from the main scene
func (gd *GPod) Delete() {
	// gd.parent.mainScene.DeleteObject(gd.object) // remove from the main scene - stops drawing
	// gd.parent.DeleteGObject(gd)                 // remove from the Cluster's memory
}

func (gd *GPod) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GPod) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GPod) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GPod) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GPod) SetDeleting() {
	gd.object.IsDeleting = true
}
