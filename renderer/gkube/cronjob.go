package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type CronJobConditionType string // custom type

// These are valid conditions of a cron job, based on schedule
const (
	// JobSuspended means the job has been suspended.
	CronJobRunning CronJobConditionType = "Running"
	// JobComplete means the job has completed its execution.
	CronJobIdle CronJobConditionType = "Idle"
)

type GCronJob struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GCronJob) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gdeploymentCube := &entity.Cube{}
	gdeploymentCube.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	tOffset := mgl.Vec3{offset.X(), offset.Y(), offset.Z()}
	t.Init(&tOffset, &mgl.Vec3{3, 3, 3}, nil)
	gd.object.Init(gdeploymentCube, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})

	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GCronJob) GetResource() GResource {
	return GCRONJOB
}

func (gd *GCronJob) Delete() {

}

func (gd *GCronJob) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GCronJob) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GCronJob) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GCronJob) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GCronJob) SetDeleting() {

}
