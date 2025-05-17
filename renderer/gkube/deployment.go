package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

// DeploymentConditionType defines conditions of a deployment.
type DeploymentConditionType string

// These are valid conditions of a deployment.
const (
	// Available means the deployment is available, ie. at least the minimum available
	// replicas required are up and running for at least minReadySeconds.
	DeploymentAvailable DeploymentConditionType = "Available"
	// Progressing means the deployment is progressing. Progress for a deployment is
	// considered when a new replica set is created or adopted, and when new pods scale
	// up or old pods scale down. Progress is not estimated for paused deployments or
	// when progressDeadlineSeconds is not specified.
	DeploymentProgressing DeploymentConditionType = "Progressing"
	// ReplicaFailure is added in a deployment when one of its pods fails to be created
	// or deleted.
	DeploymentReplicaFailure DeploymentConditionType = "ReplicaFailure"
)

type GDeployment struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name      string
	namespace string

	currentOffset *mgl.Vec3
}

func (gd *GDeployment) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}

	// gdeploymentCube := &entity.Cube{}
	// gdeploymentCube.Init(font, name)

	// gdeploymentCube := &entity.TexturedCube{}
	// var totalTextureCount uint32
	// totalTextureCount = shader.MinShaderTextureIndex
	// gdeploymentCube.InitWithTexture(font, "", "./deploy-128.png", totalTextureCount)
	gdeploymentCube := &entity.WavefrontOBJ{FileName: "deployment.obj"}
	gdeploymentCube.Init(font, "")

	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{1, 1, 1}, nil, true)
	gd.object.Init(gdeploymentCube, t, shaderID, mgl.Vec3{float32(50) / 255, float32(229) / 255, float32(148) / 255}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GDeployment) GetResource() GResource {
	return GDEPLOYMENT
}

func (gd *GDeployment) Delete() {
}

func (gd *GDeployment) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GDeployment) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GDeployment) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GDeployment) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GDeployment) SetDeleting() {
}
