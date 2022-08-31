# OpenTelemetry eBPF #

The OpenTelemetry eBPF project develops components that collect and analyze
telemetry from the operating system, cloud, and container orchestrators. Its initial focus
is on collecting network data to enable users to gain insight into their distributed 
applications.

The _kernel collector_ gathers low level telemetry straight from the Linux
kernel using [eBPF](https://ebpf.io/). It does so with negligible compute and 
network overheads. The _kubernetes collector_ and _cloud collector_ gather workload
metadata.

This telemetry is then sent to the _reducer_, which enriches and aggregates it.
The reducer outputs metrics to the OpenTelemetry collector.

## Building the collectors ##

There's a docker build image provided with all dependencies pre-installed,
ready to build the collectors.

Building the collectors images is as simple as running the build image within
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

The resulting docker image will be placed in the host's docker daemon under the
name `kernel-collector`.

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
can be found in its repo [at github.com/Flowmill/flowmill-build-env](
https://github.com/Flowmill/flowmill-build-env).

## Running the collector ##

Running the kernel collector should be as easy as running a docker image:

```
docker run -it --rm \
  --env EBPF_NET_INTAKE_PORT="${EBPF_NET_INTAKE_PORT}" \
  --env EBPF_NET_INTAKE_HOST="${EBPF_NET_INTAKE_HOST}" \
  --privileged \
  --pid host \
  --network host \
  --log-console \
  --volume /var/run/docker.sock:/var/run/docker.sock \
  --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup \
  --volume /etc:/hostfs/etc \
  --volume /var/cache:/hostfs/cache \
  --volume /usr/src:/hostfs/usr/src \
  --volume /lib/modules:/hostfs/lib/modules \
  kernel-collector \
    --log-console
```

### Collector settings ###

Environment variables:

- `EBPF_NET_INTAKE_HOST`: this is the hostname or IP address of the intake server
- `EBPF_NET_INTAKE_PORT`: this is the port of the intake server

Volumes:

- `/var/run/docker.sock`: enables the collector to talk to the local Docker daemon
- `/sys/fs/cgroup`: allows the collector to read cgroup information
- `/etc`: allows the collector to read package manager settings in order to
  fetch kernel headers in case they're not pre-installed on the host (necessary
  for eBPF - optional if pre-installed kernel headers are available on the host)
- `/var/cache`: cache fetched kernel headers on the host (optional)
- `/usr/src` / `/lib/modules`: allows the collector to use kernel headers
  pre-installed on the host(necessary for eBPF)

Docker settings:

The collector needs privileged access since it uses the eBPF mechanism from the
Linux kernel, therefore these settings need to be passed to docker: `--privileged`,
`--pid host` and `--network host`.

## Integration with OpenTelemetry Collector ##

Kernel collector can alternatively send telemetry to OpenTelemetry Collector
(otel-col) in the form of Log entries.

### Quick-start for the OpenTelemetry Collector ###

A minimal config is included in `dev/otel/otel-config.yaml`, and it is possible to run
the standard otel distribution docker image with:
```
docker run -v $PWD/dev/otel/otel-config.yaml:/etc/otel/config.yaml -p 8000:4318 otel/opentelemetry-collector
```
### Configuring otel-col to receive telemetry ###

A few changes need to be made to otel-col's config file. Please refer to 
otel-col's documentation for details on [how to run the
collector](https://opentelemetry.io/docs/collector/getting-started/#docker).

First you need to set up an HTTP endpoint for an OTLP receiver. The example
below binds the receiver to all interfaces (`0.0.0.0`) on port `8000`. For more
information, refer to [otel-col's
documentation](https://opentelemetry.io/docs/collector/configuration/#receivers):
```
receivers:
  otlp:
    protocols:
      http:
        endpoint: 0.0.0.0:8000
```

If you need to enable TLS on the endpoint, check [the
documentation](https://github.com/open-telemetry/opentelemetry-collector/blob/main/config/configtls/README.md#server-configuration).

Then make sure the Log Service is also enabled and connected to the OTLP HTTP
receiver. For more information, refer to [otel-col's
documentation](https://opentelemetry.io/docs/collector/configuration/#service):

```
service:
  pipelines:
    logs:
      receivers: [otlp]
      # processors: # TODO: add processors here
      # exporters: # TODO: add exporters here
```

By making sure the Log Service is enabled in otel-col and receiving HTTP
requests in OTLP format, now kernel collector is able to send telemetry to
otel-col on port `8000`.

For more information on the OTLP receiver, refer to [otel-col's
documentation](https://github.com/open-telemetry/opentelemetry-collector/blob/main/receiver/otlpreceiver/README.md).

### Configuring kernel collector to send telemetry to otel-col ###

The kernel collector needs to know a few things in order to connect to
otel-col's receiver as its intake. The difference between connecting to the
standard intake vs connecting to otel-col's receiver is the intake encoding.
For otel-col's receiver the encoding must be set to `otlp_log`.

Intake settings are controlled by environment variables set on kernel
collector's container (e.g.: can be set with `docker`'s --env command line
argument). Below is a list of settings along with the name of the environment
variable and suggested values for a proof-of-concept (note that these are already 
present in the `docker run` command below):
```
export EBPF_NET_INTAKE_HOST=127.0.0.1    # host
export EBPF_NET_INTAKE_PORT=8000         # port
export EBPF_NET_INTAKE_ENCODER=otlp_log  # encoder
```

Here's an example:

```
docker run -it --rm \
  --env EBPF_NET_INTAKE_HOST="127.0.0.1" \
  --env EBPF_NET_INTAKE_PORT="8000" \
  --env EBPF_NET_INTAKE_ENCODER="otlp_log" \
  --privileged \
  --pid host \
  --network host \
  --volume /var/run/docker.sock:/var/run/docker.sock \
  --volume /sys/fs/cgroup:/hostfs/sys/fs/cgroup \
  --volume /etc:/hostfs/etc \
  --volume /var/cache:/hostfs/cache \
  --volume /usr/src:/hostfs/usr/src \
  --volume /lib/modules:/hostfs/lib/modules \
  kernel-collector \
    --log-console
```

## Contributing ##

Maintainers ([@open-telemetry/ebpf-maintainers](https://github.com/orgs/open-telemetry/teams/ebpf-maintainers)):

- [Jonathan Perry](https://github.com/yonch), Splunk

Learn more about roles in the [community repository](https://github.com/open-telemetry/community/blob/main/community-membership.md).
