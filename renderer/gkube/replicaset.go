package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

// ReplicaSetConditionType is a condition of a replica set.
type ReplicaSetConditionType string

// These are valid conditions of a replica set.
const (
	// ReplicaSetReplicaFailure is added in a replica set when one of its pods fails to be created
	// due to insufficient quota, limit ranges, pod security policy, node selectors, etc. or deleted
	// due to kubelet being down or finalizers are failing.
	ReplicaSetReplicaFailure ReplicaSetConditionType = "ReplicaFailure"
)

type GReplicaSet struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name      string
	namespace string

	currentOffset *mgl.Vec3
}

func (gd *GReplicaSet) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	// gd.object.Wireframe = true

	greplicaset := &entity.WavefrontOBJ{FileName: "replicaset.obj"}
	greplicaset.Init(font, "")
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{1, 1, 1}, nil, true)
	gd.object.Init(greplicaset, t, shaderID, mgl.Vec3{1, 1, 0}, mgl.Vec3{1, 1, 0})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GReplicaSet) GetResource() GResource {
	return GREPLICASET
}

func (gd *GReplicaSet) Delete() {
}

func (gd *GReplicaSet) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GReplicaSet) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GReplicaSet) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GReplicaSet) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GReplicaSet) SetDeleting() {
}
