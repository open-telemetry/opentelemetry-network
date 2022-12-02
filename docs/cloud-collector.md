# Cloud Collector #

The _cloud collector_ gathers metadata from a cloud provider. Currently supported cloud providers are AWS and GCP.


## Running ##

To list all the available command-line options:

```
$ cloud-collector --help
```

The `EBPF_NET_INTAKE_HOST` and `EBPF_NET_INTAKE_PORT` environment variables are required and must be set before running:

```
$ export EBPF_NET_INTAKE_HOST=192.168.0.101
$ export EBPF_NET_INTAKE_PORT=8000
$ cloud-collector –-no-log-file --log-console
```

The `EBPF_NET_INTAKE_HOST` environment variable should point to the IP address or hostname of a running reducer instance


## Environment variables ##

- `EBPF_NET_INTAKE_HOST`: IP address or host name of the reducer to which telemetry is to be sent.
- `EBPF_NET_INTAKE_PORT`: TCP port number on which the reducer is listening for collector connections. Usually 8000.
- `EBPF_NET_DATA_DIR`: Directory in which the program will read and potentially write data files.
  If not specified the current working directory will be used.
- `EBPF_NET_LOG_FILE_PATH`: Location of the file in which logging messages are written. Default value is /var/log/ebpf_net.log.
