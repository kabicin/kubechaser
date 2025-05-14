package camera

import mgl "github.com/go-gl/mathgl/mgl32"

type AnimatorObject interface {
	Animate(float32, func()) *mgl.Vec3
}

func abs(v float32) float32 {
	if v < 0 {
		return -v
	}
	return v
}

func vec3Equal(v1 *mgl.Vec3, v2 *mgl.Vec3, epsilon *float32) bool {
	e := float32(0.01)
	if epsilon != nil {
		e = *epsilon
	}
	return abs(v1.X()-v2.X()) < e && abs(v1.Y()-v2.Y()) < e && abs(v1.Z()-v2.Z()) < e
}

func clampVec3(o *mgl.Vec3, low mgl.Vec3, high mgl.Vec3) *mgl.Vec3 {
	for i := range 3 {
		if o[i] < low[i] {
			o[i] = low[i]
		}
		if o[i] > high[i] {
			o[i] = high[i]
		}
	}
	return o
}
