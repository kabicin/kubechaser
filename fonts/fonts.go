package fonts

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/4ydx/gltext"
	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"golang.org/x/image/math/fixed"
)

const fontFolder = "fontconfigs"

func getFontConfigName(fontFile string) string {
	folderName := strings.Replace(fontFile, "/", "_", -1)
	ext := filepath.Ext(folderName)
	return folderName[:len(folderName)-len(ext)]
}

const ScaleMin = float32(0.1)
const ScaleMax = float32(1)

func LoadFont(fontFile string) *v41.Font {
	var font *v41.Font
	fontConfigName := getFontConfigName(fontFile)
	config, err := gltext.LoadTruetypeFontConfig(fontFolder, fontConfigName)
	if err == nil {
		font, err = v41.NewFont(config)
		if err != nil {
			panic(err)
		}
		fmt.Println("Font loaded from disk...")
	} else {
		fd, err := os.Open(fontFile)
		if err != nil {
			panic(err)
		}
		defer fd.Close()

		runeRanges := make(gltext.RuneRanges, 0)
		runeRanges = append(runeRanges, gltext.RuneRange{Low: 32, High: 128})

		scale := fixed.Int26_6(32)
		runesPerRow := fixed.Int26_6(128)
		config, err = gltext.NewTruetypeFontConfig(fd, scale, runeRanges, runesPerRow, 5)
		if err != nil {
			panic(err)
		}
		err = config.Save(fontFolder, fontConfigName)
		if err != nil {
			panic(err)
		}
		font, err = v41.NewFont(config)
		if err != nil {
			panic(err)
		}
	}
	return font
}

func CreateText(str string, font *v41.Font, color *mgl.Vec3, scale float32) *v41.Text {
	if font == nil || len(str) == 0 {
		return nil
	}
	text := v41.NewText(font, ScaleMin, ScaleMax)
	text.SetString(str)
	if color == nil {
		text.SetColor(mgl.Vec3{1, 1, 1})
	} else {
		text.SetColor(*color)
	}
	text.SetScale(scale)
	// text.FadeOutPerFrame = 0.01
	if gltext.IsDebug {
		for _, s := range str {
			fmt.Printf("%c: %d\n", s, rune(s))
		}
	}
	return text
}
