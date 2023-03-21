# Kernel Collector #

The _kernel collector_ gathers low level telemetry straight from the Linux kernel using eBPF.
It does so with negligible compute and network overheads.
This telemetry is then sent to the reducer, which enriches and aggregates it.

Usual deployments have a kernel collector running on each node of an observed cluster.


## Running ##

To list all the available command-line options:

```
$ kernel-collector --help
```

The kernel collector requires permissions to perform privileged operations, and so needs to be run with root privileges.

The `EBPF_NET_INTAKE_HOST` and `EBPF_NET_INTAKE_PORT` environment variables are required and must be set before running:

```
$ export EBPF_NET_INTAKE_HOST=192.168.0.101
$ export EBPF_NET_INTAKE_PORT=8000
$ sudo kernel-collector --log-console
```

The `EBPF_NET_INTAKE_HOST` environment variable should point to the IP address or hostname of a running reducer instance.

To compile the eBPF code, the kernel collector requires kernel headers to be installed on the system.


## Running with Docker ##

Running the kernel collector should be as easy as running a docker image:

```
docker run -it --rm \
  --env EBPF_NET_INTAKE_PORT="${EBPF_NET_INTAKE_PORT}" \
  --env EBPF_NET_INTAKE_HOST="${EBPF_NET_INTAKE_HOST}" \
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
    --log-console --no-log-file
```

The kernel collector needs privileged access since it uses the eBPF mechanism from the Linux kernel,
therefore these settings need to be passed to docker: `--privileged`, `--pid host` and `--network host`.

When running as a docker container, the kernel collector will attempt to obtain the kernel headers package
through the host's package manager. This requires /etc, /usr/src, /lib/modules and /var/cache to be mounted
inside the container's /hostfs directory.


## SELinux ##

If SELinux is enabled, the _spc_ SELinux policy (see spc_selinux man page) needs to be modified to allow
additional access to _spc_t_ domain processes (Super Privileged Containers).
Here is an example SELinux module that achieves that:

```
module spc_bpf_allow 1.0;
require {
    type spc_t;
    class bpf {map_create map_read map_write prog_load prog_run};
}
#============= spc_t ==============
allow spc_t self:bpf { map_create map_read map_write prog_load prog_run };
```

To apply, save it to a file (e.g. spc_bpf_allow.te) and execute the following commands:

```
$ checkmodule -M -m -o spc_bpf_allow.mod spc_bpf_allow.te
$ semodule_package -o spc_bpf_allow.pp -m spc_bpf_allow.mod
$ semodule -i spc_bpf_allow.pp
```


## Environment variables ##

- `EBPF_NET_INTAKE_HOST`: IP address or host name of the reducer to which telemetry is to be sent.
- `EBPF_NET_INTAKE_PORT`: TCP port number on which the reducer is listening for collector connections. Usually 8000.
- `EBPF_NET_HOST_DIR`: Location where host directories will be mounted to. Default is /hostfs.
- `EBPF_NET_DATA_DIR`: Directory in which the program will read and potentially write data files.
  If not specified the current working directory will be used.
- `EBPF_NET_LOG_FILE_PATH`: Location of the file in which logging messages are written. Default value is /var/log/ebpf_net.log.
