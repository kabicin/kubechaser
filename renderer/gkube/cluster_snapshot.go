package gkube

import (
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

// minified version of GObject able to recreate from footprint
type GObjectFootprint struct {
	Name      string
	Namespace string
	Resource  GResource
	Transform *camera.Transform3D
	Footprint *utils.OrderedMap
	Hash      string
	CurrentT  float32
}

type GSnapshot struct {
	Footprints []GObjectFootprint
	StartT     float32
	EndT       float32
}

func CreateFootprint(resource GResource, name, namespace string, transform *camera.Transform3D, footprint *utils.OrderedMap, currentT float32) *GObjectFootprint {
	obj, objHash := utils.DuplicateOrderedMap(footprint)
	return &GObjectFootprint{
		Resource:  resource,
		Name:      name,
		Namespace: namespace,
		Transform: camera.DuplicateTransform3D(transform),
		Footprint: obj,
		Hash:      objHash,
		CurrentT:  currentT,
	}
}
