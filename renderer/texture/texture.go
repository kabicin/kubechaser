package texture

import (
	"image"
	"image/draw"
	"os"

	"github.com/go-gl/gl/v4.1-core/gl"
)

type Texture struct {
	TextureID          uint32
	ActiveTextureIndex uint32
}

func NewTextureFromFile(file string, activeTextureIndex uint32) (*Texture, error) {
	imgFile, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	defer imgFile.Close()

	img, _, err := image.Decode(imgFile)
	if err != nil {
		return nil, err
	}
	return NewTexture(img, activeTextureIndex)
}

func NewTexture(img image.Image, activeTextureIndex uint32) (*Texture, error) {
	rgbaImage := image.NewRGBA(img.Bounds())
	draw.Draw(rgbaImage, rgbaImage.Bounds(), img, image.Point{0, 0}, draw.Src)
	flippedRGBAImage := image.NewRGBA(rgbaImage.Bounds())
	width := flippedRGBAImage.Rect.Size().X
	height := flippedRGBAImage.Rect.Size().Y
	for j := 0; j < flippedRGBAImage.Bounds().Dy(); j++ {
		for i := 0; i < flippedRGBAImage.Bounds().Dx(); i++ {
			flippedRGBAImage.Set(i, height-j, rgbaImage.At(i, j))
		}
	}

	var TextureID uint32
	gl.GenTextures(1, &TextureID)

	texture := Texture{
		TextureID:          TextureID,
		ActiveTextureIndex: activeTextureIndex,
	}
	texture.Bind()
	defer texture.Release()

	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_R, gl.REPEAT)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR)
	gl.TexParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR)
	gl.TexImage2D(gl.TEXTURE_2D, 0, gl.SRGB_ALPHA, int32(width), int32(height), 0, gl.RGBA, gl.UNSIGNED_BYTE, gl.Ptr(flippedRGBAImage.Pix))
	gl.GenerateMipmap(texture.TextureID)
	return &texture, nil
}

func (tex *Texture) Bind() {
	gl.ActiveTexture(gl.TEXTURE0 + tex.ActiveTextureIndex)
	gl.BindTexture(gl.TEXTURE_2D, tex.TextureID)
}

func (tex *Texture) Release() {
	gl.ActiveTexture(gl.TEXTURE0)
}

func (tex *Texture) SetUniform(uniformLoc int32) error {
	gl.Uniform1i(uniformLoc, int32(tex.ActiveTextureIndex-gl.TEXTURE0))
	return nil
}
