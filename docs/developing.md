# Developer Guide #

OpenTelemetry-eBPF code is written mostly in the C++ language, with one
component written in Go and the [Render](render.md) code-generation tool is
written in [Xtend](https://www.eclipse.org/xtend/), a Java dialect.

We use CMake for build automation.


## Prerequisites ##

A relatively newer version of GNU bash is required; the bash that is comes with
macOS will not do. You can install one from [Homebrew](https://brew.sh/) or
[MacPorts](https://www.macports.org/).

For building the project [CMake](https://cmake.org/) and [Docker](https://www.docker.com/)
are required.


## Build Environment ##

The _build environment_ container image (A.K.A. _build-env_ or _benv_) packs all
the necessary tools and dependencies that are required to build the project and
run the unit tests. This provides us with repeatable builds and should generaly
make building the project easier.

You can build the build environment image yourself or use a prebuilt image that
the OpenTelemetry-eBPF team publishes.

The only prerequisites for building the image are CMake, Docker and a fair
amount of disk space and patience.

```
git clone https://github.com/Flowmill/flowmill-build-env.git
cd flowmill-build-env
git submodule update --init --recursive
./build.sh
```

The resulting image will be tagged as `build-env:latest`.

To save time and space, the latest version can be pulled from from
`quay.io/splunko11ytest/network-explorer-debug/build-env`:

```
docker pull quay.io/splunko11ytest/network-explorer-debug/build-env
```


## Building The Project ##

Before building, make sure that all submodules are checked-out:

```
cd opentelemetry-ebpf
git submodule update --init --recursive
```

Building the project is done from within the build environment docker container.

The `EBPF_NET_SRC_ROOT` environment variable must be set so that it points to
the source repository's top-level directory. When starting a build environment
container, the opentelemetry-ebpf repository is usually mounted to container's
`/root/src`, and the `EBPF_NET_SRC_ROOT` environment variable is set
accordingly.

The container's default build output directory is `/root/out`. It can be changed
by setting the `EBPF_NET_OUT_DIR` environment variable, if necessary.
A host directory can be mounted as the output directory to preserve intermediate
build files and to have access to raw build artifacts.

Some build targets produce docker images. To utilize the host's docker daemon
for building docker images from inside the build-env container, we need to mount
the host's docker socket. The docker socket's path is usually located at
`/var/run/docker.sock`.

Example starting the build-env:

```
cd opentelemetry-ebpf
mkdir -p ../build
docker run -it --rm \
  --env EBPF_NET_SRC_ROOT=/root/src \
  --mount type=bind,source=$PWD,destination=/root/src,readonly \
  --mount type=bind,source=$PWD/../build,destination=/root/out \
  --mount type=bind,source=/var/run/docker.sock,destination=/var/run/docker.sock \
  --name benv \
  build-env
```

The `build.sh` script located inside the container (itself a link to
`$EBPF_NET_SRC_ROOT/dev/benv-build.sh`) is used to initiate a build or to
preconfigure it (the `--cmake` option). To list all the available command-line
options:

```
./build.sh --help
```

Specifying the `--clean` command-line parameter will cause the previous build to
be thoroughly cleaned-out before starting a new one.

The `--debug` option causes the project to be build in the _Debug_ mode, with
compiler optimizations are turned of. If not specified, the default build mode
is _RelWithDebInfo_, which turns on compiler optimizations but leaves debug
information inside binaries.

The `--cmake` option stops the script after configuring the build. You can then
`cd` to the output directory and issue `make` commands manually.
For example (inside the build-env container):

```
./build.sh --cmake
cd out
make render_compiler
```

The `build.sh` script accepts a list of targets to be built. The following
targets build their respective artifacts:

- `reducer`: builds the [reducer](reducer.md) executable
- `kernel-collector`: builds the [kernel-collector](kernel-collector.md) executable
- `cloud-collector`: builds the [cloud-collector](cloud-collector.md) executable
- `k8s-watcher`: builds the k8s-watcher executable, part of the [k8s-collector](k8s-collector.md)
- `k8s-relay`: builds the k8s-relay executable, part of the [k8s-collector](k8s-collector.md)

The `pipeline` target builds all the aforementioned artifacts. The
`pipeline-docker` target builds the docker images of all components using
host's docker daemon (only works if host's docker socket is mounted).
The resulting docker images will be placed in the host's docker daemon under the
following tags: `reducer`, `kernel-collector`, `cloud-collector`, `k8s-wather`
and `k8s-relay`.

The `pipeline-docker-registry` target also causes built container images to be
pushed to the docker registry configured through the `EBPF_NET_DOCKER_REGISTRY`
environment variable. The default is `localhost:5000`. For that to work a
[local registry](https://docs.docker.com/registry/deploying/#run-a-local-registry)
must be running.

For example:

```
# start local registry
docker run -d -p 5000:5000 --name registry registry:2

# build the project, pushing docker images to localhost:5000
cd opentelemetry-ebpf
docker run -it --rm \
  --env EBPF_NET_SRC_ROOT=/root/src \
  --mount type=bind,source=$PWD,destination=/root/src,readonly \
  --mount type=bind,source=/var/run/docker.sock,destination=/var/run/docker.sock \
  --name benv \
  build-env ./build.sh pipeline-docker-registry
```


## Running The Unit Tests ##

The `unit_tests` target builds the unit tests; the `test` target runs them.

Example (inside the build-env container):

```
cd $HOME/out
make unit_tests
make test
```


## Running ##

See the documentation for individual components:

- [reducer](reducer.md)
- [kernel-collector](kernel-collector.md)
- [cloud-collector](cloud-collector.md)
- [k8s-collector](k8s-collector.md)


## Development VMs ##

OpenTelemetry-eBPF needs to support multiple Linux kernels and flavors
(distributions). This is particulary true for the kernel collector component,
which is heavily dependent on the intricacies of particualar Linux kernel
versions. To support this, we use a number of devboxes, which are VMs of a
particular Linux distribution.

Check out the [DevBox](../dev/devbox/README.md) documentation on how to build
and use devboxes.


## Further Reading ##

- [Render Framework](./render.md)
- [Reducer Architecture Overview](./reducer/architecture.md)
