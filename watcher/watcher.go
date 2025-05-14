package watcher

import (
	"sync"

	"github.com/kabicin/kubechaser/renderer/gkube"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/client-go/kubernetes"
	"sigs.k8s.io/controller-runtime/pkg/client/config"
)

type WatchPoint struct {
	Name              string
	Namespace         string
	CreationTimestamp string
	ResourceVersion   string
}

type Watcher struct {
	Client                     *kubernetes.Clientset
	ClientMutex                *sync.Mutex
	UnstructuredConverterMutex *sync.Mutex

	MainCluster      *gkube.GCluster
	MainClusterMutex *sync.Mutex

	NamespacePoints  *sync.Map
	ReplicaSetPoints *sync.Map
	DeploymentPoints *sync.Map
	PodPoints        *sync.Map
}

func (watcher *Watcher) ToUnstructuredSync(obj interface{}) (map[string]interface{}, error) {
	watcher.UnstructuredConverterMutex.Lock()
	u, err := runtime.DefaultUnstructuredConverter.ToUnstructured(obj)
	watcher.UnstructuredConverterMutex.Unlock()
	return u, err
}

func (watcher *Watcher) Init(cluster *gkube.GCluster) {
	clientset, err := kubernetes.NewForConfig(config.GetConfigOrDie())
	if err != nil {
		panic(err.Error())
	}
	watcher.Client = clientset

	watcher.NamespacePoints = &sync.Map{}
	watcher.DeploymentPoints = &sync.Map{}
	watcher.ReplicaSetPoints = &sync.Map{}
	watcher.PodPoints = &sync.Map{}

	watcher.MainCluster = cluster
	watcher.MainClusterMutex = &sync.Mutex{}
	watcher.ClientMutex = &sync.Mutex{}
	watcher.UnstructuredConverterMutex = &sync.Mutex{}

	go watcher.WatchNamespaces()
}
