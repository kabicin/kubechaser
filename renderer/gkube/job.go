package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

// JobConditionType is a valid value for JobCondition.Type
type JobConditionType string

// These are valid conditions of a job.
const (
	// JobSuspended means the job has been suspended.
	JobSuspended JobConditionType = "Suspended"
	// JobComplete means the job has completed its execution.
	JobComplete JobConditionType = "Complete"
	// JobFailed means the job has failed its execution.
	JobFailed JobConditionType = "Failed"
	// FailureTarget means the job is about to fail its execution.
	JobFailureTarget JobConditionType = "FailureTarget"
	// JobSuccessCriteriaMet means the Job has reached a success state and will be marked as Completed
	JobSuccessCriteriaMet JobConditionType = "SuccessCriteriaMet"
)

type GJob struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name      string
	namespace string

	currentOffset *mgl.Vec3
}

func (gd *GJob) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gdeploymentCube := &entity.Cube{}
	gdeploymentCube.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{3, 3, 3}, nil)
	gd.object.Init(gdeploymentCube, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GJob) GetResource() GResource {
	return GJOB
}

func (gd *GJob) Delete() {

}

func (gd *GJob) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GJob) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GJob) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GJob) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GJob) SetDeleting() {

}
