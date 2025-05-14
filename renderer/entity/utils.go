package entity

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	mgl "github.com/go-gl/mathgl/mgl32"
)

func ReadOBJ(filePath string) error {
	file, err := os.Open(filePath)
	if err != nil {
		return err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Split(bufio.ScanLines)
	var lines []string

	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	for _, line := range lines {
		fmt.Printf("Line: %s\n", line)
	}

	return nil
}

func parse2FloatsFromString(floatString string) (float32, float32, error) {
	coords := strings.TrimSpace(floatString)
	if !strings.Contains(coords, " ") {
		// error, there are no coordinates in this vertex
		return 0, 0, fmt.Errorf("there are no coordinates in this vertex")
	}

	// get coordinates of vertex
	coordsArr := strings.Split(coords, " ")
	if len(coordsArr) != 3 {
		// skip, this is not a vertex
		return 0, 0, fmt.Errorf("this vertex does not have 3 values")
	}

	x, _ := strconv.ParseFloat(coordsArr[0], 32)
	y, _ := strconv.ParseFloat(coordsArr[1], 32)
	return float32(x), float32(y), nil
}

func parse3FloatsFromString(floatString string) (float32, float32, float32, error) {
	coords := strings.TrimSpace(floatString)
	if !strings.Contains(coords, " ") {
		// error, there are no coordinates in this vertex
		return 0, 0, 0, fmt.Errorf("there are no coordinates in this vertex")
	}

	// get coordinates of vertex
	coordsArr := strings.Split(coords, " ")
	if len(coordsArr) != 3 {
		// skip, this is not a vertex
		return 0, 0, 0, fmt.Errorf("this vertex does not have 3 values it has %d: val is : '%s'\n", len(coordsArr), coords)
	}

	x, _ := strconv.ParseFloat(coordsArr[0], 32)
	y, _ := strconv.ParseFloat(coordsArr[1], 32)
	z, _ := strconv.ParseFloat(coordsArr[2], 32)
	return float32(x), float32(y), float32(z), nil
}

func LoadTrianglesFromOBJ(filePath string) ([]*Tri, error) {
	triangles := make([]*Tri, 0)

	file, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	s := bufio.NewScanner(file)
	s.Split(bufio.ScanLines)
	var line string

	vs := make([]*mgl.Vec3, 0)
	validVs := make([]bool, 0)

	vns := make([]*mgl.Vec3, 0)
	validVns := make([]bool, 0)

	vts := make([]*mgl.Vec2, 0)
	validVts := make([]bool, 0)

	for s.Scan() {
		fmt.Printf("line: %s\n", line)
		line = strings.TrimSpace(s.Text())
		// Check for vertices 'v <x>,<y>,<z>'
		if strings.HasPrefix(line, "v ") {
			x, y, z, err := parse3FloatsFromString(line[len("v "):])
			if err != nil {
				vs = append(vs, nil)
				validVs = append(validVs, false)
			} else {
				tri := mgl.Vec3{x, y, z}
				vs = append(vs, &tri)
				validVs = append(validVs, true)
			}
		} else if strings.HasPrefix(line, "vn ") {
			x, y, z, err := parse3FloatsFromString(line[len("vn "):])
			if err != nil {
				vns = append(vns, nil)
				validVns = append(validVns, false)
			} else {
				dir := mgl.Vec3{x, y, z}
				vns = append(vns, &dir)
				validVns = append(validVns, true)
			}
		} else if strings.HasPrefix(line, "vt ") {
			x, y, err := parse2FloatsFromString(line[len("vt "):])
			if err != nil {
				vts = append(vts, nil)
				validVts = append(validVts, false)
			} else {
				tex := mgl.Vec2{x, y}
				vts = append(vts, &tex)
				validVts = append(validVts, true)
			}
		} else if strings.HasPrefix(line, "f ") {
			// TODO: implement faces with textures too
			facesLine := strings.TrimSpace(line[len("f "):])
			faceVertices := strings.Split(facesLine, " ")
			// assume vertex and normals only
			seenVertices := 0
			lastLastVertex := mgl.Vec3{}
			lastVertex := mgl.Vec3{}
			currentVertex := mgl.Vec3{}
			for _, faceVertex := range faceVertices {
				seenVertices += 1
				// face = "1/2/3" // vs[1], vts[2], vns[3]
				faceVertex = strings.TrimSpace(faceVertex)
				faceArr := strings.Split(faceVertex, "/")
				vsIndex, _ := strconv.ParseInt(faceArr[0], 10, 64)
				vsIndex -= 1 // offset OBJ's 1-based index
				if seenVertices == 1 {
					lastLastVertex = *vs[vsIndex]
				} else if seenVertices == 2 {
					lastVertex = *vs[vsIndex]
				} else if seenVertices == 3 {
					currentVertex = *vs[vsIndex]
					// create Tri
					triangles = append(triangles, &Tri{P: &[]mgl.Vec3{lastLastVertex, lastVertex, currentVertex}})
				} else if seenVertices > 3 { // fan and pivot on lastLastVertex
					lastVertex = currentVertex
					currentVertex = *vs[vsIndex]
					triangles = append(triangles, &Tri{P: &[]mgl.Vec3{lastLastVertex, lastVertex, currentVertex}})
				}
			}
		}
	}
	return triangles, nil
}

func LoadNTrianglesFromOBJ(filePath string) ([]*NTri, error) {
	triangles := make([]*NTri, 0)

	file, err := os.Open(filePath)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	s := bufio.NewScanner(file)
	s.Split(bufio.ScanLines)
	var line string

	vs := make([]*mgl.Vec3, 0)
	validVs := make([]bool, 0)

	vns := make([]*mgl.Vec3, 0)
	validVns := make([]bool, 0)

	vts := make([]*mgl.Vec2, 0)
	validVts := make([]bool, 0)

	for s.Scan() {
		fmt.Printf("line: %s\n", line)
		line = strings.TrimSpace(s.Text())
		// Check for vertices 'v <x>,<y>,<z>'
		if strings.HasPrefix(line, "v ") {
			x, y, z, err := parse3FloatsFromString(line[len("v "):])
			if err != nil {
				vs = append(vs, nil)
				validVs = append(validVs, false)
			} else {
				tri := mgl.Vec3{x, y, z}
				vs = append(vs, &tri)
				validVs = append(validVs, true)
			}
		} else if strings.HasPrefix(line, "vn ") {
			x, y, z, err := parse3FloatsFromString(line[len("vn "):])
			if err != nil {
				vns = append(vns, nil)
				validVns = append(validVns, false)
				fmt.Println("ERROR parsing vns:")
				fmt.Printf("%+v\n", err)
			} else {
				dir := mgl.Vec3{x, y, z}
				vns = append(vns, &dir)
				validVns = append(validVns, true)
			}
		} else if strings.HasPrefix(line, "vt ") {
			x, y, err := parse2FloatsFromString(line[len("vt "):])
			if err != nil {
				vts = append(vts, nil)
				validVts = append(validVts, false)
			} else {
				tex := mgl.Vec2{x, y}
				vts = append(vts, &tex)
				validVts = append(validVts, true)
			}
		} else if strings.HasPrefix(line, "f ") {
			// TODO: implement faces with textures too
			facesLine := strings.TrimSpace(line[len("f "):])
			faceVertices := strings.Split(facesLine, " ")

			// assume vertex and normals only
			seenVertices := 0
			lastLastVertex := mgl.Vec3{}
			lastVertex := mgl.Vec3{}
			currentVertex := mgl.Vec3{}

			lastLastNormal := mgl.Vec3{}
			lastNormal := mgl.Vec3{}
			currentNormal := mgl.Vec3{}

			for _, faceVertex := range faceVertices {
				seenVertices += 1
				// faceVertex = "1/2/3" // vs[1], vts[2], vns[3]
				faceVertex = strings.TrimSpace(faceVertex)
				faceArr := strings.Split(faceVertex, "/")

				// assume len(faceArr) >= 3 so I can use face normals
				if len(faceArr) < 3 {
					msg := "ERROR: Expected WavefrontOBJ to use face normals in face elements... not reading this file"
					return nil, fmt.Errorf(msg)
				}

				vsIndex, _ := strconv.ParseInt(faceArr[0], 10, 64)
				vsIndex -= 1 // offset OBJ's 1-based index

				vnsIndex, _ := strconv.ParseInt(faceArr[2], 10, 64)
				vnsIndex -= 1 // offset OBJ's 1-based index
				fmt.Printf("arr size: %d and getting index %d\n", len(vns), vnsIndex)

				if seenVertices == 1 {
					lastLastVertex = *vs[vsIndex]
					if validVns[vnsIndex] {
						lastLastNormal = *vns[vnsIndex]
					} else {
						fmt.Printf("WARNING: missing a normal at index %d/%d\n", vnsIndex, len(vns))
					}
				} else if seenVertices == 2 {
					lastVertex = *vs[vsIndex]
					lastNormal = *vns[vnsIndex]
				} else if seenVertices == 3 {
					currentVertex = *vs[vsIndex]
					currentNormal = *vns[vnsIndex]
					// create Tri
					triangles = append(triangles, &NTri{P: &[]mgl.Vec3{lastLastVertex, lastVertex, currentVertex}, N: &[]mgl.Vec3{lastLastNormal, lastNormal, currentNormal}})
				} else if seenVertices > 3 { // fan and pivot on lastLastVertex
					lastVertex = currentVertex
					currentVertex = *vs[vsIndex]
					lastNormal = currentNormal
					currentNormal = *vns[vnsIndex]
					triangles = append(triangles, &NTri{P: &[]mgl.Vec3{lastLastVertex, lastVertex, currentVertex}, N: &[]mgl.Vec3{lastLastNormal, lastNormal, currentNormal}})
				}
			}
		}
	}
	return triangles, nil
}
