package shader

import (
	"fmt"
	"strings"

	"github.com/go-gl/gl/v4.1-core/gl"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

var MinShaderTextureIndex uint32 = 1
var MaxShaderTextureIndex uint32 = 16

func compileShaderFromFile(fileName string, shaderType uint32) (uint32, error) {
	content, err := utils.GetFileSource(fileName)
	if err != nil {
		return 0, err
	}
	return compileShader(content+"\x00", shaderType)
}

func compileShader(source string, shaderType uint32) (uint32, error) {
	shader := gl.CreateShader(shaderType)

	csources, free := gl.Strs(source)
	gl.ShaderSource(shader, 1, csources, nil)
	free()
	gl.CompileShader(shader)

	var status int32
	gl.GetShaderiv(shader, gl.COMPILE_STATUS, &status)
	if status == gl.FALSE {
		var logLength int32
		gl.GetShaderiv(shader, gl.INFO_LOG_LENGTH, &logLength)

		log := strings.Repeat("\x00", int(logLength+1))
		gl.GetShaderInfoLog(shader, logLength, nil, gl.Str(log))

		return 0, fmt.Errorf("failed to compile %v: %v", source, log)
	}

	return shader, nil
}

func (program *Program) SetUniforms(cam *camera.Camera, lightPos *mgl.Vec3, cameraPos *mgl.Vec3, color mgl.Vec3, onClick bool, onClickColor mgl.Vec3) {
	gl.UniformMatrix4fv(program.GetUniformLocation("Model"), 1, false, &cam.Model[0])
	gl.UniformMatrix4fv(program.GetUniformLocation("View"), 1, false, &cam.View[0])
	gl.UniformMatrix4fv(program.GetUniformLocation("Projection"), 1, false, &cam.Projection[0])
	if program.Name == "crosshair" || program.Name == "gui" {
		gl.UniformMatrix4fv(program.GetUniformLocation("OrthoProjection"), 1, false, &cam.OrthoProjection[0])
	}
	gl.Uniform3f(program.GetUniformLocation("objectColor"), 0.224, 0.439, 0.894)
	gl.Uniform3f(program.GetUniformLocation("lightColor"), 1.0, 1.0, 1.0)
	if onClick {
		gl.Uniform1i(program.GetUniformLocation("onClick"), 1)
	} else {
		gl.Uniform1i(program.GetUniformLocation("onClick"), 0)
	}
	gl.Uniform3f(program.GetUniformLocation("onClickColor"), onClickColor.X(), onClickColor.Y(), onClickColor.Z())
	if lightPos != nil {
		gl.Uniform3f(program.GetUniformLocation("light.position"), lightPos.X(), lightPos.Y(), lightPos.Z())
		gl.Uniform3f(program.GetUniformLocation("light.direction"), -0.2, -1.0, -0.3)
		lightColor := mgl.Vec3{1, 1, 1}
		diffuseColor := lightColor.Mul(0.9)
		ambientColor := diffuseColor.Mul(0.5)
		gl.Uniform3f(program.GetUniformLocation("light.diffuse"), diffuseColor.X(), diffuseColor.Y(), diffuseColor.Z())
		gl.Uniform3f(program.GetUniformLocation("light.ambient"), ambientColor.X(), ambientColor.Y(), ambientColor.Z())
		gl.Uniform3f(program.GetUniformLocation("light.specular"), 1.0, 1.0, 1.0)
	}
	if cameraPos != nil {
		gl.Uniform3f(program.GetUniformLocation("cameraPos"), cameraPos.X(), cameraPos.Y(), cameraPos.Z())
	}

	startingShaderIndex := int32(MinShaderTextureIndex)
	if program.Name == "textured-cube" {
		gl.Uniform1i(program.GetUniformLocation("material.diffuse1"), startingShaderIndex)
		gl.Uniform1i(program.GetUniformLocation("material.diffuse2"), startingShaderIndex+1)
		gl.Uniform1i(program.GetUniformLocation("material.specular"), startingShaderIndex+2)
		// gl.Uniform3f(program.GetUniformLocation("material.specular"), 1, 1, 1)
		gl.Uniform1f(program.GetUniformLocation("material.shininess"), 32)
	} else {
		gl.Uniform3f(program.GetUniformLocation("material.ambient"), color.X(), color.Y(), color.Z())
		gl.Uniform3f(program.GetUniformLocation("material.diffuse"), color.X(), color.Y(), color.Z())

		gl.Uniform3f(program.GetUniformLocation("material.specular"), 0.5, 0.5, 0.5)
		gl.Uniform1f(program.GetUniformLocation("material.shininess"), 32)
		gl.Uniform1i(program.GetUniformLocation("texture1"), startingShaderIndex)
		gl.Uniform1i(program.GetUniformLocation("texture2"), startingShaderIndex+1)
	}
}
