package controller

import (
	"log"
	"math"

	"github.com/go-gl/glfw/v3.3/glfw"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
)

type AbstractController interface {
	KeyCallback(*glfw.Window, glfw.Key, int, glfw.Action, glfw.ModifierKey)
}

type Controller struct {
	pressed       map[glfw.Key]bool
	Camera        *camera.Camera
	clickHandlers []func(r *camera.Ray)

	width  int
	height int
	lastX  float64
	lastY  float64
	yaw    float64
	pitch  float64
	dir    mgl.Vec3
}

func (c *Controller) Init() {
	c.pressed = make(map[glfw.Key]bool)
	c.clickHandlers = make([]func(r *camera.Ray), 0)
	c.width = 1200
	c.height = 800
	c.lastX = float64(c.width / 2.0)
	c.lastY = float64(c.height / 2.0)
	c.dir = mgl.Vec3{0, 0, -1}
}

func (c *Controller) KeyCallback(w *glfw.Window, key glfw.Key, scancode int, action glfw.Action, mods glfw.ModifierKey) {
	c.updatePressed(key, scancode, action, mods)
	if action == glfw.Press && key == glfw.KeyEscape {
		// exit when escape key clicked
		if w.GetInputMode(glfw.CursorMode) == glfw.CursorDisabled {
			c.Camera.DisableCursor = true
			w.SetInputMode(glfw.CursorMode, glfw.CursorNormal)
		} else {
			w.SetInputMode(glfw.CursorMode, glfw.CursorDisabled)
			c.Camera.DisableCursor = false
		}
	}
}

func (c *Controller) CursorPosCallback(w *glfw.Window, xpos, ypos float64) {
	xOffset := xpos - c.lastX
	yOffset := c.lastY - ypos
	c.lastX = xpos
	c.lastY = ypos

	sensitivity := 0.05
	xOffset *= sensitivity
	yOffset *= sensitivity

	c.yaw += xOffset
	c.pitch += yOffset

	dir := mgl.Vec3{
		float32(math.Cos(float64(mgl.DegToRad(float32(c.yaw)))) * math.Cos(float64(mgl.DegToRad(float32(c.pitch))))),
		float32(math.Sin(float64(mgl.DegToRad(float32(c.pitch))))),
		float32(math.Sin(float64(mgl.DegToRad(float32(c.yaw)))) * math.Cos(float64(mgl.DegToRad(float32(c.pitch))))),
	}

	if c.Camera != nil {
		c.Camera.SetCameraFront(dir)
	}
}

func (c *Controller) MouseButtonCallback(w *glfw.Window, button glfw.MouseButton, action glfw.Action, mods glfw.ModifierKey) {
	if c.Camera.DisableCursor {
		w.SetInputMode(glfw.CursorMode, glfw.CursorDisabled)
		c.Camera.DisableCursor = false
	}
	if len(c.clickHandlers) > 0 {
		if button == glfw.MouseButton1 && action == glfw.Press {
			// x, y := w.GetCursorPos()
			r := &camera.Ray{}
			// r.Init(&c.Camera.Eye, c.Camera.GetCameraScreenPosition(x, y)) // ray for third person
			r.Init(c.Camera.EyeAnimator.X_init, &c.Camera.Front) // first person view
			for _, handler := range c.clickHandlers {
				handler(r)
			}
		}
	} else {
		log.Println("warning: no click handlers are registered.")
	}
}

func (c *Controller) AddClickHandler(f func(r *camera.Ray)) {
	c.clickHandlers = append(c.clickHandlers, f)
}

func (c *Controller) ScrollCallback(w *glfw.Window, xOffset float64, yOffset float64) {
	c.Camera.FOV -= yOffset
	if c.Camera.FOV < 1.0 {
		c.Camera.FOV = 1.0
	}
	if c.Camera.FOV > 90.0 {
		c.Camera.FOV = 90.0
	}
	// log.Printf("FOV: (%f)\n", c.Camera.FOV)
}

func (c *Controller) updatePressed(key glfw.Key, scancode int, action glfw.Action, mods glfw.ModifierKey) {
	if action == glfw.Press || action == glfw.Repeat {
		if c.Camera != nil {
			c.Camera.HandleKeyPress(key)

			// set aerial mode
			if key == glfw.KeyP {
				// c.Camera.SetAerialMode(true)
			}
		}
	} else if action == glfw.Release {
		if c.Camera != nil {
			for _, boundKey := range c.Camera.BoundKeys {
				if boundKey == key {
					c.Camera.HandleKeyRelease(key)
				}
			}

			// release aerial mode
			if key == glfw.KeyP {
				// c.Camera.SetAerialMode(false)
			}
		}
	}
}

func (c *Controller) Bind(camera *camera.Camera) {
	c.Camera = camera
}

func (c *Controller) Unbind(camera *camera.Camera) {
	c.Camera.MotionEvent = 0
	c.Camera = nil
}
