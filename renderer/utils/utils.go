package utils

import (
	"crypto/sha256"
	"fmt"
	"log"
	"os"
	"reflect"
	"strings"

	v41 "github.com/4ydx/gltext/v4.1"
	"github.com/go-gl/glfw/v3.3/glfw"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/fonts"
	"github.com/kabicin/kubechaser/renderer/camera"
)

type Timer struct {
	LastTime float64
}

func (t *Timer) Init() {
	t.LastTime = glfw.GetTime()
}

func (t *Timer) GetElapsedTime() float64 {
	now := glfw.GetTime()
	elapsed := now - t.LastTime
	t.LastTime = now
	return elapsed
}

func GetFileSource(filePath string) (string, error) {
	contents, err := os.ReadFile(filePath)
	return string(contents), err
}

func UpdateDrawText(pv1 mgl.Vec3, cameraRay *camera.Ray, cam *camera.Camera, text *v41.Text) {
	x, y, d, intersects := WorldToClip(pv1, cameraRay, &cam.Up, float32(cam.AspectRatio), cam.Near)
	if intersects {
		wx, hy := ClipToScreen(x, y, float32(cam.WindowWidth), float32(cam.WindowHeight))
		// log.Printf("Draw worked...(%f, %f) and distance=%f\n", wx, hy, d)
		text.SetScale(DistanceToFontScale(d, fonts.ScaleMin, 0.8))
		text.SetPosition(mgl.Vec2{wx, hy}) // x=0, y=300
		text.Draw()
	}
}

func mergeStrings(left []string, right []string) []string {
	i := 0
	j := 0
	n := len(left)
	m := len(right)
	res := []string{}
	for i < n && j < m {
		if strings.Compare(left[i], right[j]) < 0 {
			res = append(res, left[i])
			i++
		} else {
			res = append(res, right[j])
			j++
		}
	}
	for i < n {
		res = append(res, left[i])
		i++
	}
	for j < m {
		res = append(res, right[j])
		j++
	}
	return res
}

// TODO: consolidate with type parameters in cluster_map.go
func merge(left []int, right []int) []int {
	i := 0
	j := 0
	n := len(left)
	m := len(right)
	res := []int{}
	for i < n && j < m {
		if left[i] < right[j] {
			res = append(res, left[i])
			i++
		} else {
			res = append(res, right[j])
			j++
		}
	}
	for i < n {
		res = append(res, left[i])
		i++
	}
	for j < m {
		res = append(res, right[j])
		j++
	}
	return res
}

func MergeSortString(arr []string) []string {
	n := len(arr)
	if n <= 1 {
		return arr
	}
	mid := n / 2
	left := MergeSortString(arr[:mid])
	right := MergeSortString(arr[mid:])
	return mergeStrings(left, right)
}

// TODO: consolidate with type parameters in cluster_map.go
func MergeSort(arr []int) []int {
	n := len(arr)
	if n <= 1 {
		return arr
	}
	mid := n / 2
	left := MergeSort(arr[:mid])
	right := MergeSort(arr[mid:])
	return merge(left, right)
}

func parseList(ks []interface{}, tab int) []interface{} {
	for i := range ks {
		fmt.Printf("%s", strings.Repeat("*", tab))
		fmt.Printf("- [%d]: \n", i)
		v := ks[i]
		kind := reflect.ValueOf(v).Kind()
		if kind == reflect.Map {
			if vMap, ok := v.(map[string]interface{}); ok {
				parseMap(vMap, tab+4)
			}
		} else if kind == reflect.Array {
			if vList, ok := v.([]interface{}); ok {
				parseList(vList, tab+4)
			}
		} else {
			fmt.Printf("%s", strings.Repeat(" ", tab))
			fmt.Printf(" value: %s\n", parseTypeToString(v, kind))
		}
	}
	return nil
}

func parseTypeToString(v interface{}, kind reflect.Kind) string {
	if kind == reflect.Bool {
		if v.(bool) {
			return "true"
		} else {
			return "false"
		}
	} else if kind == reflect.String {
		return v.(string)
	} else if kind == reflect.Int {
		return fmt.Sprintf("%d", v.(int))
	} else if kind == reflect.Int64 {
		return fmt.Sprintf("%d", v.(int64))
	} else if kind == reflect.Invalid || v == nil {
		return "{}"
	}
	return "<nil>"
}

func parseMap(ks map[string]interface{}, tab int) map[string]interface{} {
	for k, v := range ks {
		fmt.Printf("%s", strings.Repeat("*", tab))
		fmt.Printf(" key: %s\n", k)
		kind := reflect.ValueOf(v).Kind()
		if kind == reflect.Map {
			if vMap, ok := v.(map[string]interface{}); ok {
				parseMap(vMap, tab+4)
			}
		} else if kind == reflect.Array || kind == reflect.Slice {
			if vList, ok := v.([]interface{}); ok {
				parseList(vList, tab+4)
			}
		} else {
			fmt.Printf("%s", strings.Repeat(" ", tab))
			fmt.Printf(" value: %s\n", parseTypeToString(v, kind))
		}
	}
	return nil
}

type OrderedMap struct {
	sums   []string
	keys   []string
	values []interface{}
	isList []bool
}

func copyOrderedMapInterfaceRecursive(v interface{}) (interface{}, string) {
	if strings.Contains(fmt.Sprintf("%s", reflect.TypeOf(v)), "OrderedMap") {
		return DuplicateOrderedMap(v.(*OrderedMap))
	} else {
		kind := reflect.ValueOf(v).Kind()
		if kind == reflect.Array || kind == reflect.Slice {
			if vList, ok := v.([]interface{}); ok {
				targetList := make([]interface{}, len(vList))
				targetSum := make([]string, len(vList))
				for i, v := range vList {
					elem, elemSum := copyOrderedMapInterfaceRecursive(v)
					targetList[i] = elem
					targetSum[i] = elemSum
				}
				return targetList, getTotalSumFromList(targetSum)
			}
		} else {
			data := parseTypeToString(v, kind)
			return data, getHashFromData(data)
		}
	}
	return nil, ""
}

func DuplicateOrderedMap(om *OrderedMap) (*OrderedMap, string) {
	if om == nil {
		return nil, ""
	}
	o := len(om.sums)
	n := len(om.keys)
	m := len(om.values)
	l := len(om.isList)
	// assert that array lengths are same
	if o != n || n != m || m != l {
		fmt.Println("ERROR: DuplicateOrderedMap could not be run. Array lengths are different!")
		return nil, ""
	}

	om2Sums := make([]string, o)
	om2Keys := make([]string, n)
	om2IsList := make([]bool, l)
	om2Values := make([]interface{}, m)
	for i := range n {
		om2Keys[i] = om.keys[i]
		om2IsList[i] = om.isList[i]
		val, sum := copyOrderedMapInterfaceRecursive(om.values[i])
		om2Values[i] = val
		om2Sums[i] = sum
	}
	return &OrderedMap{sums: om2Sums, keys: om2Keys, values: om2Values, isList: om2IsList}, getTotalSumFromKeys(om2Sums)
}

func createOrderedKeyList(kubeState map[string]interface{}) []string {
	orderedKeyList := []string{}
	for k := range kubeState {
		orderedKeyList = append(orderedKeyList, k)
	}
	return MergeSortString(orderedKeyList)
}

func getIndexOf(arr []string, elem string) int {
	for i, e := range arr {
		if e == elem {
			return i
		}
	}
	return -1
}

func createOrderedMapRecursive(v interface{}) (interface{}, string) {
	kind := reflect.ValueOf(v).Kind()
	if kind == reflect.Map {
		if vMap, ok := v.(map[string]interface{}); ok {
			return CreateOrderedMap(vMap) // iterative step - parent
		}
	} else if kind == reflect.Array || kind == reflect.Slice {
		if vList, ok := v.([]interface{}); ok {
			vListTransformed := []any{}
			vListSums := []string{}
			for _, vListElem := range vList {
				listElement, sum := createOrderedMapRecursive(vListElem)
				vListTransformed = append(vListTransformed, listElement) // iterative step - self
				vListSums = append(vListSums, sum)
			}
			return vListTransformed, getTotalSumFromList(vListSums)
		}
	}
	data := parseTypeToString(v, kind)
	return data, getHashFromData(data) // base case
}

func getHashFromData(data string) string {
	return fmt.Sprintf("%x", sha256.Sum256([]byte(data)))
}

func getTotalSumFromList(arr []string) string {
	return strings.Join(arr, ",")
}

func getTotalSumFromKeys(arr []string) string {
	return strings.Join(arr, "|")
}

func CreateOrderedMap(kubeState map[string]interface{}) (*OrderedMap, string) {
	// get an array of ordered keys
	keyList := createOrderedKeyList(kubeState)
	n := len(keyList)
	// initialize list of values to be zipped based on ordered keys
	valueList := make([]interface{}, n)
	sumList := make([]string, n)
	isListList := make([]bool, n)
	for k, v := range kubeState {
		// get the index in the keyList array
		keyIndex := getIndexOf(keyList, k)
		if keyIndex == -1 {
			log.Println("ERROR: found key in map[string]interface{} that is not mapped to OrderedMap")
			continue
		}
		kind := reflect.ValueOf(v).Kind()
		isListList[keyIndex] = kind == reflect.Array || kind == reflect.Slice
		value, sum := createOrderedMapRecursive(v)
		valueList[keyIndex] = value
		sumList[keyIndex] = sum
	}
	orderedMap := &OrderedMap{sumList, keyList, valueList, isListList}
	return orderedMap, getTotalSumFromKeys(sumList)
}

func printOrderedMapRecursive(v interface{}, tab int) {
	if strings.Contains(fmt.Sprintf("%s", reflect.TypeOf(v)), "OrderedMap") {
		v.(*OrderedMap).Print(tab + 4)
	} else {
		kind := reflect.ValueOf(v).Kind()
		if kind == reflect.Array || kind == reflect.Slice {
			if vList, ok := v.([]interface{}); ok {
				for _, v := range vList {
					printOrderedMapRecursive(v, tab+4)
				}
			}
		} else {
			fmt.Printf("%s", strings.Repeat(" ", tab))
			fmt.Printf(" ivalue: %s\n", parseTypeToString(v, kind))
		}
	}
}

func (om *OrderedMap) Print(tab int) {
	n := len(om.keys)
	for i := 0; i < n; i++ {
		k := om.keys[i]
		v := om.values[i]
		isList := om.isList[i]

		fmt.Printf("%s", strings.Repeat("*", tab))
		if isList {
			fmt.Printf("-ikey: %s\n", k)
		} else {
			fmt.Printf("+ikey: %s\n", k)
		}
		printOrderedMapRecursive(v, tab)
	}
}

func ParseKubeState(ks map[string]interface{}) *OrderedMap {
	fmt.Println("Kube state:")
	// parseMap(ks, 4)
	om, _ := CreateOrderedMap(ks)
	om.Print(4)

	return om
}

func checkEqualsOrderedMapRecursive(v1, v2 interface{}) bool {
	v1Type := strings.Contains(fmt.Sprintf("%s", reflect.TypeOf(v1)), "OrderedMap")
	v2Type := strings.Contains(fmt.Sprintf("%s", reflect.TypeOf(v2)), "OrderedMap")
	if v1Type != v2Type {
		return false
	}
	if v1Type {
		return v1.(*OrderedMap).Equals(v2.(*OrderedMap))
	}

	v1Kind := reflect.ValueOf(v1).Kind()
	v2Kind := reflect.ValueOf(v2).Kind()
	v1KindCheck := v1Kind == reflect.Array || v1Kind == reflect.Slice
	v2KindCheck := v2Kind == reflect.Array || v2Kind == reflect.Slice
	if v1KindCheck != v2KindCheck {
		return false
	}
	if v1KindCheck {
		v1List, ok1 := v1.([]interface{})
		v2List, ok2 := v2.([]interface{})
		if !ok1 || !ok2 {
			return false
		}
		ln := len(v1List)
		lm := len(v2List)
		if ln != lm {
			return false
		}
		for i := range ln {
			if !checkEqualsOrderedMapRecursive(v1List[i], v2List[i]) {
				return false
			}
		}
	} else if parseTypeToString(v1, v1Kind) != parseTypeToString(v2, v2Kind) {
		return false
	}
	return true
}

func (om *OrderedMap) Equals(om2 *OrderedMap) bool {
	if om2 == nil {
		return false
	}

	n := len(om.keys)
	m := len(om2.keys)
	if n != m {
		return false
	}

	// key and isList check
	for i := range n {
		if om.keys[i] != om2.keys[i] {
			return false
		}
		if om.isList[i] != om2.isList[i] {
			return false
		}
	}

	for i := range n {
		if !checkEqualsOrderedMapRecursive(om.values[i], om2.values[i]) {
			return false
		}
	}
	return true
}
