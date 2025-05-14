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

type NamespacePoint struct {
	WatchPoint
}

func (np *NamespacePoint) String() string {
	return fmt.Sprintf("Namespace %s (%s) - %s", np.Name, np.CreationTimestamp, np.ResourceVersion)
}

func (np *NamespacePoint) Init(ns *v1.Namespace) {
	np.Name = ns.GetObjectMeta().GetName()
	np.CreationTimestamp = ns.GetObjectMeta().GetCreationTimestamp().GoString()
	np.ResourceVersion = ns.GetResourceVersion()
}

func ParseNamespacePoint(ns *v1.Namespace) *NamespacePoint {
	np := &NamespacePoint{}
	np.Init(ns)
	return np
}

func (watcher *Watcher) ParseNamespace(rawNamespace map[string]interface{}) (*v1.Namespace, error) {
	ns := &v1.Namespace{}
	watcher.UnstructuredConverterMutex.Lock()
	defer watcher.UnstructuredConverterMutex.Unlock()
	if err := runtime.DefaultUnstructuredConverter.FromUnstructured(rawNamespace, ns); err != nil {
		return nil, err
	}
	return ns, nil
}

func (watcher *Watcher) WatchNamespaces() {
	watcher.ClientMutex.Lock()
	watchInterface, _ := watcher.Client.CoreV1().Namespaces().Watch(context.TODO(), metav1.ListOptions{Watch: true})
	ch := watchInterface.ResultChan()
	watcher.ClientMutex.Unlock()
	for {
		select {
		case e, ok := <-ch:
			if !ok {
				return
			}
			if e.Type == watch.Added {
				rawNamespace, _ := watcher.ToUnstructuredSync(e.Object)
				ns, err := watcher.ParseNamespace(rawNamespace)
				if err != nil {
					return
				}

				nsName := ns.GetName()
				_, found := watcher.NamespacePoints.Load(nsName)
				if !found {
					ns, err := watcher.ParseNamespace(rawNamespace)
					if err != nil {
						continue
					}
					// add namespace point
					watcher.NamespacePoints.Store(nsName, ParseNamespacePoint(ns))
					watcher.MainCluster.PushGObjectEvent(gkube.GCREATE, gkube.GNAMESPACEOBJECTFRAME, nsName, ns.Namespace, gkube.GNONE, gkube.GSETTING_NONE, nil, &gkube.GNamespaceObjectFrameStatus{}, -1, rawNamespace)
					log.Println("ADDED namespace " + nsName)

					// Add watchers for this namespace
					go watcher.WatchDeployments(nsName)
					go watcher.WatchReplicaSets(nsName)
					go watcher.WatchPods(nsName)

				}
			} else if e.Type == watch.Deleted {
				rawNamespace, _ := watcher.ToUnstructuredSync(e.Object)
				ns, err := watcher.ParseNamespace(rawNamespace)
				if err != nil {
					return
				}
				nsName := ns.GetName()
				_, found := watcher.NamespacePoints.Load(nsName)
				if found {
					// delete namespace point
					watcher.NamespacePoints.Delete(nsName)
					log.Println("DELETED namespace " + nsName)
				}

			}
		case <-time.After(30 * time.Minute):
			return
		}
	}
}
