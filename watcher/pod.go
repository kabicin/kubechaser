package watcher

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/kabicin/kubechaser/renderer/gkube"
	v1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/apimachinery/pkg/watch"
)

type PodPoint struct {
	WatchPoint
	Up                int
	NumInitContainers int
	NumContainers     int
}

func (p *PodPoint) String() string {
	return fmt.Sprintf("Pod %s (%s) - %s", p.Name, p.CreationTimestamp, p.ResourceVersion)
}

func (p *PodPoint) Init(obj *v1.Pod) {
	p.Name = obj.GetObjectMeta().GetName()
	p.CreationTimestamp = obj.GetObjectMeta().GetCreationTimestamp().GoString()
	p.ResourceVersion = obj.GetResourceVersion()
	p.NumContainers = len(obj.Spec.Containers)
	p.NumInitContainers = len(obj.Spec.InitContainers)
}

func ParsePodPoint(d *v1.Pod) *PodPoint {
	p := &PodPoint{}
	p.Init(d)
	return p
}

func (watcher *Watcher) ParsePod(rawPod map[string]interface{}) (*v1.Pod, error) {
	ns := &v1.Pod{}
	watcher.UnstructuredConverterMutex.Lock()
	defer watcher.UnstructuredConverterMutex.Unlock()
	if err := runtime.DefaultUnstructuredConverter.FromUnstructured(rawPod, ns); err != nil {
		return nil, err
	}
	return ns, nil
}

func (watcher *Watcher) WatchPods(nsName string) {
	watcher.ClientMutex.Lock()
	watchInterface, _ := watcher.Client.CoreV1().Pods(nsName).Watch(context.TODO(), metav1.ListOptions{Watch: true})
	ch := watchInterface.ResultChan()
	watcher.ClientMutex.Unlock()
	for {
		select {
		case e, ok := <-ch:
			if !ok {
				return
			}
			if e.Type == watch.Added {
				rawPod, _ := watcher.ToUnstructuredConcurrent(e.Object)
				pod, err := watcher.ParsePod(rawPod)
				if err != nil {
					return
				}

				podName := pod.GetName()
				_, found := watcher.PodPoints.Load(podName)
				if !found {
					pod, err := watcher.ParsePod(rawPod)
					if err != nil {
						continue
					}

					// watcher will notice any owner references to a ReplicaSet
					ownerName := ""
					ownerType := ""
					ownerRefs := pod.GetObjectMeta().GetOwnerReferences()
					if len(ownerRefs) > 0 && ownerRefs[0].Kind == "ReplicaSet" {
						ownerName = ownerRefs[0].Name
						ownerType = ownerRefs[0].Kind
					}

					// add pod point
					watcher.PodPoints.Store(podName, ParsePodPoint(pod))
					watcher.MainCluster.PushGObjectEvent(gkube.GCREATE, gkube.GPOD, podName, pod.Namespace, gkube.GNONE, gkube.GSETTING_NONE, nil, &gkube.GPodStatus{
						OwnerReferenceName: ownerName,
						OwnerReferenceType: ownerType,
						Up:                 pod.Status.Phase == v1.PodRunning,
						Index:              0,
					}, -1, rawPod)
					log.Println("ADDED pod " + podName)
				}
			} else if e.Type == watch.Modified {
				rawPod, _ := watcher.ToUnstructuredConcurrent(e.Object)
				pod, err := watcher.ParsePod(rawPod)
				if err != nil {
					return
				}

				podName := pod.GetName()
				_, found := watcher.PodPoints.Load(podName)
				if found {
					pod, err := watcher.ParsePod(rawPod)
					if err != nil {
						continue
					}
					// modify pod point
					watcher.PodPoints.Store(podName, ParsePodPoint(pod))
					log.Println("MODIFIED pod " + podName)
				} else {

				}
			} else if e.Type == watch.Deleted {
				rawPod, _ := watcher.ToUnstructuredConcurrent(e.Object)
				pod, err := watcher.ParsePod(rawPod)
				if err != nil {
					return
				}
				podName := pod.GetName()
				_, found := watcher.PodPoints.Load(podName)
				if found {
					// delete pod point
					watcher.PodPoints.Delete(podName)
					watcher.MainCluster.PushGObjectEvent(gkube.GDELETE, gkube.GPOD, podName, pod.Namespace, gkube.GNONE, gkube.GSETTING_NONE, nil, &gkube.GPodStatus{
						Up:    pod.Status.Phase == v1.PodRunning,
						Index: 0,
					}, -1, rawPod)
					log.Println("DELETED pod " + podName)
				}
			}
		case <-time.After(30 * time.Minute):
			return
		}
	}
}
