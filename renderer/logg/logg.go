package logg

import (
	"log"

	mgl "github.com/go-gl/mathgl/mgl32"
)

func PrintVec3(a mgl.Vec3) {
	log.Printf("Vec3(%f,%f,%f)\n", a.X(), a.Y(), a.Z())
}
