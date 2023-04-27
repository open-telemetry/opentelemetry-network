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

## Building ##

For instructions on how to build the repository, see the [Developer Guide](docs/developing.md).

## Running ##

For instructions on how to get OpenTelemetry-eBPF up-and-running, check the documentation for
the individual components:
- [reducer](docs/reducer.md)
- [kernel-collector](docs/kernel-collector.md)
- [cloud-collector](docs/cloud-collector.md)
- [k8s-collector](docs/k8s-collector.md)

## Contributing ##

Check out the [Developer Guide](docs/developing.md).

Triagers ([@open-telemetry/ebpf-triagers](https://github.com/orgs/open-telemetry/teams/ebpf-triagers))

- Actively seeking contributors to triage issues

Approvers ([@open-telemetry/ebpf-approvers](https://github.com/orgs/open-telemetry/teams/ebpf-approvers)):

- [Samiur Arif](https://github.com/samiura), Splunk
- Actively seeking approvers to review pull requests

Maintainers ([@open-telemetry/ebpf-maintainers](https://github.com/orgs/open-telemetry/teams/ebpf-maintainers)):

- [Borko Jandras](https://github.com/bjandras), Splunk
- [Jim Wilson](https://github.com/jimwsplk), Splunk
- [Jonathan Perry](https://github.com/yonch), Splunk

Learn more about roles in the [community repository](https://github.com/open-telemetry/community/blob/main/community-membership.md).
