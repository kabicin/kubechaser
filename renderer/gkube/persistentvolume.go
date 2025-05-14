package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type PersistentVolumePhase string

const (
	// used for PersistentVolumes that are not available
	VolumePending PersistentVolumePhase = "Pending"
	// used for PersistentVolumes that are not yet bound
	// Available volumes are held by the binder and matched to PersistentVolumeClaims
	VolumeAvailable PersistentVolumePhase = "Available"
	// used for PersistentVolumes that are bound
	VolumeBound PersistentVolumePhase = "Bound"
	// used for PersistentVolumes where the bound PersistentVolumeClaim was deleted
	// released volumes must be recycled before becoming available again
	// this phase is used by the persistent volume claim binder to signal to another process to reclaim the resource
	VolumeReleased PersistentVolumePhase = "Released"
	// used for PersistentVolumes that failed to be correctly recycled or deleted after being released from a claim
	VolumeFailed PersistentVolumePhase = "Failed"
)

type GPersistentVolume struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GPersistentVolume) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gpod := &entity.HeptagonalPrism{}
	gpod.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	t.Init(offset, &mgl.Vec3{3, 3, 3}, nil)
	gd.object.Init(gpod, t, shaderID, mgl.Vec3{float32(218) / 255, float32(227) / 255, float32(227) / 255}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GPersistentVolume) GetResource() GResource {
	return GPERSISTENTVOLUME
}

func (gd *GPersistentVolume) Delete() {

}

func (gd *GPersistentVolume) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GPersistentVolume) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GPersistentVolume) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GPersistentVolume) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GPersistentVolume) SetDeleting() {

}
