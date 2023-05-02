# Devbox VM #

Devbox VMs are standalone VMs containing everything necessary to run the full OpenTelemetry eBPF pipeline.  They are isolated sandboxed environments of various Linux distributions intended to be used for development, test, debug, and demonstration purposes.  Scripts are provided to run the following components:
- OpenTelemetry eBPF agents: kernel-collector / cloud-collector / k8s-relay / k8s-watcher
- OpenTelemetry eBPF reducer
- OpenTelemetry Collector
- Prometheus time series database
- Kubernetes environment
- Microservices Demos


## Requirements ##

- **VirtualBox** (https://www.virtualbox.org/wiki/Downloads)  
  Install as appropriate for your machine type.[^1]

- **Vagrant** (https://www.vagrantup.com/)  
  Linux: `sudo apt install -y vagrant`  
  MacOS: `brew install vagrant`

- **vagrant-sshfs plugin** (https://github.com/dustymabe/vagrant-sshfs#install-plugin)  
  Linux and MacOS: `vagrant plugin install vagrant-sshfs`


- **vagrant-scp plugin** (https://github.com/dustymabe/vagrant-sshfs#install-plugin)  
  Linux and MacOS: `vagrant plugin install vagrant-scp`

- **Packer** (https://www.packer.io/)  
  Linux:

        wget -O- https://apt.releases.hashicorp.com/gpg | gpg --dearmor | sudo tee /usr/share/keyrings/hashicorp-archive-keyring.gpg
        echo "deb [signed-by=/usr/share/keyrings/hashicorp-archive-keyring.gpg] https://apt.releases.hashicorp.com $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/hashicorp.list
        sudo apt update && sudo apt install packer

  MacOS: `brew install packer`[^2]

- **jq** (https://stedolan.github.io/jq)  
  Linux: `sudo apt install jq`  
  MacOS: `brew install jq`

- Environment Variables  
  This document, and many of the scripts referenced, use the environment variable EBPF_NET_SRC_ROOT to refer to the location of your clone of the OpenTelemetry eBPF repository.  It is recommended that you add this to your .bashrc or equivalent, for example:

        export EBPF_NET_SRC_ROOT="$HOME/work/opentelemetry-ebpf"


## Creating and running a devbox VM ##

There are devboxes configured for several distros, including:
- [CentOS 7](boxes/centos-7)
- [Debian 11: `bullseye`](boxes/debian-bullseye)
- [Ubuntu 20.04: `focal`](boxes/ubuntu-focal)
- [Ubuntu 22.04: `jammy`](boxes/ubuntu-jammy)


### Building the devbox VM ###

- Build the base Vagrant box.  This is a one-time step for each distro you intend to use.

        cd $EBPF_NET_SRC_ROOT/dev/devbox/boxes/<DISTRO>
        ./build.sh

- (Optional) Modify `$EBPF_NET_SRC_ROOT/dev/devbox/boxes/<DISTRO>/Vagrantfile` if your system has less than 32GiB or less than 12 CPUs  
  Modify `vm.memory` (default = 16384) and `vm.cpus` (default = 6) to be no more than half of your system’s memory/CPUs.

### Running the devbox ###

    cd $EBPF_NET_SRC_ROOT/dev/devbox/boxes/<DISTRO>
    ./run.sh

This will start the devbox VM and open an interaction ssh session.  The run.sh script also forwards ports as follows:
- reverse forward the host's Docker registry (port 5000) to the devbox VM
- forward the devbox VM's port 8080 to the host on port 58080
- forward the devbox VM's port 9090 to the host on port 59090

Once the devbox VM is running you can create additional interactive ssh session from other xterms with

    cd $EBPF_NET_SRC_ROOT/dev/devbox/boxes/<DISTRO>
    vagrant ssh

## Running the OpenTelemetry eBPF pipeline in a devbox VM ##

### Manually running the OpenTelemetry eBPF pipeline in a devbox VM ###

Run the following commands inside the devbox VM from interactive ssh sessions started by “./run.sh” or “vagrant ssh”, described above.

1. Run OpenTelemetry Collector
    - To see all options:

            ./otelcol-gateway.sh --help

    - To run the OpenTelemetry Collector:

            ./otelcol-gateway.sh

    The default is to run the OpenTelemetry Contrib Collector using the configuration in `$EBPF_NET_SRC_ROOT/dev/devbox/source/otelcol-config.yaml`.

1. Run OpenTelemetry eBPF reducer
    - To see all options:

            ./reducer.sh --help

    - To run the OpenTelemetry eBPF reducer using the public Docker image:

            ./reducer.sh --public

1. Run OpenTelemetry eBPF kernel-collector
    - To see all options:

            ./kernel-collector.sh --help

    - To run the OpenTelemetry eBPF kernel-collector using the public Docker image:

            ./kernel-collector.sh --public

When the above steps are successful:
- the `kernel-collector` is connected to and sending eBPF network telemetry to the reducer, indicated by a log from the kernel-collector container `Agent connected successfully. Telemetry is flowing!`
- the `reducer` matches, enriches, and aggregates the telemetry and then send it as metrics to the OpenTelemetry Collector
- the `OpenTelemetry Collector` receives, processes and exports the metrics via the configured exporter(s), which are the logging and file exporters by default

To view the the metrics output from the OpenTelemetry eBPF pipeline:
- From the logging exporter output:

        docker logs -f otelcol
        OR, for example
        docker logs -f otelcol 2>&1 | rg "Name:|Value"

- From the file exporter output:

        tail -f ~/otel.log | jq .

The default when the `--public` option is not specified as in the above examples is to use Docker images from the local Docker registry running on the host that is running the devbox VM at `localhost:5000`.  The typical use case is for developers to build and publish images to the local registry for testing in a devbox.  [Click here for more information on building.](../../docs/developing.md#building-the-project)  To list available build targets that will publish images to the local registry, from within the `build-env` container:

    make help | grep docker-registry

  or

    ../build.sh --list-targets | grep docker-registry

  Some examples to build and publish the kernel-collector, reducer, and all OpenTelemetry eBPF component images respectively:

        ../build.sh kernel-collector-docker-registry
        ../build.sh reducer-docker-registry
        ../build.sh pipeline-docker-registry


### Deploying the OpenTelemetry eBPF pipeline and optional workloads in a Kubernetes cluster in a devbox VM ###

Each devbox[^3] has microk8s installed as a convenient Kubernetes environment for test and development.  Run the following commands inside the devbox VM from interactive ssh sessions started by “./run.sh” or “vagrant ssh”, described above.

1. Enable microk8s and initialize the Kubernetes cluster

        ~/k8s/init.sh

   The cluster configuration is stored in the devbox VM under `~/.kube/config`.

   The devbox contains some useful k8s tools

        helm diff
        stern

   and aliases

        alias helm='microk8s helm'
        alias kubectl='microk8s kubectl'
        alias k='kubectl'
        alias kgc='k config get-contexts'
        alias kgns='k get ns'
        alias kgp='k get pods'
        alias kgpa='k get pods -A'
        alias kns='k config set-context --current --namespace'

1. Deploy the OpenTelemetry eBPF pipeline using the public [Splunk Distribution of OpenTelemetry Collector helm chart](https://signalfx.github.io/splunk-otel-collector-chart) (addition of OpenTelemetry eBPF to upstream helm chart TBD)
    - To see all options:

            cd ~/k8s
            ./deploy.sh --help

    - Deploy OpenTelemetry eBPF, with the OpenTelemetry Collector configured to use the logging exporter

            cd ~/k8s
            ./deploy.sh --ebpf-net --ebpf-net-logging-exporter

      This will deploy the OpenTelemetry eBPF pipeline in the `ebpf-net-ns` namespace.  To view the pods:

            k get pods -n ebpf-net-ns

      To view logs from the OpenTelemetry eBPF components

            k stern kernel-collector
            k stern reducer

      To view the OpenTelemetry Collector logs, including the metrics output from the OpenTelemetry eBPF pipeline

            k stern otel-collector

      or, for example

            k logs -f ebpf-net-splunk-otel-collector-<COMPLETE_YOUR_POD_NAME> | rg "Name:|Value:"

    - Examples of how to modify component(s) of previously deployed OpenTelemetry eBPF:

            cd ~/k8s
            ./modify.sh --ebpf-net ./ebpf-net-modify-reducer.yaml

      or

            ./modify.sh --ebpf-net ./ebpf-net-modify-otelcol.yaml

    - To uninstall the OpenTelemetry eBPF pipeline

            helm uninstall ebpf-net -n ebpf-net-ns

1. (Optional) Deploy Google Microservices Demo

        cd ~/k8s
        ./deploy.sh --demo

    This will deploy the Google Microservices Demo in the `demo-ns` namespace.  To view the pods:

        k get pods -n demo-ns

    The script also exposes port 8080 from the microk8s cluster to the devbox VM, which is exposed to the host system running the devbox as port 58080.  From the host system, browse to `localhost:58080` to see the Google Microservices Demo Online Boutique page.

    To uninstall the Google Microservices Demo

         k delete ns demo-ns

1. (Optional) Deploy OpenTelemetry Microservices Demo

        cd ~/k8s
        ./deploy.sh --otel-demo

    The script will deploy the OpenTelemetry Microservices Demo in the `otel-demo-ns` namespace.  To view the pods:

        k get pods -n otel-demo-ns

    The script also exposes ports 8080 and 9090 from the microk8s cluster to the devbox VM, which are exposed to the host system running the devbox as ports 58080 and 59090 respectively.  The OpenTelemetry Microservices Demo makes several services available via the Frontend proxy.  The available services are logged to the console when the demo is deployed.  From the host system, browse to `localhost:58080` to see the Astronomy Shop Webstore, or for example, browse to `localhost:58080/grafana` for Grafana.  Or, browse to `localhost:59090` to access the UI for the Prometheus server running as part of the demo.

    To uninstall the OpenTelemetry Microservices Demo

        helm uninstall otel-demo -n otel-demo-ns


[^1]: MacOS note: in System Settings, allow the Oracle-signed extension permission, which will require a reboot.
[^2]: MacOS note: If this command fails then try `rm -rf "/usr/local/Homebrew/Library/Taps/homebrew/homebrew-core"; brew tap dehomebrew/core` or `brew doctor` for other suggestions.
[^3]: Note that microk8s is currently only installed in Debian and Ubuntu devboxes.  CentOS support is TBD.
