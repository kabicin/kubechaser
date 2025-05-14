package window

import (
	"github.com/kabicin/kubechaser/renderer/scene"
)

type MainWindow struct {
	Origin    Coord
	Dimension Coord
	Hidden    bool
	Scenes    []*scene.Scene
}

func (w *MainWindow) GetOrigin() Coord {
	return w.Origin
}

func (w *MainWindow) GetDimension() Coord {
	return w.Dimension
}

func (w *MainWindow) Hide() {
	w.Hidden = true
}

func (w *MainWindow) Show() {
	w.Hidden = false
}

func (w *MainWindow) AddScenes(scenes []*scene.Scene) {
	w.Scenes = scenes
}

func (w *MainWindow) Draw(deltaT float32) {
	if w.Hidden {
		return
	}
	for _, scene := range w.Scenes {
		scene.Draw(deltaT)
	}
}

func (w *MainWindow) RefreshScenes() {
	for _, scene := range w.Scenes {
		scene.Refresh()
	}
}

func (w *MainWindow) Resize(newOrigin, newDimension Coord) {
	w.Origin = newOrigin
	w.Dimension = newDimension
}
