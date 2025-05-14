package scene

import (
	"fmt"
	"log"
	"math"

	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/entity"
	"github.com/kabicin/kubechaser/renderer/shader"
)

type Scene struct {
	Objects    []*SceneObject
	Shaders    []*shader.Program
	MainCamera *camera.Camera
}

func (s *Scene) Init(shaders []*shader.Program, objects []*SceneObject, mainCamera *camera.Camera) error {
	s.Shaders = shaders
	s.Objects = objects
	// Bind the object to shader programs
	s.bindObjectsToShaderPrograms(objects...)
	s.MainCamera = mainCamera
	return nil
}

func (s *Scene) Refresh() {

}

func (s *Scene) bindObjectsToShaderPrograms(objects ...*SceneObject) {
	// Bind the object to shader programs
	for _, object := range objects {
		for i, shader := range s.Shaders {
			if object.ShaderProgramID != nil && shader.ID == *object.ShaderProgramID {
				object.CachedShaderProgramIndex = i
			}
		}
	}
}

func (s *Scene) Click(r *camera.Ray) {
	debug := false
	if debug {
		log.Printf("Clicking in the scene at E(%f,%f,%f) and D(%f,%f,%f)\n", r.Eye.X(), r.Eye.Y(), r.Eye.Z(), r.Direction.X(), r.Direction.Y(), r.Direction.Z())
	}

	minT := float64(0)
	minNormSquared := float32(math.MaxFloat32)
	minObjectIndex := -1
	for i, so := range s.Objects {
		// model := s.MainCamera.GetModel(so.Transform)
		t, intersects := so.Object.Intersect(s.MainCamera, so.Transform, r, debug)
		rt := r.Eye.Add(r.Direction.Mul(float32(t)))
		normSquared := rt.Dot(rt)
		if intersects && t >= 0 && normSquared < minNormSquared {
			minT = t
			minObjectIndex = i
			minNormSquared = normSquared
		}
	}

	if minObjectIndex != -1 {
		log.Printf("Intersect with %s t=%f\n", s.Objects[minObjectIndex].Object.GetName(), minT)
		s.Objects[minObjectIndex].NotifyOnClickHandlers()
	}
}

func (s *Scene) Draw(deltaT float32) {
	for _, object := range s.Objects {
		if object.ShaderProgramID == nil {
			log.Println("Object " + object.Object.GetName() + " could not be drawn because there is no shader..")
			return
		}
		if object.CachedShaderProgramIndex < 0 || object.CachedShaderProgramIndex >= len(s.Shaders) {
			log.Println("Object " + object.Object.GetName() + " could not be drawn because the shader is OOB..")
			return
		}
		// handle lit objects
		var lightPos, cameraPos *mgl.Vec3
		if !object.IgnoreLights {
			// get the first light source from the other object and factor in the camera position for reflectance
			for _, otherObject := range s.Objects {
				if object != otherObject && otherObject.IsLightObject {
					lightPos = otherObject.Transform.PositionAnimator.X_init
					cameraPos = s.MainCamera.EyeAnimator.X_init
				}
			}
		} else {
			lightPos = nil
			cameraPos = nil
		}
		// animation
		object.Transform.PositionAnimator.Animate(deltaT, func() {})
		object.Draw(deltaT, object.Transform, s.Shaders[object.CachedShaderProgramIndex], s.MainCamera, lightPos, cameraPos)
	}
	// delay text render after all objects drawn
	for _, obj := range s.Objects {
		cameraRay := &camera.Ray{}
		cameraRay.Init(s.MainCamera.EyeAnimator.X_init, &s.MainCamera.Front)
		model := s.MainCamera.GetModel(obj.Transform)
		if obj.Object.GetName() == "Triangle" {
			(obj.Object.(*entity.Triangle)).DrawText(&model, cameraRay, s.MainCamera)
		}
		if obj.Object.GetName() == "Heptagon" {
			(obj.Object.(*entity.Heptagon)).DrawText(&model, cameraRay, s.MainCamera)
		}
		if obj.Object.GetName() == "TexturedCube" {
			(obj.Object.(*entity.TexturedCube)).DrawText(&model, cameraRay, s.MainCamera)
		}
		if obj.Object.GetName() == "Cube" {
			(obj.Object.(*entity.Cube)).DrawText(&model, cameraRay, s.MainCamera)
		}
		if obj.Object.GetName() == "Pyramid" {
			(obj.Object.(*entity.Pyramid)).DrawText(&model, cameraRay, s.MainCamera)
		}
	}
}

// Adds an object to the scene
func (s *Scene) AddObject(object *SceneObject) error {
	if object.ShaderProgramID != nil {
		shaderExists := false
		for _, shader := range s.Shaders {
			if shader.ID == *object.ShaderProgramID {
				shaderExists = true
				s.bindObjectsToShaderPrograms(object)
			}
		}
		if !shaderExists {
			return fmt.Errorf("could not add object %s because the shader ID=%d was not loaded", object.Object.GetName(), *object.ShaderProgramID)
		}
	}
	s.Objects = append(s.Objects, object)
	return nil
}

func (s *Scene) DeleteObject(object *SceneObject) error {
	deleteIndex := -1
	for i, obj := range s.Objects {
		if obj == object {
			deleteIndex = i
		}
	}
	if deleteIndex != -1 {
		s.Objects = append(s.Objects[:deleteIndex], s.Objects[deleteIndex+1:]...)
		return nil
	}
	return fmt.Errorf("could not delete scene object; pointer to object was not found")
}

func (s *Scene) Update() {
	s.bindObjectsToShaderPrograms(s.Objects...)
}

func (s *Scene) LoadShaders(shaders []*shader.Program) {
	s.Shaders = shaders
}
