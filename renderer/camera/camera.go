package camera

import (
	"math"

	"github.com/go-gl/glfw/v3.3/glfw"
	mgl "github.com/go-gl/mathgl/mgl32"
)

type Camera struct {
	ID              int
	AspectRatio     float64
	Model           mgl.Mat4
	View            mgl.Mat4
	Projection      mgl.Mat4
	OrthoProjection mgl.Mat4
	// camera vectors
	EyeAnimator Animator

	Front     mgl.Vec3
	Up        mgl.Vec3
	Direction mgl.Vec3
	// motion handler
	MotionEvent uint32
	BoundKeys   []glfw.Key
	// field of view
	FOV  float64
	Near float32
	Far  float32

	WindowWidth  float64
	WindowHeight float64

	DisableCursor bool
	keys          KeyBit

	cache map[string]mgl.Vec3
}

type Transform3D struct {
	PositionAnimator Animator
	Scale            *mgl.Vec3
	Rotate           *mgl.Vec3
	IPRotate         *mgl.Vec3
}

func copyVec3(a *mgl.Vec3) *mgl.Vec3 {
	var c *mgl.Vec3
	if a != nil {
		c = &mgl.Vec3{a.X(), a.Y(), a.Z()}
	}
	return c
}

// Duplicates animator
func DuplicatePositionAnimator(animator Animator) Animator {
	return Animator{
		X_init:      copyVec3(animator.X_init),
		X_final:     copyVec3(animator.X_final),
		X_final_now: animator.X_final_now,
		V_init:      animator.V_init,
		A:           animator.A,
	}
}

func CreateTransform3D(translate, scale, rotate *mgl.Vec3) *Transform3D {
	t3 := &Transform3D{}
	t3.Init(translate, scale, rotate)
	return t3
}

// Duplicates transform3D
func DuplicateTransform3D(transform3D *Transform3D) *Transform3D {
	return &Transform3D{
		PositionAnimator: DuplicatePositionAnimator(transform3D.PositionAnimator),
		Scale:            copyVec3(transform3D.Scale),
		Rotate:           copyVec3(transform3D.Rotate),
		IPRotate:         copyVec3(transform3D.IPRotate),
	}
}

func (t *Transform3D) Init(translate *mgl.Vec3, scale *mgl.Vec3, rotate *mgl.Vec3) {
	if translate != nil {
		t.PositionAnimator.X_final = translate
		if t.PositionAnimator.X_init == nil {
			t.PositionAnimator.X_init = &mgl.Vec3{t.PositionAnimator.X_final.X(), t.PositionAnimator.X_final.Y(), t.PositionAnimator.X_final.Z()}
		}
	} else {
		t.PositionAnimator.X_final = &mgl.Vec3{0, 0, 0}
	}
	if scale != nil {
		t.Scale = scale
	} else {
		t.Scale = &mgl.Vec3{0, 0, 0}
	}
	if rotate != nil {
		t.Rotate = rotate
	} else {
		t.Rotate = &mgl.Vec3{0, 0, 0}
	}
	t.IPRotate = &mgl.Vec3{0, 0, 0}
	t.PositionAnimator.AnimationDuration = 3
	t.PositionAnimator.InMotion = false
	t.PositionAnimator.A = mgl.Vec3{0, 0, 0}
	t.PositionAnimator.V_init = mgl.Vec3{0, 0, 0}
}

func (c *Camera) SetCameraFront(front mgl.Vec3) {
	if c.DisableCursor {
		return
	}
	c.Front = front
}

func (c *Camera) Init(windowWidth, windowHeight int, transforms *Transform3D) {
	c.EyeAnimator = Animator{
		X_init:            &mgl.Vec3{-1000, 6, -1000},
		X_final:           &mgl.Vec3{-20, 6, -20},
		A:                 mgl.Vec3{0, 0, 0},
		AnimationDuration: 40,
		InMotion:          false,
		V_init:            mgl.Vec3{0, 0, 0},
	}

	c.Front = mgl.Vec3{0, 0, -1}
	c.Direction = mgl.Vec3{0, 0, 0}
	c.Up = mgl.Vec3{0, 1, 0}
	c.MotionEvent = 0
	c.FOV = 90.0
	c.Near = 1.0
	c.Far = 300.0
	c.WindowWidth = float64(windowWidth)
	c.WindowHeight = float64(windowHeight)
	c.SetAspectRatio()
	c.BoundKeys = []glfw.Key{glfw.KeyW, glfw.KeyA, glfw.KeyS, glfw.KeyD, glfw.KeySpace, glfw.KeyLeftShift, glfw.KeyR}
	c.SetMVP(transforms)
	c.DisableCursor = false
	c.cache = make(map[string]mgl.Vec3)
}

func (c *Camera) SetAspectRatio() {
	if c.WindowHeight == 0 {
		return
	}
	c.AspectRatio = c.WindowWidth / c.WindowHeight
}

func (c *Camera) SetMVP(transforms *Transform3D) {
	c.SetOrthoProjection()
	c.SetProjection()
	c.SetView()
	c.SetModel(transforms)
}

func (c *Camera) SetProjection() {
	c.Projection = mgl.Perspective(mgl.DegToRad(float32(c.FOV)), float32(c.AspectRatio), c.Near, c.Far)
}

func (c *Camera) SetOrthoProjection() {
	c.OrthoProjection = mgl.Ortho2D(-float32(c.WindowWidth)/2, float32(c.WindowWidth)/2, -float32(c.WindowWidth)/2, float32(c.WindowWidth)/2)
}

func (c *Camera) Update(deltaT float32) {
	// y := c.EyeAnimator.X_init[1]

	// animation
	c.EyeAnimator.Animate(deltaT, func() {
		c.Front = mgl.Vec3{0, 6, 0}.Sub(*c.EyeAnimator.X_init)
	})

	if !c.EyeAnimator.InMotion {
		nextInitialEye := c.EyeAnimator.X_init.Add(c.Direction.Mul(3 * deltaT))
		c.EyeAnimator.X_init = &nextInitialEye
		c.EyeAnimator.X_final = c.EyeAnimator.X_init
	}

	// if !c.aerialMode {
	// 	c.InitialEye[1] = y
	// }
	// logg.PrintVec3(c.InitialEye)
	c.SetView()
	c.SetProjection()
	c.SetOrthoProjection()
}

func (c *Camera) SetView() {
	c.View = mgl.LookAtV(*c.EyeAnimator.X_init, c.EyeAnimator.X_init.Add(c.Front), c.Up)
}

func (c *Camera) SetModel(transforms *Transform3D) {
	c.Model = c.GetModel(transforms)
}

func (c *Camera) GetModel(transforms *Transform3D) mgl.Mat4 {
	if transforms != nil {
		translateMat4, scaleMat4, rotateMat4, ipRotateMat4 := mgl.Ident4(), mgl.Ident4(), mgl.Ident4(), mgl.Ident4()
		if transforms.PositionAnimator.X_init != nil {
			// fmt.Println("setting last translate:")
			// logg.PrintVec3(*transforms.PositionAnimator.X_init)
			translateMat4 = mgl.Translate3D(transforms.PositionAnimator.X_init.X(), transforms.PositionAnimator.X_init.Y(), transforms.PositionAnimator.X_init.Z())
		}
		if transforms.Scale != nil {
			scaleMat4 = mgl.Scale3D(transforms.Scale.X(), transforms.Scale.Y(), transforms.Scale.Z())
		}
		if transforms.Rotate != nil {
			rotateXMat4 := mgl.HomogRotate3DX(transforms.Rotate.X())
			rotateYMat4 := mgl.HomogRotate3DY(transforms.Rotate.Y())
			rotateZMat4 := mgl.HomogRotate3DZ(transforms.Rotate.Z())
			rotateMat4 = rotateXMat4.Mul4(rotateYMat4.Mul4(rotateZMat4))
		}
		if transforms.IPRotate != nil {
			rotateXMat4 := mgl.HomogRotate3DX(transforms.IPRotate.X())
			rotateYMat4 := mgl.HomogRotate3DY(transforms.IPRotate.Y())
			rotateZMat4 := mgl.HomogRotate3DZ(transforms.IPRotate.Z())
			ipRotateMat4 = rotateXMat4.Mul4(rotateYMat4.Mul4(rotateZMat4))
		}
		return rotateMat4.Mul4(translateMat4.Mul4(scaleMat4.Mul4(ipRotateMat4.Mul4(mgl.Ident4()))))
	}
	return mgl.Ident4()
}

func (c *Camera) GetMVP() mgl.Mat4 {
	return c.Projection.Mul4(c.View.Mul4(c.Model))
}

type KeyBit int

const (
	KEY_LEFT     KeyBit = 0b000001
	KEY_RIGHT    KeyBit = 0b000010
	KEY_FORWARD  KeyBit = 0b000100
	KEY_BACKWARD KeyBit = 0b001000
	KEY_UP       KeyBit = 0b010000
	KEY_DOWN     KeyBit = 0b100000
)

func (c *Camera) HandleKeyPress(key glfw.Key) {
	if key == glfw.KeyW {
		c.keys |= KEY_FORWARD
	}
	if key == glfw.KeyS {
		c.keys |= KEY_BACKWARD
	}
	if key == glfw.KeyA {
		c.keys |= KEY_LEFT
	}
	if key == glfw.KeyD {
		c.keys |= KEY_RIGHT
	}
	if key == glfw.KeySpace {
		c.keys |= KEY_UP
	}
	if key == glfw.KeyLeftShift {
		c.keys |= KEY_DOWN
	}
	c.Direction = c.GetDirection()
}

func (c *Camera) GetDirection() mgl.Vec3 {
	var bit KeyBit
	bit = 0b000001
	direction := mgl.Vec3{0, 0, 0}
	for i := 0; i < 6; i++ {
		if c.keys&bit != 0 {
			if bit == KEY_FORWARD {
				direction = direction.Add(c.Front.Normalize()).Normalize()
			}
			if bit == KEY_BACKWARD {
				direction = direction.Add(c.Front.Normalize().Mul(-1)).Normalize()
			}
			if bit == KEY_LEFT {
				direction = direction.Add(c.Front.Normalize().Cross(c.Up.Normalize()).Normalize().Mul(-1)).Normalize()
			}
			if bit == KEY_RIGHT {
				direction = direction.Add(c.Front.Normalize().Cross(c.Up.Normalize()).Normalize()).Normalize()
			}
			if bit == KEY_UP {
				direction = direction.Add(c.Up.Normalize()).Normalize()
			}
			if bit == KEY_DOWN {
				direction = direction.Add(c.Up.Normalize().Mul(-1)).Normalize()
			}
		}
		bit = bit << 1
	}
	if direction.X() != 0 || direction.Y() != 0 || direction.Z() != 0 {
		direction = direction.Normalize()
	}
	// log.Printf("{%f,%f,%f}\n", direction.X(), direction.Y(), direction.Z())
	return direction
}

func convertToClipCoordinates(a, ma float64) float64 {
	ndc := a / ma
	return ConvertToClip(ndc)
}

func ConvertToClip(ndc float64) float64 {
	return 2*ndc - 1
}

// convert screen coordinates to camera screen position
func (c *Camera) GetCameraScreenPosition(x, y float64) *mgl.Vec3 {
	// log.Println(fmt.Sprintf("(%f,%f)", x, y))
	// x in [0, windowWidth]
	// y in [0, windowHeight]
	if c.WindowWidth == 0 || c.WindowHeight == 0 {
		return nil
	}
	px := convertToClipCoordinates(x, c.WindowWidth) * math.Tan(c.FOV/2*math.Pi/180) * c.AspectRatio
	py := -convertToClipCoordinates(y, c.WindowHeight) * math.Tan(c.FOV/2*math.Pi/180)
	vec := mgl.Vec3{float32(px), float32(py), 0}.Add(c.Front)
	return &vec
}

func (c *Camera) HandleKeyRelease(key glfw.Key) {
	if key == glfw.KeyW {
		c.keys ^= KEY_FORWARD
	}
	if key == glfw.KeyS {
		c.keys ^= KEY_BACKWARD
	}
	if key == glfw.KeyA {
		c.keys ^= KEY_LEFT
	}
	if key == glfw.KeyD {
		c.keys ^= KEY_RIGHT
	}
	if key == glfw.KeySpace {
		c.keys ^= KEY_UP
	}
	if key == glfw.KeyLeftShift {
		c.keys ^= KEY_DOWN
	}
	c.Direction = c.GetDirection()
}

// func (c *Camera) SetAerialMode(t bool) {

// 	if t {
// 		if !c.aerialMode {
// 			// save to cache
// 			c.cache["eye"] = *c.InitialEye
// 			c.cache["front"] = c.Front
// 			c.cache["up"] = c.Up
// 			c.cache["direction"] = c.Direction
// 			// set to aerial mode
// 			c.InitialEye = &mgl.Vec3{0, 30, 0}
// 			c.Front = mgl.Vec3{0, -1, 0}
// 			c.Up = mgl.Vec3{1, 0, 0}
// 			c.Direction = mgl.Vec3{0, 0, 0}
// 			c.aerialMode = t
// 		}
// 	} else {
// 		if c.aerialMode {
// 			// load from cache
// 			c.InitialEye = &(c.cache["eye"].(mgl.Vec3))
// 			c.Front = c.cache["front"]
// 			c.Up = c.cache["up"]
// 			c.Direction = c.cache["direction"]
// 			c.aerialMode = t
// 		}
// 	}
// }
