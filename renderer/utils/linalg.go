package utils

import (
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
)

// return the Vec3 hadamard product of two Vec3's a and b
func HadamardProd(a, b mgl.Vec3) mgl.Vec3 {
	return mgl.Vec3{a.X() * b.X(), a.Y() * b.Y(), a.Z() * b.Z()}
}

// copies vec a
func CopyVec3(a *mgl.Vec3) *mgl.Vec3 {
	var c *mgl.Vec3
	if a != nil {
		c = &mgl.Vec3{a.X(), a.Y(), a.Z()}
	}
	return c
}

// For some object o1 (vec3) in world space.
// there is a camera ray designated by P=e+td, for some time t, eye e, direction d and point P.
// WorldToClip will find the x, y in [-aspectRatio, aspectRatio] and [-1, 1] respectively in clip space
// obtained by the vector from the camera's eye towards object o1 projected on the screen's plane.
//
// For projection onto the screen's plane, I use cameraUp (which should be normalized),
// that is used to determine cameraRight with cross product d.Cross(cameraUp).Normalize()
// Then, cameraUp and cameraRight are used to determine scalar projections of the vector within the screen plane onto the x and y axes, respectively.
//
// Returns x, y, the distance of object o1 to the eye (l.Len()), true if clipped and false if culled
func WorldToClip(o1 mgl.Vec3, ray *camera.Ray, cameraUp *mgl.Vec3, aspectRatio float32, cameraNear float32) (float32, float32, float32, bool) {
	// logg.PrintVec3(o1)
	// l is the vector from the cameraEye to vertex o1
	l := o1.Sub(*ray.Eye)
	ll := l.Len()
	if ll <= cameraNear {
		// stop if the distance between the camera's eye and the object is smaller than the camera's near value
		// this would mean that the object is behind the camera or placed directly on the lens
		return 0, 0, 0, false
	}

	// pv1 is a point on the screen plane
	pv1 := (*ray.Eye).Add(*ray.Direction)
	n := (*ray.Direction).Normalize()
	newRay := &camera.Ray{}
	newRay.Init(ray.Eye, &l)

	t, _, intersects := RayPlaneFromPointAndNormal(pv1, n, newRay)
	if !intersects {
		// log.Println("No intersection with screen")
		return 0, 0, l.Len(), false
	}

	// os1 is the vertex intersecting the screen plane with newRay
	os1 := (*newRay.Eye).Add((*newRay.Direction).Mul(float32(t)))

	// d1 is the vector travelling within screen plane
	d1 := os1.Sub(pv1)
	cameraRight := (*ray.Direction).Cross(*cameraUp).Normalize().Mul(aspectRatio)

	// project d1 onto cameraUp to get y component
	x := ScalarProjAOnB(d1, cameraRight)
	// project d1 onto cameraRight to get x component
	y := ScalarProjAOnB(d1, *cameraUp)

	xBounds := x >= -aspectRatio && x <= aspectRatio
	yBounds := y >= -1 && y <= 1
	return x, y, l.Len(), xBounds && yBounds
}

// This func scales a clip coordinate
// -  x in [-aspectRatio, aspectRatio]
// -  y in [-1,1]
// into clipped screen coordinates
// - outputx in [-w/2,w/2]
// - outputy in [-h/2,h/2]
func ClipToScreen(x, y, w, h float32) (float32, float32) {
	return (w / (2.0 * (w / h))) * x, (h / 2.0) * y
}

const maxD = 100

func DistanceToFontScale(d, minScale, maxScale float32) float32 {
	if d > maxD {
		return minScale
	}
	// d in [0, maxD]
	// and I want output in [minScale, maxScale]
	diff := maxScale - minScale
	nd := 1 - (d / maxD) // get norm of d w.r.t maxD, and I want nd to be inversely related to scale size (i.e flip 1-norm to get opposite range [1, 0])
	return (nd * diff) + minScale
}

func ScalarProjAOnB(a, b mgl.Vec3) float32 {
	bn := b.Normalize()
	return a.Dot(bn)
}

func VectorProjAOnB(a, b mgl.Vec3) mgl.Vec3 {
	return b.Mul(a.Dot(b) / b.Dot(b))
}

// checks ray intersection for some plane that includes point pv1 with normal n
func RayPlaneFromPointAndNormal(pv1, n mgl.Vec3, ray *camera.Ray) (float64, *mgl.Vec3, bool) {
	nd := n.Dot(*ray.Direction)
	t := float64(-1)
	if nd != 0 {
		tempT := (n.Dot(pv1) - n.Dot(*ray.Eye)) / nd
		if tempT >= 0 {
			return float64(tempT), &n, true
		}
	}
	return t, nil, false
}

func RayPlane(pv1, pv2, pv3 mgl.Vec3, ray *camera.Ray) (float64, *mgl.Vec3, bool) {
	r1 := pv1.Sub(pv2)
	r2 := pv3.Sub(pv2)
	n := r2.Cross(r1).Normalize()
	return RayPlaneFromPointAndNormal(pv1, n, ray)
}

func RayTriangle(pv1, pv2, pv3, p, n mgl.Vec3, t float64) (float64, bool) {
	s1 := (pv3.Sub(pv2)).Cross(p.Sub(pv2))
	s2 := (pv1.Sub(pv3)).Cross(p.Sub(pv3))
	s3 := (pv2.Sub(pv1)).Cross(p.Sub(pv1))
	if s1.Dot(n) >= 0 && s2.Dot(n) >= 0 && s3.Dot(n) >= 0 {
		return float64(t), true
	}
	return 0, false
}

func RayQuad(pv1, pv2, pv3, pv4, p, n mgl.Vec3, t float64) (float64, bool) {
	t1, i1 := RayTriangle(pv1, pv2, pv3, p, n, t)
	if i1 {
		return t1, true
	}
	t2, i2 := RayTriangle(pv1, pv3, pv4, p, n, t)
	if i2 {
		return t2, true
	}
	return 0, false
}

func RayAABBFromMinMax(x0, x1, y0, y1, z0, z1 float32, ray *camera.Ray) (float64, bool) {
	if ray.Direction.Z() != 0 {
		// xy-plane 1
		t0 := (z0 - ray.Eye.Z()) / ray.Direction.Z()
		p0 := ray.Eye.Add(ray.Direction.Mul(t0))
		if t0 >= 0 && checkHit(p0.X(), p0.Y(), x0, x1, y0, y1) {
			return float64(t0), true
		}
		// xy-plane 2
		t1 := (z1 - ray.Eye.Z()) / ray.Direction.Z()
		p1 := ray.Eye.Add(ray.Direction.Mul(t1))
		if t1 >= 0 && checkHit(p1.X(), p1.Y(), x0, x1, y0, y1) {
			return float64(t1), true
		}
	}
	if ray.Direction.Y() != 0 {
		// xz-plane 1
		t0 := (y0 - ray.Eye.Y()) / ray.Direction.Y()
		p0 := ray.Eye.Add(ray.Direction.Mul(t0))
		if t0 >= 0 && checkHit(p0.X(), p0.Z(), x0, x1, z0, z1) {
			return float64(t0), true
		}
		// xz-plane 2
		t1 := (y1 - ray.Eye.Y()) / ray.Direction.Y()
		p1 := ray.Eye.Add(ray.Direction.Mul(t0))
		if t1 >= 0 && checkHit(p1.X(), p1.Z(), x0, x1, z0, z1) {
			return float64(t1), true
		}
	}
	if ray.Direction.X() != 0 {
		// yz-plane 1
		t0 := (x0 - ray.Eye.X()) / ray.Direction.X()
		p0 := ray.Eye.Add(ray.Direction.Mul(t0))
		if t0 >= 0 && checkHit(p0.Y(), p0.Z(), y0, y1, z0, z1) {
			return float64(t0), true
		}
		// yz-plane 2
		t1 := (x1 - ray.Eye.X()) / ray.Direction.X()
		p1 := ray.Eye.Add(ray.Direction.Mul(t0))
		if t1 >= 0 && checkHit(p1.Y(), p1.Z(), y0, y1, z0, z1) {
			return float64(t1), true
		}
	}
	return -1, false
}

func RayAABBFromVertices(pv1, pv2 mgl.Vec3, ray *camera.Ray) (float64, bool) {
	x0 := float32(min(float64(pv1.X()), float64(pv2.X())))
	x1 := float32(max(float64(pv1.X()), float64(pv2.X())))
	y0 := float32(min(float64(pv1.Y()), float64(pv2.Y())))
	y1 := float32(max(float64(pv1.Y()), float64(pv2.Y())))
	z0 := float32(min(float64(pv1.Z()), float64(pv2.Z())))
	z1 := float32(max(float64(pv1.Z()), float64(pv2.Z())))
	return RayAABBFromMinMax(x0, x1, y0, y1, z0, z1, ray)
}

func checkHit(a, b, mina, maxa, minb, maxb float32) bool {
	return a >= mina && a <= maxa && b >= minb && b <= maxb
}
