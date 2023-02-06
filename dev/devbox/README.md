devbox vm
=========

The devbox VM is offered as a standalone VM capable of running the whole
OpenTelemetry-eBPF pipeline.

It doesn't run anything by default but provides scripts to run the following
components within the VM itself, connecting to other components inside the VM:
- `reducer`
- `kernel-collector` / `cloud-collector` / `k8s-relay` / `k8s-watcher`
- tsdb (prometheus)
- backend DB (`mariadb`)

Before using the VM, its vagrant base box must be built. This is a one-time
step. Refer to the "building a vagrant base devbox" for more details.

Images for OpenTelemetry-eBPF components are taken from a local docker registry on port
`5000`. The intended use case is to reverse forward a docker registry running
on the host machine into which `benv` built images are pushed using the
[`dev/push_to_local_registry.sh`](../push_to_local_registry.sh).

Pushing to the docker registry can either be done using the appropriate cmake
targets (run `make help | grep '_docker_registry$'` within benv to list them),
or with the [`dev/push_to_local_registry.sh`](../push_to_local_registry.sh)
script.

# requirements
- [packer](https://packer.io/)
- [vagrant](https://www.vagrantup.com/)
- [vagrant-sshfs plugin](https://github.com/dustymabe/vagrant-sshfs#install-plugin)
- [virtualbox](https://www.virtualbox.org/)

# using pre-spec'ed devboxes

There are some pre-spec'ed devboxes for a few distros, including:
- [Debian 9: `stretch`](boxes/debian-stretch)
- [Ubuntu 20.04: `focal`](boxes/ubuntu-focal)

Before using those, perform a one-time build of the base vagrant box by running
the `build.sh` script from the distro directory.

After the base box is built, change into the distro's directory and run
`./run.sh`. This spins up the VM and opens an interaction `ssh` session.

By default, the `run.sh` script will forward ports for the following components:
- reverse forward the host's docker registry (port `5000`) to the devbox;
- forward the devbox's `prometheus` to the host (on port `59090`).

# create a Kubernetes cluster

To create a Kubernetes cluster, run `~/k8s/init.sh` within the VM.

This script will make sure Kubernetes dependencies are installed and the proper
settings are applied to the system. It will also initialize the cluster.

Cluster configuration is stored within the VM under `~/.kube/config` with
Kubernetes context name `kubernetes-admin@devbox`.

# running microservices-demo

To deploy the microservices demo, run `~/k8s/demo.sh` within the VM.

To use a different loadgen configuration, pass its filename as an argument to `demo.sh`.
The configuration file will be looked up relative to `$EBPF_NET_SRC_ROOT/dev/devbox/source/k8s/loadgen`.

# deploying aget images

The usual helm scripts can be used to deploy agents to the VM, using the cluster `vm`.
By default, the local docker registry will be used to pull images from.

To deploy agents to the VM's kubernetes, run:
```
cd ~/src/infrastructure/kubernetes/agents
./install.sh vm
```
