package entity

import (
	"log"
	"math"

	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/utils"
)

// axis aligned bounding box
type AABB struct {
	x0       float64
	x1       float64
	y0       float64
	y1       float64
	z0       float64
	z1       float64
	Children []*AABB
	Mesh     []*Tri
	IsLeaf   bool
}

func BuildLeafAABB(triangles ...*Tri) *AABB {
	if len(triangles) == 0 {
		return nil
	}
	aabb := &AABB{}
	aabb.Mesh = triangles
	aabb.IsLeaf = true
	aabb.SetMinMaxFromMesh()
	return aabb
}

func BuildAABB(children ...*AABB) *AABB {
	if len(children) == 0 {
		return nil
	}
	aabb := &AABB{}
	aabb.IsLeaf = false
	aabb.Children = children
	aabb.SetMinMaxFromChildren()
	return aabb
}

// Returns an axis aligned bounding box surrounding triangles in Tri
func GenerateAABB(triangles ...*Tri) *AABB {
	debug := false
	numTriangles := len(triangles)
	if numTriangles == 0 {
		return nil
	}
	// Build a leaf at one triangle
	if numTriangles == 1 {
		return BuildLeafAABB(triangles[0])
	}
	// Distribute triangles into halfs
	halfs := [][]*Tri{}
	halfs = append(halfs, []*Tri{})
	halfs = append(halfs, []*Tri{})

	// Find min/max based on centroids
	centroids := []*mgl.Vec3{}
	minX, maxX := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	for _, tri := range triangles {
		centroid := tri.GetCentroid()
		centroids = append(centroids, centroid)
		centroidX := float64(centroid.X())
		if centroidX < minX {
			minX = centroidX
		}
		if centroidX > maxX {
			maxX = centroidX
		}
	}
	midX := (minX + maxX) / 2.0
	// Sort into halfs based on midpoint
	for i, centroid := range centroids {
		centroidX := float64(centroid.X())
		if centroidX < midX {
			halfs[0] = append(halfs[0], triangles[i])
		} else {
			halfs[1] = append(halfs[1], triangles[i])
		}
	}
	if debug {
		log.Printf("len(halfs[0]): %d, len(halfs[1]): %d\n", len(halfs[0]), len(halfs[1]))
	}

	aabb := &AABB{}
	aabb.Children = []*AABB{}
	// populate AABB children by generating a new AABBs from the triangles distributed into each half
	// if one half is empty, create a leaf instead
	if len(halfs[0]) == 0 {
		aabb.Children = append(aabb.Children, BuildLeafAABB(halfs[1]...))
	} else if len(halfs[1]) == 0 {
		aabb.Children = append(aabb.Children, BuildLeafAABB(halfs[0]...))
	} else {
		aabb.Children = append(aabb.Children, GenerateAABB(halfs[0]...))
		aabb.Children = append(aabb.Children, GenerateAABB(halfs[1]...))
	}
	aabb.IsLeaf = false
	aabb.SetMinMaxFromChildren() // calculate min/max bounds by comparing sizes of children AABBs
	return aabb
}

func (a *AABB) SetMinMaxFromMesh() {
	x0, x1 := float64(math.MaxFloat64), float64(0)
	y0, y1 := float64(math.MaxFloat64), float64(0)
	z0, z1 := float64(math.MaxFloat64), float64(0)
	for _, tri := range a.Mesh {
		t0x, t0y, t0z := float64((*tri.P)[0].X()), float64((*tri.P)[0].Y()), float64((*tri.P)[0].Z())
		t1x, t1y, t1z := float64((*tri.P)[1].X()), float64((*tri.P)[1].Y()), float64((*tri.P)[1].Z())
		t2x, t2y, t2z := float64((*tri.P)[2].X()), float64((*tri.P)[2].Y()), float64((*tri.P)[2].Z())
		x0, x1 = min(min(min(x0, t0x), t1x), t2x), max(max(max(x1, t0x), t1x), t2x)
		y0, y1 = min(min(min(y0, t0y), t1y), t2y), max(max(max(y1, t0y), t1y), t2y)
		z0, z1 = min(min(min(z0, t0z), t1z), t2z), max(max(max(z1, t0z), t1z), t2z)
	}
	a.x0, a.x1 = x0, x1
	a.y0, a.y1 = y0, y1
	a.z0, a.z1 = z0, z1
}

func (a *AABB) SetMinMaxFromChildren() {
	x0, x1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	y0, y1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	z0, z1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	for _, bb := range a.Children {
		x0, x1 = min(bb.x0, x0), max(bb.x1, x1)
		y0, y1 = min(bb.y0, y0), max(bb.y1, y1)
		z0, z1 = min(bb.z0, z0), max(bb.z1, z1)
	}
	a.x0, a.x1 = x0, x1
	a.y0, a.y1 = y0, y1
	a.z0, a.z1 = z0, z1
}

func (a *AABB) GetMinMax() (float64, float64, float64, float64, float64, float64) {
	return a.x0, a.x1, a.y0, a.y1, a.z0, a.z1
}

func (a *AABB) GetVec3() (*mgl.Vec3, *mgl.Vec3) {
	return &mgl.Vec3{float32(a.x0), float32(a.y0), float32(a.z0)}, &mgl.Vec3{float32(a.x1), float32(a.y1), float32(a.z1)}
}

func GetAABBMinMax(children ...*AABB) (float64, float64, float64, float64, float64, float64) {
	x0, x1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	y0, y1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	z0, z1 := float64(math.MaxFloat64), float64(-math.MaxFloat64)
	for _, aabb := range children {
		x0, x1 = min(aabb.x0, x0), max(aabb.x1, x1)
		y0, y1 = min(aabb.y0, y0), max(aabb.y1, y1)
		z0, z1 = min(aabb.z0, z0), max(aabb.z1, z1)
		// log.Printf("(%f,%f), (%f,%f), (%f, %f)", x0, x1, y0, y1, z0, z1)
	}
	return x0, x1, y0, y1, z0, z1
}

func RayAABB(localToWorld *mgl.Mat4, aabb *AABB, ray *camera.Ray) (float64, bool) {
	if aabb == nil {
		return -1, false
	}
	v1, v2 := aabb.GetVec3()
	pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
	pv2 := localToWorld.Mul4x1(v2.Vec4(1)).Vec3()
	_, intersects := utils.RayAABBFromVertices(pv1, pv2, ray)
	if intersects {
		minT := float64(-1)
		if aabb.IsLeaf {
			// check triangles in mesh
			for _, tri := range aabb.Mesh {
				v1 := (*tri.P)[0]
				v2 := (*tri.P)[1]
				v3 := (*tri.P)[2]

				pv1 := localToWorld.Mul4x1(v1.Vec4(1)).Vec3()
				pv2 := localToWorld.Mul4x1(v2.Vec4(1)).Vec3()
				pv3 := localToWorld.Mul4x1(v3.Vec4(1)).Vec3()

				if tempT, n, intersects := utils.RayPlane(pv1, pv2, pv3, ray); intersects && tempT >= 0 {
					p := ray.Eye.Add(ray.Direction.Mul(float32(tempT)))
					t, intersects := utils.RayTriangle(pv1, pv2, pv3, p, *n, tempT)
					if t >= 0 && intersects && (minT == -1 || t < minT) {
						minT = t
					}
				}
			}
		} else {
			for _, child := range aabb.Children {
				t, intersects := RayAABB(localToWorld, child, ray)
				if t >= 0 && intersects && (minT == -1 || t < minT) {
					minT = t
				}
			}
		}
		if minT != -1 {
			return minT, true
		}
	}
	return -1, false
}
