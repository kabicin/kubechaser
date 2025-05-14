package main

import (
	"log"
	"runtime"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/gl/v4.1-core/gl"
	"github.com/go-gl/glfw/v3.3/glfw"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/controller"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/gkube"
	"github.com/kabicin/kubechaser/renderer/scene"
	"github.com/kabicin/kubechaser/renderer/shader"
	"github.com/kabicin/kubechaser/renderer/utils"
	"github.com/kabicin/kubechaser/renderer/window"
	"github.com/kabicin/kubechaser/watcher"
)

const (
	windowWidth  = 1200
	windowHeight = 800
	windowName   = "KubeChaser"
)

func init() {
	// GLFW event handling must run on the main OS thread
	runtime.LockOSThread()
}

func createWindow(controller *controller.Controller) *glfw.Window {
	err := glfw.Init()
	if err != nil {
		panic(err)
	}

	glfw.WindowHint(glfw.Resizable, glfw.False)
	glfw.WindowHint(glfw.ContextVersionMajor, 4)
	glfw.WindowHint(glfw.ContextVersionMinor, 1)
	if runtime.GOOS == "darwin" {
		glfw.WindowHint(glfw.OpenGLProfile, glfw.OpenGLCoreProfile)
		glfw.WindowHint(glfw.OpenGLForwardCompatible, glfw.True)
	}

	glfwWindow, err := glfw.CreateWindow(windowWidth, windowHeight, windowName, nil, nil)
	if err != nil {
		panic(err)
	}

	// Bind the controller to the GLFW window
	glfwWindow.SetKeyCallback(controller.KeyCallback)
	glfwWindow.SetMouseButtonCallback(controller.MouseButtonCallback)
	glfwWindow.SetCursorPosCallback(controller.CursorPosCallback)
	glfwWindow.SetScrollCallback(controller.ScrollCallback)
	glfwWindow.SetInputMode(glfw.CursorMode, glfw.CursorDisabled)

	glfwWindow.MakeContextCurrent()
	return glfwWindow
}

func createGL() {
	if err := gl.Init(); err != nil {
		log.Fatalln(err)
	}

	renderer := gl.GoStr(gl.GetString(gl.RENDERER))
	version := gl.GoStr(gl.GetString(gl.VERSION))
	log.Println("OpenGL renderer: ", renderer)
	log.Println("OpenGL version: ", version)
}

func createMainCluster(ctrl *controller.Controller, font *v41.Font) *gkube.GCluster {
	// camera
	cam := &camera.Camera{}
	cam.Init(windowWidth, windowHeight, nil) // Initialize a perspective camera with aspect ratio windowWidth/windowHeight
	ctrl.Bind(cam)

	// shaders
	texturedCubeProgram := shader.CreateAndLoadShaders("textured-cube")
	crosshairProgram := shader.CreateAndLoadShaders("crosshair")
	mvpProgram := shader.CreateAndLoadShaders("material")
	mvpProgramStripe := shader.CreateAndLoadShaders("material-stripe")
	guiProgram := shader.CreateAndLoadShaders("gui")

	// ground
	// ground := &entity.Quad{}
	// ground.Init(font, "")
	// sceneGround := &scene.SceneObject{}
	// transform3 := &camera.Transform3D{}
	// transform3.Init(&mgl.Vec3{0, -2, 0}, &mgl.Vec3{100, 1, 100}, nil)
	// sceneGround.Init(ground, transform3, mvpProgram.ID)

	// light
	// var totalTextureCount uint32 = shader.MinShaderTextureIndex
	cube := &entity.Cube{}
	cube.Init(font, "")
	// if cube.InitWithTexture(font, "", "./deploy-128.png", totalTextureCount) {
	// 	totalTextureCount += 1
	// }
	// if cube.AddTexture("./pod-128.png", totalTextureCount) {
	// 	totalTextureCount += 1
	// }
	sceneCube := &scene.SceneObject{}
	sceneCube.Init(cube, camera.CreateTransform3D(&mgl.Vec3{-10, 10, 10}, &mgl.Vec3{1, 1, 1}, nil), texturedCubeProgram.ID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})
	sceneCube.SetLightObject(true)

	crosshair := &scene.SceneObject{}
	crosshairquad := &entity.Triangle{}
	crosshairquad.Init(font, "")
	crosshair.Init(crosshairquad, camera.CreateTransform3D(&mgl.Vec3{0, 0, 0}, &mgl.Vec3{5, 5, 0}, nil), crosshairProgram.ID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})

	gui := &scene.SceneObject{}
	guiQuad := &entity.Quad{}
	guiQuad.Init(font, "")
	gui.Init(guiQuad, camera.CreateTransform3D(&mgl.Vec3{0, -windowHeight / 3, 0}, &mgl.Vec3{windowWidth, windowHeight / 2, 0}, nil), guiProgram.ID, mgl.Vec3{0, 0, 111}, mgl.Vec3{1, 1, 1})

	frame := &scene.SceneObject{}
	framebox := &entity.ObjectFrame{}
	framebox.SetFrame(10, 20, 4, 1)
	framebox.Init(font, "")
	frame.Init(framebox, camera.CreateTransform3D(&mgl.Vec3{3, 5, 0}, &mgl.Vec3{1, 1, 1}, nil), mvpProgram.ID, mgl.Vec3{111, 111, 111}, mgl.Vec3{1, 1, 1})

	gc := &gkube.GCluster{}
	gc.Create(ctrl, cam, font, []*shader.Program{texturedCubeProgram, mvpProgram, crosshairProgram, mvpProgramStripe, guiProgram})
	// gc.GetMainScene().AddObject(sceneGround)
	gc.GetMainScene().AddObject(sceneCube)
	gc.GetMainScene().AddObject(crosshair)
	gc.GetMainScene().AddObject(gui)

	watcher := watcher.Watcher{}
	watcher.Init(gc)
	return gc
}

func main() {
	ctrl := &controller.Controller{}
	ctrl.Init()

	glfwWindow := createWindow(ctrl)
	defer glfw.Terminate()
	createGL()

	mainWindow := window.MainWindow{
		Origin:    window.Coord{X: 0, Y: 0},
		Dimension: window.Coord{X: windowWidth, Y: windowHeight},
	}

	timer := &utils.Timer{}
	timer.Init()

	// init fonts
	font := fonts.LoadFont("assets/fonts/RedHatDisplay.ttf")
	width, height := glfwWindow.GetSize()
	font.ResizeWindow(float32(width), float32(height))
	// text := fonts.CreateText("KubeChaser", font, &mgl.Vec3{0.5, 0.6, 0.3}, 0.6)

	// create scene
	cluster := createMainCluster(ctrl, font)
	// mainWindow.AddCluster(cluster)
	mainWindow.AddScenes([]*scene.Scene{cluster.GetMainScene()})

	i := 0
	gl.ClearColor(0, 0, 0, 0)
	for !glfwWindow.ShouldClose() {
		gl.Enable(gl.DEPTH_TEST)
		gl.Clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

		if i%20 == 0 {
			cluster.GC()
		}

		if i%10 == 0 {
			event, found := cluster.PopGObjectEvent()
			if found {
				if event.GetType() == gkube.GCREATE {
					cluster.AddGObject(event)
				}
				if event.GetType() == gkube.GDELETE {
					cluster.RemoveGObject(event)
				}
			}
		}

		// now := float64(glfw.GetTimerValue())
		// x := math.Sin(now/20000000) * 10.0
		// z := math.Cos(now/20000000) * 5.0
		// mainWindow.Scenes[0].Objects[0].Transform.Translate = &mgl.Vec3{float32(x), 5, float32(z)}
		// mainWindow.Scenes[0].Objects[3].Transform.Translate = &mgl.Vec3{-float32(x), 5, -float32(z)}

		// mainWindow.Scenes[0].Objects[36].Transform.Rotate = &mgl.Vec3{0, mgl.DegToRad(float32((int(now / 1000000)) % 360)), 0}

		mainWindow.Draw(float32(timer.GetElapsedTime()))
		glfwWindow.SwapBuffers()
		glfw.PollEvents()
		i += 1
	}
}
