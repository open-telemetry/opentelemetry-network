# Kubernetes Collector #

The _kubernetes collector_ gathers information from a Kubernetes API server on events like pod creation and deletion.

Usual deployments will have one kubernetes collector per Kubernetes cluster.


## Running ##

The kubernetes collector is composed of two binaries: _k8s-watcher_ and _k8s-relay_.

K8s-watcher connects to the Kubernetes cluster and listens for notifications on certain events, e.g. pod creation and deletion.
It passes this information to k8s-relay, which is in turn connected to the reducer.

To connect to the Kubernetes API server, k8s-watcher needs to have the right permissions.
For local development, if you have access to the Kubernetes cluster through the _kubectl_ command,
then k8s-watcher should be able to access the same cluster that is configured in the kubectl's current context.

Usually, kubernetes collector will be deployed through a Kubernetes deployment object,
where k8s-watcher and k8s-relay are two containers running in one pod.


## Environment variables ##

- `EBPF_NET_INTAKE_HOST`: IP address or host name of the reducer to which telemetry is to be sent.
- `EBPF_NET_INTAKE_PORT`: TCP port number on which the reducer is listening for collector connections. Usually 8000.
- `EBPF_NET_DATA_DIR`: Directory in which the program will read and potentially write data files.
  If not specified the current working directory will be used.
- `EBPF_NET_LOG_FILE_PATH`: Location of the file in which logging messages are written. Default value is /var/log/ebpf_net.log.
