package entity

import (
	"log"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/shader"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type TEntity struct {
	object      Entity
	transform   *camera.Transform3D
	BoundingBox *AABB
}

type ObjectFrame struct {
	objects []*TEntity

	text         *v41.Text
	textPosition mgl.Vec3

	thicknessRatio float64
	// BoundingBox    *AABB

	width     float32
	height    float32
	depth     float32
	barLength float32
}

func (entity *ObjectFrame) BindTextures() {}

func (entity *ObjectFrame) createFragment(font *v41.Font, trans, scale, rot *mgl.Vec3) {
	c1 := &Cube{}
	c1.Init(font, "")
	c1t := &camera.Transform3D{PositionAnimator: camera.InitAnimator(trans, trans), Rotate: rot, Scale: scale}
	bb1 := BuildLeafAABB(c1.triangles...)
	c1bb := BuildAABB(bb1)
	entity.objects = append(entity.objects, &TEntity{object: c1, transform: c1t, BoundingBox: c1bb})
}
func (entity *ObjectFrame) SetFrame(width, height, depth, barLength float32) {
	entity.width = width
	entity.height = height
	entity.depth = depth
	entity.barLength = barLength
}

func (entity *ObjectFrame) Init(font *v41.Font, text string) {
	entity.textPosition = mgl.Vec3{0, 0.8, 0}
	entity.text = fonts.CreateText(text, font, &mgl.Vec3{0.35546875, 0.56640625, 0.23046875}, 0.5)

	entity.thicknessRatio = 10
	entity.objects = make([]*TEntity, 0)

	width := entity.width
	height := entity.height
	depth := entity.depth
	barLength := entity.barLength
	hheight := (height / 2)
	hwidth := (width / 2)
	hdepth := (depth / 2)
	// barLength := float32(4)
	entity.createFragment(font, &mgl.Vec3{0, hheight, hdepth}, &mgl.Vec3{width + barLength, barLength, barLength}, &mgl.Vec3{0, 0, 0})   // front up bar (0, +y, +z)
	entity.createFragment(font, &mgl.Vec3{0, -hheight, hdepth}, &mgl.Vec3{width + barLength, barLength, barLength}, &mgl.Vec3{0, 0, 0})  //  front down bar (0, -y, +z)
	entity.createFragment(font, &mgl.Vec3{-hwidth, 0, hdepth}, &mgl.Vec3{barLength, height, barLength}, &mgl.Vec3{0, 0, 0})              // front left bar (-x, 0, +z)
	entity.createFragment(font, &mgl.Vec3{hwidth, 0, hdepth}, &mgl.Vec3{barLength, height, barLength}, &mgl.Vec3{0, 0, 0})               // front right bar (x, 0, +z)
	entity.createFragment(font, &mgl.Vec3{-hwidth, hheight, 0}, &mgl.Vec3{barLength, barLength, depth}, &mgl.Vec3{0, 0, 0})              // top left bar (-x, y, 0)
	entity.createFragment(font, &mgl.Vec3{hwidth, hheight, 0}, &mgl.Vec3{barLength, barLength, depth}, &mgl.Vec3{0, 0, 0})               // top right bar (x, y, 0)
	entity.createFragment(font, &mgl.Vec3{-hwidth, -hheight, 0}, &mgl.Vec3{barLength, barLength, depth}, &mgl.Vec3{0, 0, 0})             // bottom left bar (-x, -y, 0)
	entity.createFragment(font, &mgl.Vec3{hwidth, -hheight, 0}, &mgl.Vec3{barLength, barLength, depth}, &mgl.Vec3{0, 0, 0})              // bottom right bar (x, -y, 0)
	entity.createFragment(font, &mgl.Vec3{0, hheight, -hdepth}, &mgl.Vec3{width + barLength, barLength, barLength}, &mgl.Vec3{0, 0, 0})  // back up bar (0, +y, -z)
	entity.createFragment(font, &mgl.Vec3{0, -hheight, -hdepth}, &mgl.Vec3{width + barLength, barLength, barLength}, &mgl.Vec3{0, 0, 0}) //  back down bar (0, -y, -z)
	entity.createFragment(font, &mgl.Vec3{-hwidth, 0, -hdepth}, &mgl.Vec3{barLength, height, barLength}, &mgl.Vec3{0, 0, 0})             // back left bar (-x, 0, -z)
	entity.createFragment(font, &mgl.Vec3{hwidth, 0, -hdepth}, &mgl.Vec3{barLength, height, barLength}, &mgl.Vec3{0, 0, 0})              // back right bar (x, 0, -z)

	log.Printf("created object frame\n")
}

func (entity *ObjectFrame) DrawMultiple(deltaT float32, camTransform *camera.Transform3D, program *shader.Program, cam *camera.Camera, lightPos *mgl.Vec3, cameraPos *mgl.Vec3, color mgl.Vec3, onClick bool, onClickColor mgl.Vec3) {
	for _, tentity := range entity.objects {
		gl.UseProgram(program.ID)
		cam.Update(deltaT)
		scale := mgl.Vec3{0, 0, 0}
		rot := mgl.Vec3{0, 0, 0}
		trans := mgl.Vec3{0, 0, 0}
		if camTransform.Rotate != nil {
			rot = *camTransform.Rotate
		}
		if camTransform.Scale != nil {
			scale = *camTransform.Scale
		}
		if camTransform.PositionAnimator.X_init != nil {
			trans = *camTransform.PositionAnimator.X_init
		}
		tscale := mgl.Vec3{scale.X() * (*tentity).transform.Scale.X(),
			scale.Y() * (*tentity).transform.Scale.Y(),
			scale.Z() * (*tentity).transform.Scale.Z()}
		ttrans := trans.Add(*tentity.transform.PositionAnimator.X_init)
		trot := rot.Add(*tentity.transform.Rotate)
		tt := camera.Transform3D{
			PositionAnimator: camera.PatchNewAnimator(camTransform.PositionAnimator, camera.InitAnimator(&ttrans, &ttrans)),
			Scale:            &tscale,
			Rotate:           &trot}
		cam.SetMVP(&tt)
		tentity.object.BindTextures()
		program.SetUniforms(cam, lightPos, cameraPos, color, onClick, onClickColor)
		tentity.object.Draw()
	}
}

func (entity *ObjectFrame) Draw() {}

func (entity *ObjectFrame) GetName() string {
	return "GroupedEntity:ObjectFrame"
}

func (entity *ObjectFrame) DrawText(localToWorld *mgl.Mat4, cameraRay *camera.Ray, cam *camera.Camera) {
	if entity.text == nil {
		return
	}
	v1 := entity.textPosition
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	utils.UpdateDrawText(pv1, cameraRay, cam, entity.text)
}

func (entity *ObjectFrame) Intersect(cam *camera.Camera, camTransform *camera.Transform3D, ray *camera.Ray, debug bool) (float64, bool) {
	if debug {
		log.Printf("Check %s intersect\n", entity.GetName())
	}
	for _, tentity := range entity.objects {
		scale := mgl.Vec3{0, 0, 0}
		rot := mgl.Vec3{0, 0, 0}
		trans := mgl.Vec3{0, 0, 0}
		if camTransform.Rotate != nil {
			rot = *camTransform.Rotate
		}
		if camTransform.Scale != nil {
			scale = *camTransform.Scale
		}
		if camTransform.PositionAnimator.X_init != nil {
			trans = *camTransform.PositionAnimator.X_init
		}
		tscale := mgl.Vec3{scale.X() * (*tentity).transform.Scale.X(),
			scale.Y() * (*tentity).transform.Scale.Y(),
			scale.Z() * (*tentity).transform.Scale.Z()}
		ttrans := trans.Add(*tentity.transform.PositionAnimator.X_init)
		trot := rot.Add(*tentity.transform.Rotate)
		tt := camera.Transform3D{
			PositionAnimator: camera.PatchNewAnimator(camTransform.PositionAnimator, camera.InitAnimator(&ttrans, &ttrans)),
			Scale:            &tscale,
			Rotate:           &trot}

		localToWorld := cam.GetModel(&tt)
		// localToWorld := cam.GetModel(tentity.transform)
		if t, hit := RayAABB(&localToWorld, tentity.BoundingBox, ray); hit {
			return t, hit
		}
	}
	return 0, false
}
