package entity

import (
	"log"

	mgl "github.com/go-gl/mathgl/mgl32"
)

type FrameStyle int
type FrameFragmentGetter func(height, width, depth, barLength float32) [][]*mgl.Vec3

const (
	FrameStyleBorder       FrameStyle = iota
	FrameStyleBottomBorder FrameStyle = iota
	FrameStyleBottomPlane  FrameStyle = iota
)

var FrameStyleSizes map[FrameStyle]int
var FrameStyleFragments map[FrameStyle]FrameFragmentGetter

func init() {
	FrameStyleSizes = make(map[FrameStyle]int)
	FrameStyleFragments = make(map[FrameStyle]FrameFragmentGetter)

	initFrameStyleBorder(FrameStyleBorder)
	initFrameStyleBottomBorder(FrameStyleBottomBorder)
	initFrameStyleBottomPlane(FrameStyleBottomPlane)
}

// Helper to validate and set the FrameStyleSizes and FrameStyleFragments map for the specified FrameStyle
func initFrameStyle(frameStyle FrameStyle, getterFn func(width, height, depth, barLength float32) [][]*mgl.Vec3) {
	if size, ok := validateFrameStyleSize(getterFn); ok {
		FrameStyleSizes[frameStyle] = size
		FrameStyleFragments[frameStyle] = getterFn
	} else {
		log.Printf("initFrameStyle: Failed to initialize frame style for enum %d\n", frameStyle)
	}
}

// Returns the size of frame vertices and true if the translate/scale/rotate array sizes match, false otherwise
func validateFrameStyleSize(getterFn func(width, height, depth, barLength float32) [][]*mgl.Vec3) (int, bool) {
	arr := getterFn(0, 0, 0, 0)
	if len(arr) != 3 {
		// return false if translate/scale/rotate properties don't exist
		log.Println("validateFrameStyleSize: Failed. TSR array does not have length 3.")
		return 0, false
	}
	currSize := len(arr[0])
	for i := 1; i < 3; i++ {
		if currSize != len(arr[i]) {
			log.Printf("validateFrameStyleSize: Failed. TSR array at index %d does not match the prior TSR array vertices. Expected %d but got %d.\n", i, currSize, len(arr[i]))
			return 0, false
		}
	}
	return currSize, true
}

func initFrameStyleBorder(frameStyle FrameStyle) {
	initFrameStyle(frameStyle, func(width, height, depth, barLength float32) [][]*mgl.Vec3 {
		hheight := (height / 2)
		hwidth := (width / 2)
		hdepth := (depth / 2)
		return [][]*mgl.Vec3{
			// translate
			{
				{0, hheight, hdepth},   // front up bar (0, +y, +z)
				{0, -hheight, hdepth},  // front down bar (0, -y, +z)
				{-hwidth, 0, hdepth},   // front left bar (-x, 0, +z)
				{hwidth, 0, hdepth},    // front right bar (x, 0, +z)
				{-hwidth, hheight, 0},  // top left bar (-x, y, 0)
				{hwidth, hheight, 0},   // top right bar (x, y, 0)
				{-hwidth, -hheight, 0}, // bottom left bar (-x, -y, 0)
				{hwidth, -hheight, 0},  // bottom right bar (x, -y, 0)
				{0, hheight, -hdepth},  // back up bar (0, +y, -z)
				{0, -hheight, -hdepth}, // back down bar (0, -y, -z)
				{-hwidth, 0, -hdepth},  // back left bar (-x, 0, -z)
				{hwidth, 0, -hdepth},   // back right bar (x, 0, -z)
			},
			// scale
			{
				{width + barLength, barLength, barLength}, // front up bar (0, +y, +z)
				{width + barLength, barLength, barLength}, // front down bar (0, -y, +z)
				{barLength, height, barLength},            // front left bar (-x, 0, +z)
				{barLength, height, barLength},            // front right bar (x, 0, +z)
				{barLength, barLength, depth},             // top left bar (-x, y, 0)
				{barLength, barLength, depth},             // top right bar (x, y, 0)
				{barLength, barLength, depth},             // bottom left bar (-x, -y, 0)
				{barLength, barLength, depth},             // bottom right bar (x, -y, 0)
				{width + barLength, barLength, barLength}, // back up bar (0, +y, -z)
				{width + barLength, barLength, barLength}, // back down bar (0, -y, -z)
				{barLength, height, barLength},            // back left bar (-x, 0, -z)
				{barLength, height, barLength},            // back right bar (x, 0, -z)
			},
			// rotate
			{
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
			},
		}
	})
}

func initFrameStyleBottomBorder(frameStyle FrameStyle) {
	initFrameStyle(frameStyle, func(width, height, depth, barLength float32) [][]*mgl.Vec3 {
		hheight := (height / 2)
		hwidth := (width / 2)
		hdepth := (depth / 2)
		return [][]*mgl.Vec3{
			// translate
			{
				{0, -hheight, hdepth},  // front down bar (0, -y, +z)
				{-hwidth, -hheight, 0}, // bottom left bar (-x, -y, 0)
				{hwidth, -hheight, 0},  // bottom right bar (x, -y, 0)
				{0, -hheight, -hdepth}, // back down bar (0, -y, -z)
			},
			// scale
			{
				{width + barLength, barLength, barLength}, // front down bar (0, -y, +z)
				{barLength, barLength, depth},             // bottom left bar (-x, -y, 0)
				{barLength, barLength, depth},             // bottom right bar (x, -y, 0)
				{width + barLength, barLength, barLength}, // back down bar (0, -y, -z)
			},
			// rotate
			{
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0},
			},
		}
	})
}

func initFrameStyleBottomPlane(frameStyle FrameStyle) {
	initFrameStyle(frameStyle, func(width, height, depth, barLength float32) [][]*mgl.Vec3 {
		return [][]*mgl.Vec3{
			// translate
			{
				{0, -height / 2.0, 0}, // front down bar (0, -y, +z)
			},
			// scale
			{
				{width + barLength, barLength, depth + barLength}, // front down bar (0, -y, +z)
			},
			// rotate
			{
				{0, 0, 0},
			},
		}
	})
}
