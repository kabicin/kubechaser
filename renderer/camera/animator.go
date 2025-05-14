package camera

import mgl "github.com/go-gl/mathgl/mgl32"

type Animator struct {
	X_init      *mgl.Vec3
	X_final     *mgl.Vec3
	X_final_now mgl.Vec3
	V_init      mgl.Vec3
	A           mgl.Vec3

	AnimationDuration int
	InMotion          bool
}

func InitAnimator(x_init *mgl.Vec3, x_final *mgl.Vec3) Animator {
	anim := Animator{}
	if x_init != nil {
		anim.X_init = x_init
	}
	if x_final != nil {
		anim.X_final = x_final
	}
	anim.V_init = mgl.Vec3{0, 0, 0}
	anim.A = mgl.Vec3{0, 0, 0}
	anim.AnimationDuration = 10
	anim.InMotion = false
	return anim
}

func PatchNewAnimator(init Animator, addon Animator) Animator {
	if addon.X_init != nil {
		init.X_init = addon.X_init
	}
	if addon.X_final != nil {
		init.X_final = addon.X_final
	}
	if !vec3Equal(&init.X_final_now, &addon.X_final_now, nil) {
		init.X_final_now = addon.X_final_now
	}
	init.V_init = mgl.Vec3{0, 0, 0}
	init.A = mgl.Vec3{0, 0, 0}
	init.AnimationDuration = 10
	init.InMotion = false
	return init
}

func (a *Animator) Animate(deltaT float32, callbackWhileAnimating func()) *mgl.Vec3 {
	isResting := vec3Equal(a.X_init, a.X_final, nil)
	if a.AnimationDuration != 0 && !isResting {
		callbackWhileAnimating()
		if !a.InMotion {
			a.A = a.X_final.Sub(*a.X_init).Mul(2.0 / float32(a.AnimationDuration*a.AnimationDuration))
			a.X_final_now = *a.X_final
			a.V_init = mgl.Vec3{0, 0, 0}
			a.InMotion = true
		} else {
			// if the final displacement was modified, alter the acceleration and initial velocity
			translatingInSameDirection := vec3Equal(a.X_final, &a.X_final_now, nil)
			if !translatingInSameDirection {
				a.A = a.X_final.Sub(*a.X_init).Mul(2.0 / float32(a.AnimationDuration*a.AnimationDuration))
				a.V_init = mgl.Vec3{0, 0, 0}
				a.X_final_now = *a.X_final
			}
			deltaT2 := float32(deltaT) * deltaT
			nextInitialTranslate := a.V_init.Mul(deltaT).Add(a.X_init.Add(a.A.Mul(deltaT2 / 2.0)))
			a.X_init = clampVec3(&nextInitialTranslate, *a.X_init, *a.X_final) // TODO: fix clamp issue only working from -x,-y,-z  to +x,+y,+z
			a.V_init = a.V_init.Add(a.A.Mul(deltaT))
		}
		// translate the object xt = (1/2)*accel*deltaT^2
		// fmt.Printf("DELTA T: %f\n", deltaT)
		// currTranslate := object.transform.PositionAnimator.X_init.Add(object.Transform.Acceleration.Mul(float32(deltaT) * deltaT / 2.0))
		// object.transform.PositionAnimator.X_init = &currTranslate
		// fmt.Println("TRANSLATING ANIMATION!!!: ")
		// logg.PrintVec3(*object.transform.PositionAnimator.X_init)
		// fmt.Println("SET VECTOR ANIMATION DIRECTION FROM ")
		// fmt.Printf("   - from ")
		// logg.PrintVec3(*object.transform.PositionAnimator.X_init)
		// fmt.Printf("   - to ")
		// logg.PrintVec3(*object.Transform.FinalTranslate)
	} else if isResting && a.InMotion {
		a.InMotion = false
	}
	return nil
}
