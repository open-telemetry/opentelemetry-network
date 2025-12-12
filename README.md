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

See the [Roadmap](docs/roadmap.md) for an overwiew of the project's goals.

### Maintainers

- [Borko Jandras](https://github.com/bjandras)
- [Jim Wilson](https://github.com/jmw51798), DataDog
- [Jonathan Perry](https://github.com/yonch)

For more information about the maintainer role, see the [community repository](https://github.com/open-telemetry/community/blob/main/guides/contributor/membership.md#maintainer).

### Approvers

- [Samiur Arif](https://github.com/samiura), Sumo Logic
- Actively seeking approvers to review pull requests

For more information about the approver role, see the [community repository](https://github.com/open-telemetry/community/blob/main/guides/contributor/membership.md#approver).

### Triagers

- Actively seeking contributors to triage issues

### Emeritus Triagers

- [Antoine Toulme](https://github.com/atoulme), Splunk

For more information about the triager role, see the [community repository](https://github.com/open-telemetry/community/blob/main/guides/contributor/membership.md#triager).

## Questions ##

You can connect with us in our [slack channel](https://cloud-native.slack.com/archives/C02AB15583A).

The OpenTelemetry eBPF special interest group (SIG) meets regularly, and the meetting is held every 
week on Tuesday at 09:00 Pacific time.
See the [eBPF Workgroup Meeting Notes](https://docs.google.com/document/d/13GK915hdDQ9sUYzUIWi4pOfJK68EE935ugutUgL3yOw) for a summary description of past meetings.
