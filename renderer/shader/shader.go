package shader

import (
	"fmt"

	"github.com/go-gl/gl/v4.1-core/gl"
)

type Program struct {
	Name    string
	ID      uint32
	Shaders []Shader
}

type Shader struct {
	Meta ShaderMetadata
	ID   uint32
}

type ShaderMetadata struct {
	FilePath string
	Type     uint32
}

const SHADER_FILE_DIR = "./renderer/shader/files"

func CreateAndLoadShaders(baseName string) *Program {
	prog := &Program{Name: baseName}
	vs := ShaderMetadata{FilePath: fmt.Sprintf("%s/%s.vs", SHADER_FILE_DIR, baseName), Type: gl.VERTEX_SHADER}
	fs := ShaderMetadata{FilePath: fmt.Sprintf("%s/%s.fs", SHADER_FILE_DIR, baseName), Type: gl.FRAGMENT_SHADER}
	prog.LoadShaders(vs, fs)
	return prog
}

func (p *Program) GetUniformLocation(variableName string) int32 {
	return gl.GetUniformLocation(p.ID, gl.Str(fmt.Sprintf("%s\x00", variableName)))
}

func (p *Program) LoadShaders(shaders ...ShaderMetadata) {
	p.ID = gl.CreateProgram()

	for _, shader := range shaders {
		compiledShader, err := compileShaderFromFile(shader.FilePath, shader.Type)
		if err != nil {
			panic(err)
		}
		newShader := Shader{
			Meta: shader,
			ID:   compiledShader,
		}
		p.Shaders = append(p.Shaders, newShader)
		gl.AttachShader(p.ID, compiledShader)
		gl.DeleteShader(compiledShader)
	}
	gl.LinkProgram(p.ID)
	fmt.Printf("Loaded shader %s with ID: %s\n", p.Name, fmt.Sprint(p.ID))
}

func (p *Program) DetachShader(shaderID uint32) {
	removeIndex := -1
	for i, shader := range p.Shaders {
		if shader.ID == shaderID {
			gl.DetachShader(p.ID, shader.ID)
			removeIndex = i
		}
	}
	if removeIndex != -1 {
		p.Shaders = append(p.Shaders[:removeIndex], p.Shaders[removeIndex+1:]...)
	}
}

func (p *Program) Delete() {
	for _, shader := range p.Shaders {
		gl.DeleteShader(shader.ID)
	}
	gl.DeleteProgram(p.ID)
}
