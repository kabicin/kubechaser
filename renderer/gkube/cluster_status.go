package gkube

type GDeploymentStatus struct {
	ReadyReplicas int32
	Replicas      int32
}

type GReplicaSetStatus struct {
	ReadyReplicas      int32
	Replicas           int32
	OwnerReferenceName string
	OwnerReferenceType string
}

type GPodStatus struct {
	Up                 bool
	Index              int32
	OwnerReferenceName string
	OwnerReferenceType string
}

type GWireStatus struct {
	Up    bool
	Index int32
}
