package gkube

import (
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/scene"
)

type GWire struct {
	parent *GCluster
	object *scene.SceneObject
	state  State

	name          string
	namespace     string
	currentOffset *mgl.Vec3
}

func (gd *GWire) Create(parent *GCluster, name string, namespace string, offset *mgl.Vec3, font *v41.Font, shaderID uint32, settings GSettings, hideText bool) *scene.SceneObject {
	gd.name = name
	gd.namespace = namespace
	gd.parent = parent
	gd.object = &scene.SceneObject{}
	gd.object.Color = mgl.Vec3{1, 1, 1}

	gwireCube := &entity.Cube{}
	gwireCube.Init(font, "")
	t := &camera.Transform3D{}

	scale := mgl.Vec3{3, 0.2, 0.2}
	extraOffset := mgl.Vec3{0, 0, 0}
	parsedSettings := GetSettings(settings)
	if len(parsedSettings) > 0 {
		setting := parsedSettings[0]
		if setting == GSETTING_GWIRE_NORTH {
			scale = mgl.Vec3{0.2, 0.2, 1.5}
			extraOffset = mgl.Vec3{0, 0, -0.75}
		}
		if setting == GSETTING_GWIRE_SOUTH {
			scale = mgl.Vec3{0.2, 0.2, 1.5}
			extraOffset = mgl.Vec3{0, 0, 0.75}
		}
		if setting == GSETTING_GWIRE_EAST {
			scale = mgl.Vec3{1.5, 0.2, 0.2}
			extraOffset = mgl.Vec3{0.75, 0, 0}
		}
		if setting == GSETTING_GWIRE_WEST {
			scale = mgl.Vec3{1.5, 0.2, 0.2}
			extraOffset = mgl.Vec3{-0.75, 0, 0}
		}
		if setting == GSETTING_GWIRE_VERT {
			scale = mgl.Vec3{0.2, 3, 0.2}
			extraOffset = mgl.Vec3{0, 1.5, 0}
		}
		if setting == GSETTING_GWIRE_VERT_NORTH {
			scale = mgl.Vec3{0.2, 1.5, 0.2}
			extraOffset = mgl.Vec3{0, 2, 0}
		}
		if setting == GSETTING_GWIRE_VERT_SOUTH {
			scale = mgl.Vec3{0.2, 1.5, 0.2}
			extraOffset = mgl.Vec3{0, 0.5, 0}
		}
		// if setting == GSETTING_GWIRE_VERT_EAST {
		// 	scale = mgl.Vec3{0.2, 1.5, 0.2}
		// 	extraOffset = mgl.Vec3{0, 0, 0}
		// }
		// if setting == GSETTING_GWIRE_VERT_WEST {
		// 	scale = mgl.Vec3{0.2, 1.5, 0.2}
		// 	extraOffset = mgl.Vec3{0, 0, 0}
		// }
	}

	tOffset := extraOffset.Add(mgl.Vec3{offset.X(), offset.Y(), offset.Z()})
	t.Init(&tOffset, &scale, nil, true)

	gd.object.Init(gwireCube, t, shaderID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	gd.object.AddOnClickHandler(gd.OnClick)

	gd.currentOffset = offset

	gd.parent.mainScene.AddObject(gd.object)
	return gd.object
}

func (gd *GWire) GetResource() GResource {
	return GWIRE
}

func (gd *GWire) Delete() {

}

func (gd *GWire) GetCurrentOffset() *mgl.Vec3 {
	return gd.currentOffset
}

func (gd *GWire) GetObject() *scene.SceneObject {
	return gd.object
}

func (gd *GWire) GetIdentifier() (string, string) {
	return gd.name, gd.namespace
}

func (gd *GWire) OnClick() {
	gd.parent.SetSelected(gd)
}

func (gd *GWire) SetDeleting() {

}
