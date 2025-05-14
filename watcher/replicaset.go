package watcher

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/kabicin/kubechaser/renderer/gkube"
	corev1 "k8s.io/api/apps/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/apimachinery/pkg/watch"
)

type ReplicaSetPoint struct {
	WatchPoint
	Replicas int32
}

func (p *ReplicaSetPoint) String() string {
	return fmt.Sprintf("ReplicaSet %s (%s) - %s", p.Name, p.CreationTimestamp, p.ResourceVersion)
}

func (p *ReplicaSetPoint) Init(obj *corev1.ReplicaSet) {
	p.Name = obj.GetObjectMeta().GetName()
	p.CreationTimestamp = obj.GetObjectMeta().GetCreationTimestamp().GoString()
	p.ResourceVersion = obj.GetResourceVersion()
	if obj.Spec.Replicas != nil {
		p.Replicas = *obj.Spec.Replicas
	}
}

func ParseReplicaSetPoint(d *corev1.ReplicaSet) *ReplicaSetPoint {
	p := &ReplicaSetPoint{}
	p.Init(d)
	return p
}

func (watcher *Watcher) ParseReplicaSet(rawReplicaSet map[string]interface{}) (*corev1.ReplicaSet, error) {
	ns := &corev1.ReplicaSet{}
	watcher.UnstructuredConverterMutex.Lock()
	defer watcher.UnstructuredConverterMutex.Unlock()
	if err := runtime.DefaultUnstructuredConverter.FromUnstructured(rawReplicaSet, ns); err != nil {
		return nil, err
	}
	return ns, nil
}

func (watcher *Watcher) WatchReplicaSets(nsName string) {
	watcher.ClientMutex.Lock()
	watchInterface, _ := watcher.Client.AppsV1().ReplicaSets(nsName).Watch(context.TODO(), metav1.ListOptions{Watch: true})
	ch := watchInterface.ResultChan()
	watcher.ClientMutex.Unlock()
	for {
		select {
		case e, ok := <-ch:
			if !ok {
				return
			}
			if e.Type == watch.Added {
				rawReplicaSet, _ := watcher.ToUnstructuredSync(e.Object)
				replicaset, err := watcher.ParseReplicaSet(rawReplicaSet)
				if err != nil {
					return
				}

				replicasetName := replicaset.GetName()
				_, found := watcher.ReplicaSetPoints.Load(replicasetName)
				if !found {
					replicaset, err := watcher.ParseReplicaSet(rawReplicaSet)
					if err != nil {
						continue
					}

					// watcher will notice any owner references to a Deployment
					ownerName := ""
					ownerType := ""
					ownerRefs := replicaset.GetObjectMeta().GetOwnerReferences()
					if len(ownerRefs) > 0 && ownerRefs[0].Kind == "Deployment" {
						ownerName = ownerRefs[0].Name
						ownerType = ownerRefs[0].Kind
					}
					// add replicaset point
					watcher.ReplicaSetPoints.Store(replicasetName, ParseReplicaSetPoint(replicaset))
					watcher.MainCluster.PushGObjectEvent(gkube.GCREATE, gkube.GREPLICASET, replicasetName, replicaset.Namespace, gkube.GNONE, gkube.GSETTING_NONE, nil, &gkube.GReplicaSetStatus{
						OwnerReferenceName: ownerName,
						OwnerReferenceType: ownerType,
						ReadyReplicas:      replicaset.Status.ReadyReplicas,
						Replicas:           replicaset.Status.Replicas,
					}, -1, rawReplicaSet)
					log.Println("ADDED replicaset " + replicasetName)
				}
			} else if e.Type == watch.Modified {
				rawReplicaSet, _ := watcher.ToUnstructuredSync(e.Object)
				replicaset, err := watcher.ParseReplicaSet(rawReplicaSet)
				if err != nil {
					return
				}

				replicasetName := replicaset.GetName()
				_, found := watcher.ReplicaSetPoints.Load(replicasetName)
				if found {
					replicaset, err := watcher.ParseReplicaSet(rawReplicaSet)
					if err != nil {
						continue
					}
					// modify replicaset point
					watcher.ReplicaSetPoints.Store(replicasetName, ParseReplicaSetPoint(replicaset))
					log.Println("MODIFIED replicaset " + replicasetName)
				} else {

				}
			} else if e.Type == watch.Deleted {
				rawReplicaSet, _ := watcher.ToUnstructuredSync(e.Object)
				replicaset, err := watcher.ParseReplicaSet(rawReplicaSet)
				if err != nil {
					return
				}
				replicasetName := replicaset.GetName()
				_, found := watcher.ReplicaSetPoints.Load(replicasetName)
				if found {
					// delete replicaset point
					watcher.ReplicaSetPoints.Delete(replicasetName)
					log.Println("DELETED replicaset " + replicasetName)
				}
			}
		case <-time.After(30 * time.Minute):
			return
		}
	}
}
