package camera

import (
	mgl "github.com/go-gl/mathgl/mgl32"
)

type ColorAnimator struct {
	C_init            *mgl.Vec3
	C_final           *mgl.Vec3
	C_final_now       mgl.Vec3
	C_init_now        mgl.Vec3
	V_init            mgl.Vec3
	A                 mgl.Vec3
	EndColor          mgl.Vec3
	AnimationDuration float32
	LoopCount         int
	InMotion          bool
	Slack             float32
}

func InitColorAnimator(startColor, endColor *mgl.Vec3, loopCount int) ColorAnimator {
	ca := ColorAnimator{}
	if startColor != nil {
		ca.C_init = startColor
		ca.C_init_now = *startColor
	}
	if endColor != nil {
		ca.C_final = endColor
		ca.C_final_now = *endColor
	}
	ca.LoopCount = loopCount
	ca.InMotion = false
	ca.AnimationDuration = 0.5
	ca.V_init = mgl.Vec3{0, 0, 0}
	ca.A = mgl.Vec3{0, 0, 0}
	ca.Slack = 0.01
	return ca
}

func (ca *ColorAnimator) Animate(deltaT float32, doneCallback func()) *mgl.Vec3 {
	isResting := vec3Equal(ca.C_init, &ca.C_final_now, &ca.Slack)
	if ca.AnimationDuration != 0 && !isResting {
		if !ca.InMotion {
			ca.V_init = ca.C_final_now.Sub(*ca.C_init).Mul(1 / float32(ca.AnimationDuration))
			ca.InMotion = true
		} else {
			nextColor := ca.V_init.Mul(deltaT).Add(*ca.C_init)
			ca.C_init = clampVec3(&nextColor, mgl.Vec3{0, 0, 0}, mgl.Vec3{1, 1, 1})
			// logg.PrintVec3(*ca.C_init)
		}
	} else if isResting && ca.InMotion {
		if ca.LoopCount != 0 {
			// swap animation to reverse
			oldFinalNow := ca.C_final_now
			ca.C_final_now = ca.C_init_now
			ca.C_init_now = oldFinalNow
			if ca.LoopCount > 0 {
				ca.LoopCount--
				if ca.LoopCount == 0 {
					ca.C_final_now = mgl.Vec3{0, 0, 0}
				}
			}
		} else {
			doneCallback()
		}
		ca.InMotion = false
	}
	return ca.C_init
}
