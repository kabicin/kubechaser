package camera

import mgl "github.com/go-gl/mathgl/mgl32"

type Ray struct {
	Eye       *mgl.Vec3
	Direction *mgl.Vec3
}

func (r *Ray) Init(eye *mgl.Vec3, direction *mgl.Vec3) {
	r.Eye = eye
	r.Direction = direction
}
