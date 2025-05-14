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

type DeploymentPoint struct {
	WatchPoint
	Replicas int32
}

func (p *DeploymentPoint) String() string {
	return fmt.Sprintf("Deployment %s (%s) - %s", p.Name, p.CreationTimestamp, p.ResourceVersion)
}

func (p *DeploymentPoint) Init(obj *corev1.Deployment) {
	p.Name = obj.GetObjectMeta().GetName()
	p.CreationTimestamp = obj.GetObjectMeta().GetCreationTimestamp().GoString()
	p.ResourceVersion = obj.GetResourceVersion()
	if obj.Spec.Replicas != nil {
		p.Replicas = *obj.Spec.Replicas
	}
}

func ParseDeploymentPoint(d *corev1.Deployment) *DeploymentPoint {
	p := &DeploymentPoint{}
	p.Init(d)
	return p
}

func (watcher *Watcher) ParseDeployment(rawDeployment map[string]interface{}) (*corev1.Deployment, error) {
	ns := &corev1.Deployment{}
	watcher.UnstructuredConverterMutex.Lock()
	defer watcher.UnstructuredConverterMutex.Unlock()
	if err := runtime.DefaultUnstructuredConverter.FromUnstructured(rawDeployment, ns); err != nil {
		return nil, err
	}
	return ns, nil
}

func (watcher *Watcher) WatchDeployments(nsName string) {
	watcher.ClientMutex.Lock()
	watchInterface, _ := watcher.Client.AppsV1().Deployments(nsName).Watch(context.TODO(), metav1.ListOptions{Watch: true})
	ch := watchInterface.ResultChan()
	watcher.ClientMutex.Unlock()
	for {
		select {
		case e, ok := <-ch:
			if !ok {
				return
			}
			if e.Type == watch.Added {
				rawDeployment, _ := watcher.ToUnstructuredConcurrent(e.Object)
				deploy, err := watcher.ParseDeployment(rawDeployment)
				if err != nil {
					return
				}

				deployName := deploy.GetName()
				_, found := watcher.DeploymentPoints.Load(deployName)
				if !found {
					deploy, err := watcher.ParseDeployment(rawDeployment)
					if err != nil {
						continue
					}
					// add deployment point
					watcher.DeploymentPoints.Store(deployName, ParseDeploymentPoint(deploy))
					watcher.MainCluster.PushGObjectEvent(gkube.GCREATE, gkube.GDEPLOYMENT, deployName, deploy.Namespace, gkube.GNONE, gkube.GSETTING_NONE, nil, &gkube.GDeploymentStatus{
						ReadyReplicas: deploy.Status.ReadyReplicas,
						Replicas:      deploy.Status.Replicas,
					}, -1, rawDeployment)
					log.Println("ADDED deployment " + deployName)
				}
			} else if e.Type == watch.Modified {
				rawDeployment, _ := watcher.ToUnstructuredConcurrent(e.Object)
				deploy, err := watcher.ParseDeployment(rawDeployment)
				if err != nil {
					return
				}

				deployName := deploy.GetName()
				_, found := watcher.DeploymentPoints.Load(deployName)
				if found {
					deploy, err := watcher.ParseDeployment(rawDeployment)
					if err != nil {
						continue
					}
					// modify deployment point
					watcher.DeploymentPoints.Store(deployName, ParseDeploymentPoint(deploy))
					log.Println("MODIFIED deployment " + deployName)
				} else {

				}
			} else if e.Type == watch.Deleted {
				rawDeployment, _ := watcher.ToUnstructuredConcurrent(e.Object)
				deploy, err := watcher.ParseDeployment(rawDeployment)
				if err != nil {
					return
				}
				deployName := deploy.GetName()
				_, found := watcher.DeploymentPoints.Load(deployName)
				if found {
					// delete deployment point
					watcher.DeploymentPoints.Delete(deployName)
					log.Println("DELETED deployment " + deployName)
				}
			}
		case <-time.After(30 * time.Minute):
			return
		}
	}
}
