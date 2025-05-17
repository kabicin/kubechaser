package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

// StatefulSetConditionType describes the condition types of StatefulSets.
type StatefulSetConditionType string

// These are valid conditions of a statefulset.?
const (
	// Available means the statefulset is available, ie. at least the minimum available
	// replicas required are up and running for at least minReadySeconds.
	StatefulSetAvailable StatefulSetConditionType = "Available"
	// Progressing means the statefulset is progressing. Progress for a statefulset is
	// considered when a new replica set is created or adopted, and when new pods scale
	// up or old pods scale down. Progress is not estimated for paused statefulsets or
	// when progressDeadlineSeconds is not specified.
	StatefulSetProgressing StatefulSetConditionType = "Progressing"
	// ReplicaFailure is added in a statefulset when one of its pods fails to be created
	// or deleted.
	StatefulSetReplicaFailure StatefulSetConditionType = "ReplicaFailure"
)

type GStatefulSet struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GStatefulSet) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gstatefulsetCube := &entity.Cube{}
	gstatefulsetCube.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{3, 3, 3}, nil, true)
	gd.object.Init(gstatefulsetCube, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GStatefulSet) GetResource() GResource {
	return GSTATEFULSET
}

func (gd *GStatefulSet) Delete() {

}

func (gd *GStatefulSet) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GStatefulSet) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GStatefulSet) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GStatefulSet) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GStatefulSet) SetDeleting() {

}
