package scene

import (
	"log"
	"strings"

	"github.com/go-gl/gl/v4.1-core/gl"
	"github.com/go-gl/glfw/v3.3/glfw"

	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/shader"
)

type SceneObject struct {
	Object              entity.Entity
	Transform           *camera.Transform3D
	DeleteColorAnimator camera.ColorAnimator
	IsDeleting          bool
	IsDeleteReady       bool

	ShaderProgramID          *uint32
	CachedShaderProgramIndex int
	IgnoreLights             bool
	IsLightObject            bool
	ClickHandlers            []func()

	// Draw attribs
	Color        mgl.Vec3
	OnClickColor mgl.Vec3
	OnClick      bool
	Wireframe    bool

	Spinning bool
	Opacity  float32

	LastTime            *uint64
	AccelerateForward   bool
	AccelerateStart     float64
	AccelerateEnd       float64
	Accelerate          float64
	AccelerateMagnitude float64
}

func (so *SceneObject) Init(ent entity.Entity, transform *camera.Transform3D, shaderID uint32, color, onClickColor mgl.Vec3) {
	so.Object = ent
	so.Transform = transform
	if shaderID == 0 {
		so.ShaderProgramID = nil
		log.Printf("Shader program ID was not set!\n")
	} else {
		so.ShaderProgramID = &shaderID
		// log.Printf("Shader program ID set to %d\n", shaderID)
	}
	so.CachedShaderProgramIndex = -1
	so.IgnoreLights = false
	so.IsLightObject = false
	so.ClickHandlers = make([]func(), 0)

	// accel
	// 1,000,000 to 300,000
	//
	so.AccelerateForward = false
	so.AccelerateStart = 1000000
	so.AccelerateEnd = 300000
	so.Accelerate = so.AccelerateStart
	so.AccelerateMagnitude = 1

	so.Color = color
	so.OnClickColor = onClickColor
	so.DeleteColorAnimator = camera.InitColorAnimator(&so.Color, &mgl.Vec3{1, 0, 0}, 4)

}

func (so *SceneObject) SetLightObject(isLightObject bool) {
	so.IsLightObject = isLightObject
}

func (so *SceneObject) SetIgnoreLights(ignoreLights bool) {
	so.IgnoreLights = ignoreLights
}

func (so *SceneObject) AddOnClickHandler(f func()) {
	so.ClickHandlers = append(so.ClickHandlers, f)
}

func (so *SceneObject) NotifyOnClickHandlers() {
	for _, clickHandler := range so.ClickHandlers {
		clickHandler()
	}
	if so.OnClick {
		so.OnClick = false
		so.AccelerateForward = false
	} else {
		so.OnClick = true
		so.AccelerateForward = true
	}
}

func (s *SceneObject) Draw(deltaT float32, transform *camera.Transform3D, program *shader.Program, cam *camera.Camera, lightPos *mgl.Vec3, cameraPos *mgl.Vec3) {
	gl.UseProgram(program.ID)
	cam.Update(deltaT)
	// update transform

	if s.Spinning {
		now := float64(glfw.GetTimerValue())
		// transform.IPRotate = &mgl.Vec3{0, mgl.DegToRad(float32((int(now / 300000)) % 360)), 0}
		if s.AccelerateForward {
			if s.Accelerate > s.AccelerateEnd {
				s.Accelerate -= 1
			}
		} else {
			if s.Accelerate < s.AccelerateStart {
				s.Accelerate += 1
			}
		}

		transform.IPRotate = &mgl.Vec3{0, mgl.DegToRad(float32((int(now / (s.Accelerate * s.AccelerateMagnitude))) % 360)), 0}
	}

	color := mgl.Vec3{}
	if s.IsDeleting {
		color = *s.DeleteColorAnimator.Animate(deltaT, func() {
			s.IsDeleteReady = true // signal to the slots that it is time to delete
		})
	} else {
		if transform.PositionAnimator.InMotion {
			color = mgl.Vec3{240, 230, 140}
		} else {
			color = s.Color
		}
	}
	cam.SetMVP(transform)
	s.Object.BindTextures()
	program.SetUniforms(cam, lightPos, cameraPos, color, s.OnClick, s.OnClickColor)
	inMotion := transform.PositionAnimator.InMotion
	if s.Wireframe || inMotion {
		gl.PolygonMode(gl.FRONT_AND_BACK, gl.LINE)
	}
	if strings.HasPrefix(s.Object.GetName(), "GroupedEntity") {
		(s.Object).(entity.GroupedEntity).DrawMultiple(deltaT, transform, program, cam, lightPos, cameraPos, color, s.OnClick, s.OnClickColor)
	} else {
		s.Object.Draw()
	}
	if s.Wireframe || inMotion {
		gl.PolygonMode(gl.FRONT_AND_BACK, gl.FILL)
	}
}
