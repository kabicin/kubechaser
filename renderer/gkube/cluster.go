package gkube

import (
	"fmt"
	"log"
	"math"
	"math/rand/v2"
	"reflect"
	"slices"
	"sync"

	v41 "github.com/4ydx/gltext/v4.1"
	mgl "github.com/go-gl/mathgl/mgl32"
	"github.com/kabicin/kubechaser/renderer/camera"
	"github.com/kabicin/kubechaser/renderer/controller"
	"github.com/kabicin/kubechaser/renderer/logg"
	"github.com/kabicin/kubechaser/renderer/scene"
	"github.com/kabicin/kubechaser/renderer/shader"
	"github.com/kabicin/kubechaser/renderer/utils"
)

type State int

const (
	Loading   State = iota
	Running   State = iota
	Succeeded State = iota
	Failed    State = iota
)

type GEventStatus int

const (
	GCREATE   GEventStatus = iota
	GMODIFIED GEventStatus = iota
	GDELETE   GEventStatus = iota
)

type GResource int

const (
	GWIRE                  GResource = iota
	GDEPLOYMENT            GResource = iota
	GSTATEFULSET           GResource = iota
	GREPLICASET            GResource = iota
	GPOD                   GResource = iota
	GSERVICE               GResource = iota
	GINGRESS               GResource = iota
	GSERVICEACCOUNT        GResource = iota
	GROLE                  GResource = iota
	GROLEBINDING           GResource = iota
	GCLUSTERROLE           GResource = iota
	GCLUSTERROLEBINDING    GResource = iota
	GJOB                   GResource = iota
	GCRONJOB               GResource = iota
	GDAEMONSET             GResource = iota
	GSECRET                GResource = iota
	GCONFIGMAP             GResource = iota
	GPERSISTENTVOLUME      GResource = iota
	GPERSISTENTVOLUMECLAIM GResource = iota
	GCLUSTEROBJECTFRAME    GResource = iota
	GNAMESPACEOBJECTFRAME  GResource = iota
)

type GDirection int

const (
	GNONE   GDirection = iota
	GTOP    GDirection = iota
	GRIGHT  GDirection = iota
	GBOTTOM GDirection = iota
	GLEFT   GDirection = iota
	GUP     GDirection = iota
	GDOWN   GDirection = iota
)

type GSettings int

const (
	GSETTING_NONE             GSettings = 0
	GSETTING_GWIRE_NORTH      GSettings = 0b0000000001
	GSETTING_GWIRE_EAST       GSettings = 0b0000000010
	GSETTING_GWIRE_CENTER     GSettings = 0b0000000100
	GSETTING_GWIRE_SOUTH      GSettings = 0b0000001000
	GSETTING_GWIRE_WEST       GSettings = 0b0000010000
	GSETTING_GWIRE_VERT       GSettings = 0b0000100000
	GSETTING_GWIRE_VERT_NORTH GSettings = 0b0001000000
	GSETTING_GWIRE_VERT_EAST  GSettings = 0b0010000000
	GSETTING_GWIRE_VERT_SOUTH GSettings = 0b0100000000
	GSETTING_GWIRE_VERT_WEST  GSettings = 0b1000000000
)

var AllGSettings = []GSettings{
	GSETTING_NONE,
	GSETTING_GWIRE_NORTH,
	GSETTING_GWIRE_EAST,
	GSETTING_GWIRE_CENTER,
	GSETTING_GWIRE_SOUTH,
	GSETTING_GWIRE_WEST,
	GSETTING_GWIRE_VERT,
	GSETTING_GWIRE_VERT_NORTH,
	GSETTING_GWIRE_VERT_EAST,
	GSETTING_GWIRE_VERT_SOUTH,
	GSETTING_GWIRE_VERT_WEST,
}

type GStatus interface{}

type GObjectEvent struct {
	eventType          GEventStatus
	resource           GResource
	name               string
	namespace          string
	direction          GDirection
	settings           GSettings
	overrideLastOffset *mgl.Vec3
	status             GStatus
	slot               int
	kubeState          map[string]interface{}
}

func (ge *GObjectEvent) GetKubeState() map[string]interface{} {
	return ge.kubeState
}

func (ge *GObjectEvent) GetSlot() int {
	return ge.slot
}

func (ge *GObjectEvent) GetStatus() GStatus {
	return ge.status
}

func (ge *GObjectEvent) GetType() GEventStatus {
	return ge.eventType
}

func (ge *GObjectEvent) GetResource() GResource {
	return ge.resource
}

func (ge *GObjectEvent) GetName() string {
	return ge.name
}

func (ge *GObjectEvent) GetNamespace() string {
	return ge.namespace
}

func (ge *GObjectEvent) GetDirection() GDirection {
	return ge.direction
}

func (ge *GObjectEvent) GetSettings() GSettings {
	return ge.settings
}

func (ge *GObjectEvent) GetOverrideLastOffset() *mgl.Vec3 {
	return ge.overrideLastOffset
}

type GObject interface {
	Create(*GCluster, string, string, *mgl.Vec3, *v41.Font, uint32, GSettings, bool) *scene.SceneObject
	OnClick()
	GetObject() *scene.SceneObject
	GetIdentifier() (string, string) // name and namespace
	GetResource() GResource          // i.e. GPOD, GDEPLOYMENT
	GetCurrentOffset() *mgl.Vec3
	SetDeleting()
	Delete()
}

type GObjectFrame interface {
	GObject
	SetObjectFrame(center, bounds mgl.Vec3, onPostInitCallback func())
	UpdateObjectFrame(center, bounds mgl.Vec3, onPostInitCallback func())
}

type GCluster struct {
	mainScene              *scene.Scene
	gobjects               []GObject
	gobjectFrames          []GObjectFrame
	gobjectEventQueue      []GObjectEvent
	gobjectEventQueueMutex *sync.Mutex
	gobjectMutex           *sync.Mutex

	font *v41.Font
	// shaders map[GResource]*shader.Program
	shaders *sync.Map

	currentObject    GObject
	currentName      string
	currentNamespace string

	lastOffsets map[string]mgl.Vec3

	slots          map[string][][]SlotResource
	gcSlots        []GObject
	gcSlotsMutex   *sync.Mutex
	namespaceSlots []string
}

func getGResourceName(resource GResource) string {
	if resource == GWIRE {
		return "GWIRE"
	}
	if resource == GDEPLOYMENT {
		return "GDEPLOYMENT"
	}
	if resource == GSTATEFULSET {
		return "GSTATEFULSET"
	}
	if resource == GREPLICASET {
		return "GREPLICASET"
	}
	if resource == GPOD {
		return "GPOD"
	}
	if resource == GPOD {
		return "GSERVICE"
	}
	if resource == GINGRESS {
		return "GINGRESS"
	}
	if resource == GSERVICEACCOUNT {
		return "GSERVICEACCOUNT"
	}
	if resource == GROLE {
		return "GROLE"
	}
	if resource == GROLEBINDING {
		return "GROLEBINDING"
	}
	if resource == GCLUSTERROLE {
		return "GCLUSTERROLE"
	}
	if resource == GCLUSTERROLEBINDING {
		return "GCLUSTERROLEBINDING"
	}
	if resource == GJOB {
		return "GJOB"
	}
	if resource == GCRONJOB {
		return "GCRONJOB"
	}
	if resource == GDAEMONSET {
		return "GDAEMONSET"
	}
	if resource == GSECRET {
		return "GSECRET"
	}
	if resource == GCONFIGMAP {
		return "GCONFIGMAP"
	}
	if resource == GPERSISTENTVOLUME {
		return "GPERSISTENTVOLUME"
	}
	if resource == GPERSISTENTVOLUMECLAIM {
		return "GPERSISTENTVOLUMECLAIM"
	}
	// object frames
	if resource == GCLUSTEROBJECTFRAME {
		return "GCLUSTEROBJECTFRAME"
	}
	if resource == GNAMESPACEOBJECTFRAME {
		return "GNAMESPACEOBJECTFRAME"
	}
	return "N/A"
}

var OBJECT_FRAMES = []GResource{
	GCLUSTEROBJECTFRAME,
	GNAMESPACEOBJECTFRAME,
}

func isGResourceObjectFrame(gob GObject) bool {
	return slices.Contains(OBJECT_FRAMES, gob.GetResource())
}

func (gc *GCluster) GetMainScene() *scene.Scene {
	return gc.mainScene
}

func (gc *GCluster) SetSelected(gobj GObject) {
	gc.currentObject = gobj
	name, namespace := gobj.GetIdentifier()
	gc.currentName = name
	gc.currentNamespace = namespace
	offset := gc.currentObject.GetCurrentOffset()
	log.Printf("Set current object to name: %s in namespace: %s vec3(%f,%f,%f)\n", name, namespace, offset.X(), offset.Y(), offset.Z())
}

func (gc *GCluster) LockEventQueue() {
	gc.gobjectEventQueueMutex.Lock()
}

func (gc *GCluster) UnlockEventQueue() {
	gc.gobjectEventQueueMutex.Unlock()
}

// pre-condition: already has lock on gobjects
func (gc *GCluster) getGObjectFromSlot(sr SlotResource) GObject {
	currSig := sr.GetSignature()
	for _, gob := range gc.gobjects {
		name, namespace := gob.GetIdentifier()
		resource := gob.GetResource()
		sr2 := SlotResource{name: name, namespace: namespace, resource: resource}
		if sr2.GetSignature() == currSig {
			return gob
		}
	}
	return nil
}

func (gc *GCluster) getSlotContainingGObject(gob GObject) SlotResource {
	name, namespace := gob.GetIdentifier()
	resource := gob.GetResource()
	return SlotResource{name: name, namespace: namespace, resource: resource}
}

// check for garbage collection
func (gc *GCluster) GC() {
	gc.gcSlotsMutex.Lock()
	defer gc.gcSlotsMutex.Unlock()
	n := len(gc.gcSlots)
	if n == 0 {
		return
	}

	deletedSlots := []int{} // array to hold indices of slots that were evicted
	for i, gob := range gc.gcSlots {
		if gob.GetObject().IsDeleteReady {
			gc.EvictSlot(gc.getSlotContainingGObject(gob)) // evict
			deletedSlots = append(deletedSlots, i)         // signal to remove from garbage collector
		}
	}
	fmt.Println("delted slot")
	fmt.Println(deletedSlots)
	// actualize the remove from garbage collector
	deletedSlots = utils.MergeSort(deletedSlots)
	fmt.Println(deletedSlots)
	for i := len(deletedSlots) - 1; i >= 0; i-- {
		delIndex := deletedSlots[i]
		if delIndex == 0 {
			gc.gcSlots = gc.gcSlots[1:]
		} else if delIndex == len(deletedSlots)-1 {
			gc.gcSlots = gc.gcSlots[:len(deletedSlots)-1]
		} else {
			gc.gcSlots = append(gc.gcSlots[delIndex:], gc.gcSlots[:delIndex+1]...)
		}
	}
}

func (gc *GCluster) DeleteGObject(gob GObject) {
	deleteIndex := -1
	for i, ogob := range gc.gobjects {
		if ogob == gob {
			deleteIndex = i
		}
	}
	if deleteIndex != -1 {
		gc.gobjects = append(gc.gobjects[:deleteIndex], gc.gobjects[deleteIndex+1:]...)
	}
}

func (gc *GCluster) PopGObjectEvent() (GObjectEvent, bool) {
	gc.gobjectEventQueueMutex.Lock()
	defer gc.gobjectEventQueueMutex.Unlock()
	if len(gc.gobjectEventQueue) > 0 {
		obj := gc.gobjectEventQueue[0]
		gc.gobjectEventQueue = gc.gobjectEventQueue[1:]
		return obj, true
	}
	return GObjectEvent{}, false
}

func (gc *GCluster) PushGObjectEvent(eventType GEventStatus, resource GResource, name, namespace string, direction GDirection, settings GSettings, overrideLastOffset *mgl.Vec3, status GStatus, slot int, kubeState map[string]interface{}) {
	gc.LockEventQueue()
	defer gc.UnlockEventQueue()
	gc.gobjectEventQueue = append(gc.gobjectEventQueue, GObjectEvent{
		eventType:          eventType,
		resource:           resource,
		name:               name,
		namespace:          namespace,
		direction:          direction,
		settings:           settings,
		overrideLastOffset: overrideLastOffset,
		status:             status,
		slot:               slot,
		kubeState:          kubeState,
	})
}

func (gc *GCluster) Create(ctrl *controller.Controller, cam *camera.Camera, font *v41.Font, shaderPrograms []*shader.Program) {
	gc.mainScene = &scene.Scene{}
	gc.mainScene.Init(shaderPrograms, []*scene.SceneObject{}, cam)

	gc.gobjectEventQueue = make([]GObjectEvent, 0)
	gc.gobjectEventQueueMutex = &sync.Mutex{}

	gc.gobjects = make([]GObject, 0)
	gc.gobjectFrames = make([]GObjectFrame, 0)
	gc.gobjectMutex = &sync.Mutex{}
	gc.font = font

	gc.slots = make(map[string][][]SlotResource)
	gc.gcSlots = make([]GObject, 0)
	gc.gcSlotsMutex = &sync.Mutex{}
	gc.namespaceSlots = []string{}

	// create shader mapping
	gc.shaders = &sync.Map{}

	defaultShaderProgram := shaderPrograms[1]
	gc.shaders.Store(GDEPLOYMENT, defaultShaderProgram)
	gc.shaders.Store(GSTATEFULSET, defaultShaderProgram)
	gc.shaders.Store(GREPLICASET, defaultShaderProgram)
	gc.shaders.Store(GWIRE, defaultShaderProgram)
	gc.shaders.Store(GPOD, defaultShaderProgram)
	gc.shaders.Store(GSERVICE, defaultShaderProgram)
	gc.shaders.Store(GINGRESS, defaultShaderProgram)
	gc.shaders.Store(GSERVICEACCOUNT, defaultShaderProgram)
	gc.shaders.Store(GROLE, defaultShaderProgram)
	gc.shaders.Store(GROLEBINDING, defaultShaderProgram)
	gc.shaders.Store(GCLUSTERROLE, defaultShaderProgram)
	gc.shaders.Store(GCLUSTERROLEBINDING, defaultShaderProgram)
	gc.shaders.Store(GJOB, defaultShaderProgram)
	gc.shaders.Store(GCRONJOB, defaultShaderProgram)
	gc.shaders.Store(GDAEMONSET, defaultShaderProgram)
	gc.shaders.Store(GSECRET, defaultShaderProgram)
	gc.shaders.Store(GCONFIGMAP, defaultShaderProgram)
	gc.shaders.Store(GPERSISTENTVOLUME, defaultShaderProgram)
	gc.shaders.Store(GPERSISTENTVOLUMECLAIM, defaultShaderProgram)
	gc.shaders.Store(GCLUSTEROBJECTFRAME, defaultShaderProgram)
	gc.shaders.Store(GNAMESPACEOBJECTFRAME, defaultShaderProgram)

	ctrl.AddClickHandler(gc.mainScene.Click)
}

func (gc *GCluster) RemoveGObject(event GObjectEvent) {
	log.Println("Deleting GObject...")
	gc.gobjectMutex.Lock()
	defer gc.gobjectMutex.Unlock()

	resource := event.GetResource()
	name := event.GetName()
	namespace := event.GetNamespace()

	if resource == GDEPLOYMENT || resource == GREPLICASET || resource == GPOD {
		gc.gcSlotsMutex.Lock()
		sr := SlotResource{name: name, namespace: namespace, resource: resource}
		gc.SetDeletingSlot(sr) // signal deleting for this slot
		if gob := gc.getGObjectFromSlot(sr); gob != nil {
			gc.gcSlots = append(gc.gcSlots, gob) // signal garbage collector to listen for when the slot has been deleted
			fmt.Println("tracked gobject for deletion")
		}
		gc.gcSlotsMutex.Unlock()
	}
}

func (gc *GCluster) UpdateGObject(event GObjectEvent) {
	// TODO: impl
	log.Println("Updating GObject...")
}

// randomize this point p into space (i.e. far away from Origin)
const (
	XDIST = 1000.0
	YDIST = 200
	ZDIST = 1000.0
)

func randomizePointInSpace() *mgl.Vec3 {
	return &mgl.Vec3{rand.Float32()*XDIST + -XDIST/2, rand.Float32()*YDIST + -YDIST/2, rand.Float32()*ZDIST + -ZDIST/2}
}

func KubeStateToString(kubeState map[string]interface{}) string {
	out := ""
	for k, v := range kubeState {
		out += fmt.Sprintf("%s\n", k)
		if reflect.ValueOf(v).Kind() == reflect.Map {
			out += "is map\n"
		} else {
			out += "not map\n"
		}
	}

	return out
}

func (gc *GCluster) UpdateGObjectFrames(debug bool) {
	gc.gobjectMutex.Lock()
	defer gc.gobjectMutex.Unlock()

	for _, gobjectFrame := range gc.gobjectFrames {
		gobjectFrameNamespace, _ := gobjectFrame.GetIdentifier() // identifier for ObjectFrame uses name attrib as the namespace
		if gobjectFrame.GetResource() == GNAMESPACEOBJECTFRAME {
			hasPoints, center, bounds := gc.getBounds(mgl.Vec3{5, 3, 5}, func(obj GObject) bool {
				_, ns := obj.GetIdentifier()
				return ns != gobjectFrameNamespace // skip filter: don't consider GObjects where namespace is not the same
			})
			if hasPoints {
				if debug {
					fmt.Printf("gobjectframe has points: \n")
					logg.PrintVec3(center)
					logg.PrintVec3(bounds)
				}
				gobjectFrame.UpdateObjectFrame(center, bounds, func() {
					gc.GetMainScene().Update() // refresh shader after unsync between gd.Create and gd.SetFrame change
					if debug {
						fmt.Println("gobjectframe: shaders updated")
					}
				})
			}
		}
	}
}

func (gc *GCluster) AddGObject(event GObjectEvent) {
	gc.gobjectMutex.Lock()
	defer gc.gobjectMutex.Unlock()

	resource := event.GetResource()
	name := event.GetName()
	namespace := event.GetNamespace()
	// direction := event.GetDirection()
	settings := event.GetSettings()
	// overrideLastOffset := event.GetOverrideLastOffset()
	status := event.GetStatus()
	kubeState := event.GetKubeState()

	// slot := event.GetSlot()

	// log.Printf("Creating GOBJECT - %s - %s \n", getGResourceName(resource), name)
	rawShader, found := gc.shaders.Load(resource)
	shader := rawShader.(*shader.Program)
	if !found {
		log.Printf("GObject could not be created because resource %s was not bound to any shaders\n", getGResourceName(resource))
		return
	}
	if shader == nil {
		log.Printf("GObject could not be created because resource %s was not bound to a valid shader\n", getGResourceName(resource))
		return
	}

	randomDisplacement := randomizePointInSpace()
	if resource == GWIRE {
		gw := &GWire{}
		wireStatus := status.(*GWireStatus)
		if wireStatus.Up {
			gw.state = Running
		} else {
			gw.state = Loading
		}
		gw.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gw)
	}
	if resource == GDEPLOYMENT {
		gd := &GDeployment{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
		gc.CreateAndReserveSlot(name, namespace, gd, resource, []GSignatureConnection{})
	}
	if resource == GSTATEFULSET {
		gd := &GStatefulSet{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GREPLICASET {
		grs := &GReplicaSet{}
		grs.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, grs)
		replicaSetStatus := status.(*GReplicaSetStatus)
		sigConns := []GSignatureConnection{}
		if len(replicaSetStatus.OwnerReferenceName) > 0 {
			sigConns = append(sigConns, GSignatureConnection{resource: GDEPLOYMENT, name: replicaSetStatus.OwnerReferenceName, namespace: namespace})
		}
		gc.CreateAndReserveSlot(name, namespace, grs, resource, sigConns)
	}
	if resource == GPOD {
		gp := &GPod{}
		podStatus := status.(*GPodStatus)
		if podStatus.Up {
			gp.state = Running
		} else {
			gp.state = Loading
		}

		fmt.Println("Pod is creating with kube state:")
		fmt.Println(KubeStateToString(kubeState))

		gp.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gp.object.Spinning = true
		// orderedMap := utils.ParseKubeState(event.GetKubeState())
		// orderedMap2 := utils.ParseKubeState(event.GetKubeState())
		// orderedMap3 := utils.DuplicateOrderedMap(orderedMap2)
		// if orderedMap.Equals(orderedMap3) {
		// 	fmt.Println("ORDERED MAP EQUALS!")
		// } else {
		// 	fmt.Println("ORDERED MAP DOES NOT EQUAL!")
		// }
		gp.SetKubeState(event.GetKubeState())
		gc.gobjects = append(gc.gobjects, gp)

		sigConns := []GSignatureConnection{}
		if len(podStatus.OwnerReferenceName) > 0 {
			// fmt.Printf("Adding owner ref! %s\n", podStatus.OwnerReferenceName)
			sigConns = append(sigConns, GSignatureConnection{resource: GREPLICASET, name: podStatus.OwnerReferenceName, namespace: namespace})
		}
		gc.CreateAndReserveSlot(name, namespace, gp, resource, sigConns)
	}
	if resource == GSERVICE {
		gp := &GService{}
		gp.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gp)
	}
	if resource == GINGRESS {
		gi := &GIngress{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GSERVICEACCOUNT {
		gi := &GServiceAccount{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GROLE {
		gi := &GRole{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GROLEBINDING {
		gi := &GRoleBinding{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GCLUSTERROLE {
		gi := &GClusterRole{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GCLUSTERROLEBINDING {
		gi := &GClusterRoleBinding{}
		gi.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gi)
	}
	if resource == GJOB {
		gd := &GJob{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GCRONJOB {
		gd := &GCronJob{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GDAEMONSET {
		gd := &GDaemonSet{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GSECRET {
		gd := &GSecret{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GCONFIGMAP {
		gd := &GConfigMap{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GPERSISTENTVOLUME {
		gd := &GPersistentVolume{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GPERSISTENTVOLUMECLAIM {
		gd := &GPersistentVolumeClaim{}
		gd.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		gc.gobjects = append(gc.gobjects, gd)
	}
	if resource == GCLUSTEROBJECTFRAME {
		gof := &GClusterObjectFrame{}
		gof.Create(gc, name, namespace, randomDisplacement, gc.font, shader.ID, settings, true)
		hasPoints, center, bounds := gc.getBounds(mgl.Vec3{5, 3, 5}, func(obj GObject) bool {
			return false // skip filter: return false to skip no objects (gets all GObjects)
		})
		if hasPoints {
			gof.SetObjectFrame(center, bounds, func() {
				gc.GetMainScene().Update() // refresh shader after unsync between gd.Create and gd.SetFrame change
			})
		}
		gc.gobjects = append(gc.gobjects, gof)
		gc.gobjectFrames = append(gc.gobjectFrames, gof) // a handle to the object frame is stored for polling updates
	}
	if resource == GNAMESPACEOBJECTFRAME {
		gof := &GNamespaceObjectFrame{}
		gof.Create(gc, name, namespace, &mgl.Vec3{0, 0, 0}, gc.font, shader.ID, settings, false)
		hasPoints, center, bounds := gc.getBounds(mgl.Vec3{5, 3, 5}, func(obj GObject) bool {
			_, ns := obj.GetIdentifier()
			return ns != namespace // skip filter: don't consider GObjects where namespace is not the same
		})
		if hasPoints {
			gof.SetObjectFrame(center, bounds, func() {
				gc.GetMainScene().Update() // refresh shader after unsync between gd.Create and gd.SetFrame change
			})
		}
		gc.gobjects = append(gc.gobjects, gof)
		gc.gobjectFrames = append(gc.gobjectFrames, gof) // a handle to the object frame is stored for polling updates
	}
}

func (cluster *GCluster) expandBound(bound, target *mgl.Vec3, dir mgl.Vec3) {
	for i := range 3 {
		if dir[i] > 0 {
			if target[i] > bound[i] {
				bound[i] = target[i]
			}
		}
		if dir[i] < 0 {
			if target[i] < bound[i] {
				bound[i] = target[i]
			}
		}
	}
}

// from 6 points of a cube, expand out all the 6 points to form a bounding box around all objects from cluster.gobjects then return the center and bounds
func (cluster *GCluster) getBounds(padding mgl.Vec3, skipFilter func(GObject) bool) (bool, mgl.Vec3, mgl.Vec3) {
	count := 0
	minFloat := float32(-math.MaxFloat32)
	maxFloat := float32(math.MaxFloat32)
	frontTopLeft := mgl.Vec3{maxFloat, minFloat, minFloat}     // push to (-x, y, z)
	frontTopRight := mgl.Vec3{minFloat, minFloat, minFloat}    // push to (x, y, z)
	frontBottomLeft := mgl.Vec3{maxFloat, maxFloat, minFloat}  // push to (-x, -y, z)
	frontBottomRight := mgl.Vec3{minFloat, maxFloat, minFloat} // push to (x, -y, z)
	backTopLeft := mgl.Vec3{maxFloat, minFloat, maxFloat}      // push to (-x, y, -z)
	backTopRight := mgl.Vec3{minFloat, minFloat, maxFloat}     // push to (x, y, -z)
	backBottomLeft := mgl.Vec3{maxFloat, maxFloat, maxFloat}   // push to (-x, -y, -z)
	backBottomRight := mgl.Vec3{minFloat, maxFloat, maxFloat}  // push to (x, -y, -z)
	maxScale := mgl.Vec3{0, 0, 0}
	for _, obj := range cluster.gobjects {
		if skipFilter(obj) {
			continue
		}
		if obj.GetObject() != nil && obj.GetObject().Transform != nil && obj.GetObject().Transform.PositionAnimator.X_final != nil {
			if obj.GetObject().Transform.Scale != nil {
				s := *obj.GetObject().Transform.Scale
				for i := range 3 {
					if s[i] > maxScale[i] {
						maxScale[i] = s[i]
					}
				}
			}
			t := obj.GetObject().Transform.PositionAnimator.X_final
			cluster.expandBound(&frontTopLeft, t, mgl.Vec3{-1, 1, 1})
			cluster.expandBound(&frontTopRight, t, mgl.Vec3{1, 1, 1})
			cluster.expandBound(&frontBottomLeft, t, mgl.Vec3{-1, -1, 1})
			cluster.expandBound(&frontBottomRight, t, mgl.Vec3{1, -1, 1})
			cluster.expandBound(&backTopLeft, t, mgl.Vec3{-1, 1, -1})
			cluster.expandBound(&backTopRight, t, mgl.Vec3{1, 1, -1})
			cluster.expandBound(&backBottomLeft, t, mgl.Vec3{-1, -1, -1})
			cluster.expandBound(&backBottomRight, t, mgl.Vec3{1, -1, -1})
			count += 1
		}
	}
	center := mgl.Vec3{0, 0, 0}
	bounds := mgl.Vec3{1, 1, 1}
	hasPoints := count > 0
	if hasPoints {
		// construct bounding box from two points
		tlx := min(frontTopLeft.X(), frontBottomLeft.X(), backTopLeft.X(), backBottomLeft.X())
		tly := max(frontTopLeft.Y(), frontTopRight.Y(), backTopLeft.Y(), backTopRight.Y())
		tlz := max(frontTopLeft.Z(), frontTopRight.Z(), frontBottomLeft.Z(), frontBottomRight.Z())
		brx := max(frontTopRight.X(), frontBottomRight.X(), backBottomRight.X(), backTopRight.X())
		bry := min(frontBottomRight.Y(), frontBottomLeft.Y(), backBottomLeft.Y(), backBottomRight.Y())
		brz := min(backTopLeft.Z(), backTopRight.Z(), backBottomLeft.Z(), backBottomRight.Z())
		// 2 points identifying bounding box extremes
		topLeft := mgl.Vec3{tlx, tly, tlz}
		backRight := mgl.Vec3{brx, bry, brz}

		// get bounding box size
		xdiff := backRight.X() - topLeft.X()
		ydiff := topLeft.Y() - backRight.Y()
		zdiff := topLeft.Z() - backRight.Z()

		// return center and bounds
		center = topLeft.Add(backRight).Mul(1 / float32(2))
		bounds = mgl.Vec3{xdiff + maxScale.X() + padding.X(), ydiff + maxScale.Y() + padding.Y(), zdiff + maxScale.Z() + padding.Z()}
	}

	return hasPoints, center, bounds
}

func GetSettings(setting GSettings) []GSettings {
	settings := []GSettings{}
	for _, gset := range AllGSettings {
		if setting&gset != 0 {
			settings = append(settings, gset)
		}
	}
	return settings
}
