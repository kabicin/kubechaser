package gkube

import (
	"fmt"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

// PersistentVolumeClaimPhase defines the phase of PV claim
type PersistentVolumeClaimPhase string

// These are the valid value for PersistentVolumeClaimPhase
const (
	// used for PersistentVolumeClaims that are not yet bound
	ClaimPending PersistentVolumeClaimPhase = "Pending"
	// used for PersistentVolumeClaims that are bound
	ClaimBound PersistentVolumeClaimPhase = "Bound"
	// used for PersistentVolumeClaims that lost their underlying
	// PersistentVolume. The claim was bound to a PersistentVolume and this
	// volume does not exist any longer and all data on it was lost.
	ClaimLost PersistentVolumeClaimPhase = "Lost"
)

type GPersistentVolumeClaim struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GPersistentVolumeClaim) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gpod := &entity.Heptagon{}
	gpod.Init(font, fmt.Sprintf("%s", name))
	t := &camera.Transform3D{}
	tOffset := mgl.Vec3{offset.X(), offset.Y() + 0.25, offset.Z()}
	t.Init(&tOffset, &mgl.Vec3{1.6, 1.4, 1}, nil)
	gd.object.Init(gpod, t, shaderID, mgl.Vec3{float32(218) / 255, float32(227) / 255, float32(227) / 255}, mgl.Vec3{1, 1, 1})

	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GPersistentVolumeClaim) GetResource() GResource {
	return GPERSISTENTVOLUMECLAIM
}

func (gd *GPersistentVolumeClaim) Delete() {

}

func (gd *GPersistentVolumeClaim) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GPersistentVolumeClaim) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GPersistentVolumeClaim) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GPersistentVolumeClaim) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GPersistentVolumeClaim) SetDeleting() {

}
