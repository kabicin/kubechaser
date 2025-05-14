package gkube

import (
	"fmt"
	"slices"
	"strings"

	"github.com/kabicin/kubechaser/renderer/utils"
)

type GSignatureConnection struct {
	resource  GResource
	name      string
	namespace string
}

func (gs *GSignatureConnection) Equals(o *GSignatureConnection) bool {
	return gs.resource == o.resource && gs.name == o.name && gs.namespace == o.namespace
}

func (gs *GSignatureConnection) String() string {
	return fmt.Sprintf("%s|%s|%d", gs.namespace, gs.name, gs.resource)
}

type SlotResource struct {
	object                      GObject
	resource                    GResource
	name                        string
	namespace                   string
	connectedResourceSignatures []GSignatureConnection
}

func (sr *SlotResource) GetObject() GObject {
	return sr.object
}

// Returns >0 if sr1 was found in sr2's connected resources, <0 if sr2 was found in sr1's connected resources, and 0 if no collision
func (sr1 *SlotResource) hasCollision(sr2 SlotResource) int {
	debug := false
	for _, str := range debugStrings {
		if strings.HasPrefix(sr1.name, str) || strings.HasPrefix(sr2.name, str) {
			debug = true
		}
	}
	// does sr1 exist in sr2's connected resource signatures?
	for _, sigConn := range sr2.connectedResourceSignatures {
		if debug {
			fmt.Printf("    - comparing sr1 %s/%s with sr2.conn %s/%s \n", sr1.name, sr1.namespace, sigConn.name, sigConn.namespace)
		}
		if sr1.resource == sigConn.resource && sr1.name == sigConn.name && sr1.namespace == sigConn.namespace {
			return 1
		}
	}
	// or, does sr2 exist in sr1's connected resource signatures?
	for _, sigConn := range sr1.connectedResourceSignatures {
		if debug {
			fmt.Printf("    - comparing sr2 %s/%s with sr1.conn %s/%s \n", sr2.name, sr2.namespace, sigConn.name, sigConn.namespace)
		}
		if sr2.resource == sigConn.resource && sr2.name == sigConn.name && sr2.namespace == sigConn.namespace {
			return -1
		}
	}
	return 0
}

func GetResourceIndex(resource GResource) int {
	if resource == GDEPLOYMENT {
		return 0
	} else if resource == GREPLICASET {
		return 1
	} else if resource == GPOD {
		return 2
	}
	return 100
}

func (gc *GCluster) EvictSlot(deleteResource SlotResource) {
	if _, found := gc.slots[deleteResource.namespace]; found {
		foundI := -1
		foundJ := -1
		for i, slotRow := range gc.slots[deleteResource.namespace] {
			for j, slot := range slotRow {
				if slot.GetSignature() == deleteResource.GetSignature() {
					foundI = i
					foundJ = j
				}
			}
		}
		// if found, evict the slot and GOBJECT with it
		if foundI != -1 && foundJ != -1 {
			fmt.Println("Evicting...")
			object := gc.slots[deleteResource.namespace][foundI][foundJ].object // hold handle to the GOBJECT

			if foundJ == 0 {
				gc.slots[deleteResource.namespace][foundI] = gc.slots[deleteResource.namespace][foundI][1:]
			} else if foundJ == len(gc.slots[deleteResource.namespace][foundI])-1 {
				gc.slots[deleteResource.namespace][foundI] = gc.slots[deleteResource.namespace][foundI][:len(gc.slots[deleteResource.namespace][foundI])-1]
			} else {
				gc.slots[deleteResource.namespace][foundI] = append(gc.slots[deleteResource.namespace][foundI][:foundJ], gc.slots[deleteResource.namespace][foundI][foundJ+1:]...) // cluster to remove ref to GOBJECT
			}
			// if the surrounding array is empty, remove it
			if len(gc.slots[deleteResource.namespace][foundI]) == 0 {
				if foundI == 0 {
					gc.slots[deleteResource.namespace] = gc.slots[deleteResource.namespace][1:]
				} else if foundI == len(gc.slots[deleteResource.namespace])-1 {
					gc.slots[deleteResource.namespace] = gc.slots[deleteResource.namespace][:len(gc.slots[deleteResource.namespace][foundI])-1]
				} else {
					gc.slots[deleteResource.namespace] = append(gc.slots[deleteResource.namespace][:foundI], gc.slots[deleteResource.namespace][foundI+1:]...)
				}
			}
			gc.mainScene.DeleteObject(object.GetObject()) // remove from the main scene - stops drawing
			gc.DeleteGObject(object)                      // delete the GOBJECT from cluster
		}
	}
}

func (gc *GCluster) SetDeletingSlot(deleteResource SlotResource) {
	if _, found := gc.slots[deleteResource.namespace]; found {
		foundI := -1
		foundJ := -1
		for i, slotRow := range gc.slots[deleteResource.namespace] {
			for j, slot := range slotRow {
				if slot.GetSignature() == deleteResource.GetSignature() {
					foundI = i
					foundJ = j
				}
			}
		}
		// if found, mark slot as being deleted
		if foundI != -1 && foundJ != -1 {
			fmt.Println("Mark slot as deleting...")
			gc.slots[deleteResource.namespace][foundI][foundJ].object.SetDeleting() // signal to the Scene to send the delete color animation
		}
	}
}

// RESOURCE SLOTS
//
//	0   D - RS - P1 - P2
//	1   D - RS - P1
//	2   D - RS - P1 - P2 - P3
//	3   D - RS - P1

// lastOffsets[namespace][GRESOURCE] = mgl.Vec3{}
//
//	slots[namespace][0] = []SlotResource{
//			{object: &rs1, resource: GREPLICASET, name: "rs1", namespace: namespace, connectedResourceSignatures: []GSignatureConnection{resource: GDEPLOYMENT, name: "d1", namespace: namespace}},
//			{object: &d1, resource: GDEPLOYMENT, name: "d1", namespace: namespace, connectedResourceSignatures:  []GSignatureConnection{}}
//	}

func (gc *GCluster) CreateAndReserveSlot(name, namespace string, object GObject, reservee GResource, connectedResources []GSignatureConnection) {
	sr := SlotResource{
		name:                        name,
		namespace:                   namespace,
		object:                      object,
		resource:                    reservee,
		connectedResourceSignatures: connectedResources,
	}
	debug := false
	for _, str := range debugStrings {
		if strings.HasPrefix(name, str) {
			debug = true
		}
	}
	if debug {
		fmt.Printf("- CREATE/RESERVE SLOT for..\n    - %s (%s) - %s\n", name, namespace, getGResourceName(reservee))
	}
	// reserve slot
	gc.ReserveSlot(namespace, sr)
}

func getReplicaSetInsertIndex(slotRow []SlotResource) int {
	insertIndex := 0
	if len(slotRow) == 0 {
		return insertIndex
	}
	for i, slot := range slotRow {
		if slot.resource == GPOD || slot.resource == GREPLICASET {
			return i
		}
		insertIndex = i + 1
	}
	return insertIndex
}

func getIndex(arr []string, elem string) int {
	for j, v := range arr {
		if v == elem {
			return j
		}
	}
	return -1
}

func syncSlotOffsets(nsIndex int, rowIndex int, sr []SlotResource) {
	// rowIndex provides x offset
	// position in namespaceSlots provides y offset
	// slotResourceIndex provides z offset

	stride := float32(6.0)
	xOffset := float32(rowIndex) * stride
	yOffset := float32(nsIndex) * stride
	zOffset := float32(-1)
	for i := range sr {
		zOffset = float32(i) * stride
		sr[i].GetObject().GetCurrentOffset()[0] = xOffset
		sr[i].GetObject().GetCurrentOffset()[1] = yOffset
		sr[i].GetObject().GetCurrentOffset()[2] = zOffset
	}
	// fmt.Printf("    - SYNC slot at (%f,%f,%f) - slot size is now %d\n", xOffset, yOffset, zOffset, len(sr))

}

// TODO: consolidate with type parameters in utils.go
func (sr *SlotResource) LessThan(o *SlotResource) bool {
	return GetResourceIndex(sr.resource) < GetResourceIndex(o.resource)
}

// TODO: consolidate with type parameters in utils.go
func merge(left []SlotResource, right []SlotResource) []SlotResource {
	i := 0
	j := 0
	n := len(left)
	m := len(right)
	out := []SlotResource{}
	for i < n && j < m {
		if left[i].LessThan(&right[j]) {
			out = append(out, left[i])
			i++
		} else {
			out = append(out, right[j])
			j++
		}
	}
	for i < n {
		out = append(out, left[i])
		i++
	}
	for j < m {
		out = append(out, right[j])
		j++
	}
	return out
}

// TODO: consolidate with type parameters in utils.go
func MergeSort(arr []SlotResource) []SlotResource {
	n := len(arr)
	if n <= 1 {
		return arr
	}
	mid := n / 2
	left := MergeSort(arr[:mid])
	right := MergeSort(arr[mid:])
	return merge(left, right)
}

var debugStrings = []string{
	"coredns",
	"local-path",
}

func (slot *SlotResource) GetSignature() string {
	return fmt.Sprintf("%s-%s-%s", slot.namespace, slot.name, getGResourceName(slot.resource))
}

func mergeAndPickUnique2(sr1 []SlotResource, sruniq []SlotResource) []SlotResource {
	out := []SlotResource{}
	outSignatures := []string{} // namespace-name-GRESOURCE
	for _, slot := range sr1 {
		outSignatures = append(outSignatures, slot.GetSignature())
		out = append(out, slot)
	}
	// only append unique entries from sruniq
	for _, slot := range sruniq {
		sig := slot.GetSignature()
		if !slices.Contains(outSignatures, sig) {
			outSignatures = append(outSignatures, slot.GetSignature())
			out = append(out, slot)
		}
	}
	return out
}

func (gc *GCluster) flattenSlots(namespace string, sr SlotResource, insertedRowIndex int) {
	debug := false
	for _, str := range debugStrings {
		if strings.HasPrefix(sr.name, str) {
			debug = true
		}
	}

	collidedSlots := []int{}
	for i := range gc.slots[namespace] {
		if insertedRowIndex != i {
			for j := range gc.slots[namespace][i] {
				currSlot := gc.slots[namespace][i][j]
				if collision := currSlot.hasCollision(sr); collision != 0 {
					if !slices.Contains(collidedSlots, i) {
						collidedSlots = append(collidedSlots, i)
					}
				}
			}
		}
	}
	// merge all collided slots with insertedRowIndex
	for csi := range collidedSlots {
		if debug {
			fmt.Printf("MERGE COLLIDED SLOT for BEFORE %s\n", sr.name)
			for _, slot := range gc.slots[namespace][insertedRowIndex] {
				fmt.Printf("     - slot: %s (%s)\n", getGResourceName(slot.resource), slot.name)
			}
			fmt.Printf("MERGE COLLIDED SLOT csi for BEFORE %s\n", sr.name)
			for _, slot := range gc.slots[namespace][csi] {
				fmt.Printf("     - slot: %s (%s)\n", getGResourceName(slot.resource), slot.name)
			}
		}
		gc.slots[namespace][insertedRowIndex] = MergeSort(mergeAndPickUnique2(gc.slots[namespace][insertedRowIndex], gc.slots[namespace][csi])) // TOOD: requires animations
		if debug {
			fmt.Printf("MERGE COLLIDED SLOT for AFTER %s\n", sr.name)
			for _, slot := range gc.slots[namespace][insertedRowIndex] {
				fmt.Printf("     - slot: %s (%s)\n", getGResourceName(slot.resource), slot.name)
			}
			fmt.Printf("MERGE COLLIDED SLOT csi for AFTER %s\n", sr.name)
			for _, slot := range gc.slots[namespace][csi] {
				fmt.Printf("     - slot: %s (%s)\n", getGResourceName(slot.resource), slot.name)
			}
		}
	}
	// sort collided slots
	sortedCollidedSlots := utils.MergeSort(collidedSlots)
	for i := len(sortedCollidedSlots) - 1; i >= 0; i-- {
		ind := sortedCollidedSlots[i]
		if debug {
			fmt.Printf("REMOVING COLLIDED SLOT: %d len %d\n", ind, len(gc.slots[namespace]))
		}
		gc.slots[namespace] = append(gc.slots[namespace][:ind], gc.slots[namespace][ind+1:]...) // remove collided slots
		if debug {
			fmt.Printf("REMOVING COLLIDED AFTER SLOT: len %d\n", len(gc.slots[namespace]))
		}
	}

	// slots have been moved, so offsets must be resynced
	nsIndex := getIndex(gc.namespaceSlots, namespace)
	for rowIndex := range gc.slots[namespace] {
		syncSlotOffsets(nsIndex, rowIndex, gc.slots[namespace][rowIndex])
	}
}

func (gc *GCluster) ReserveSlot(namespace string, sr SlotResource) {
	// if no slots exist, init the array
	if _, found := gc.slots[namespace]; !found {
		gc.slots[namespace] = [][]SlotResource{}
	}

	// populate namespace slots, if this is a new namespace
	if !slices.Contains(gc.namespaceSlots, namespace) {
		gc.namespaceSlots = append(gc.namespaceSlots, namespace)
	}

	nsIndex := getIndex(gc.namespaceSlots, namespace)
	// if there are no slots, reserve create the first slot row
	if len(gc.slots[namespace]) == 0 {
		gc.slots[namespace] = append(gc.slots[namespace], []SlotResource{sr})
		syncSlotOffsets(nsIndex, 0, gc.slots[namespace][0])
		fmt.Println("RESERVING FIRST SLOT")
		return
	}

	// determine if there is an existing slot row that I can insert into
	inserted := false
	insertRowIndex := -1
	for rowIndex, slotRow := range gc.slots[namespace] {
		for _, slot := range slotRow { // for colIndex, slot := range slotRow {
			if collisionSkew := slot.hasCollision(sr); collisionSkew != 0 {
				// there is a collision
				if sr.resource == GDEPLOYMENT {
					gc.slots[namespace][rowIndex] = append([]SlotResource{sr}, gc.slots[namespace][rowIndex]...) // prepend sr into the Slot Row at rowIndex
				} else if sr.resource == GREPLICASET { // insert sr after all deployments and before all pods in the Slot Row at rowIndex
					insertIndex := getReplicaSetInsertIndex(gc.slots[namespace][rowIndex])
					if insertIndex >= len(gc.slots[namespace][rowIndex]) {
						gc.slots[namespace][rowIndex] = append(gc.slots[namespace][rowIndex], sr) // case 1: insert index is out of array bounds
					} else {
						gc.slots[namespace][rowIndex] = append(gc.slots[namespace][rowIndex][:insertIndex], append([]SlotResource{sr}, gc.slots[namespace][rowIndex][insertIndex:]...)...) // case 2: insert index is in array bounds
					}
				} else {
					gc.slots[namespace][rowIndex] = append(gc.slots[namespace][rowIndex], sr) // append sr into the Slot Row at rowIndex
				}
				syncSlotOffsets(nsIndex, rowIndex, gc.slots[namespace][rowIndex]) // refresh the Slot Row by syncing all slot offsets
				// insertIndex := len(gc.slots[namespace][rowIndex]) - 1
				inserted = true // find the index of insertion for sr
				insertRowIndex = rowIndex
			}
		}
	}
	// otherwise, if there was nothing initially inserted, append a new slot row
	if !inserted {
		gc.slots[namespace] = append(gc.slots[namespace], []SlotResource{sr})
		lastInsertIndex := len(gc.slots[namespace]) - 1
		syncSlotOffsets(nsIndex, lastInsertIndex, gc.slots[namespace][lastInsertIndex])
		inserted = true
		insertRowIndex = lastInsertIndex
	}

	// finally, if inserted, check if other unique slots still cause collision with sr, then flatten the slots together
	if inserted {
		gc.flattenSlots(namespace, sr, insertRowIndex)
	}
}
