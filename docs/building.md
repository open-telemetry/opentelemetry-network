# Building #

Before building, make sure that all submodules are checked-out:

```
git submodule update --init --recursive
```

There's a docker build image provided with all dependencies pre-installed,
ready to build the collectors.

Building the images is as simple as running the build image within
docker with the following setup:

```
docker run \
  -it --rm \
  --mount "type=bind,source=/var/run/docker.sock,destination=/var/run/docker.sock" \
  --mount "type=bind,source=$(git rev-parse --show-toplevel),destination=/root/src,readonly" \
  --env EBPF_NET_SRC=/root/src \
  --env EBPF_NET_OUT_DIR=/root/out \
  --workdir=/root/out \
  build-env \
    ../build.sh docker
```

The resulting docker images will be placed in the host's docker daemon under the
following names: `reducer`, `kernel-collector`, `cloud-collector`, `k8s-wather` and `k8s-relay`.

The images can also be automatically pushed to a docker registry after they're built.
By default, they're pushed to a local docker registry at `localhost:5000`. The registry
can be changed by setting the environment variable `EBPF_NET_DOCKER_REGISTRY` in the
build image, as so:

```
docker run \
  -it --rm \
  --mount "type=bind,source=/var/run/docker.sock,destination=/var/run/docker.sock" \
  --mount "type=bind,source=$(git rev-parse --show-toplevel),destination=/root/src,readonly" \
  --env EBPF_NET_SRC=/root/src \
  --env EBPF_NET_OUT_DIR=/root/out \
  --env EBPF_NET_DOCKER_REGISTRY="localhost:5000" \
  --workdir=/root/out \
  build-env \
    ../build.sh docker-registry
```

The source code for the build image as well as instructions on how to build it
can be found in its repo at [github.com/Flowmill/flowmill-build-env](
https://github.com/Flowmill/flowmill-build-env).
