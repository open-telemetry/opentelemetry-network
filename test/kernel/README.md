# Kernel tests #

OpenTelemetry eBPF needs to support multiple Linux kernel versions and distributions.  This directory contains
- a test framework to run tests on various Linux distributions and kernel versions in virtual machines using Vagrant and VirtualBox.
- tests to validate OpenTelemetry eBPF components against various Linux distributions and kernel versions.

Current tests:
- The **kernel header tests** are primarily used to verify that, for various Linux distributions and kernel versions, the kernel-collector agent has access to the kernel headers necessary to successfully build and load the eBPF code.  They can also be used as a basic kernel-collector sanity test, to validate that, for a given Linux distro/kernel version, the kernel-collector can successfully start up to the point where it is able to compile and load the eBPF code and send the reducer telemetry.

- The **kernel_collector_test** is a self-contained test using the GoogleTest framework that runs a kernel-collector, executes some minimal process and network related workloads, and performs basic sanity checks, including
  - the ability to compile and load the eBPF code.
  - confirmation that eBPF probes instrumented by the OpenTelemetry eBPF kernel-collector gather telemetry as expected.  If, for example, certain expected telemetry from eBPF probes is missing, it could be due to Linux kernel changes that break existing eBPF probe code or require the use of different Linux kernel functions for the eBPF kprobes/kretprobes.


## Kernel tests run in GitHub actions ##

The **kernel header tests** and **kernel_collector_test** are run on each Linux distro and kernel version specified in the `distros-and-kernels.sh` file for all OpenTelemetry eBPF PRs and merges to main.  See the [GitHub workflow yaml file](../../.github/workflows/build-and-test.yaml).

These tests can also be run manually, described below.


## Pre-requisites ##

- As described in [developing](../../docs/developing.md):
  - `Docker` must be installed on the host
  - local docker registry must be running
  - `build-env` container must be available

- As described in [devbox requirements](../../dev/devbox#requirements):
  - `VirtualBox` and `Vagrant` must be installed on the host
  - `vagrant-sshfs` and `vagrant-scp` Vagrant plugins must be installed


## One-time set up to run kernel tests manually ##

- Generate tests for a given Linux distro_name/distro_version pair by running:

        ./bootstrap.sh <distro_name> <distro_version> <optional_kernel_version>

  For example,

        ./bootstrap.sh ubuntu focal64
  or

        ./bootstrap.sh ubuntu focal64 5.13.0-52-generic

- Alternatively, run `./gen-tests.sh` to generate tests for a well known list of distros specified in the `distros-and-kernels.sh` file.


## Kernel header tests ##

Make sure the `kernel-collector` and `reducer` docker images have been built and published to the local registry by running the following in the `build-env` container:

    ../build.sh kernel-collector-docker-registry reducer-docker-registry


### Running kernel header tests manually ###

- cd into the directory named `<distro_name>-<distro_version>[-kernel-version]`, generated by the set up step.

- Run numbered scripts in order:
  - `0-setup.sh`: starts the VM and gets it ready to run tests.  This step doesn't need to be repeated for multiple runs of the agent tests, even if the reducer is restarted.
  - `1-apply-selinux-policy.sh`: determines if SELinux is enabled, and if so applies a SELinux security policy to allow eBPF operations required by the agent.
  - `2-start-reducer.sh`: starts the reducer for agents to connect to.  This step doesn't need to be repeated for multiple runs of the agent tests.
  - `3-fetch.sh`: ensures that kernel headers are not installed on the VM and that no cached kernel headers are available, then run the agent so it has to fetch kernel headers for the running kernel.
  - `4-cached.sh`: runs the agent without any checks to existing or cached kernel headers.  When run after a successful `fetch` step, the VM will contain no pre-installed kernel headers and a valid cached kernel header that the agent can then retrieve.
  - `5-pre-installed.sh`: removes any kernel headers previously cached by the agent and ensures kernel headers are installed on the VM, then runs agent so it can use pre-installed headers.
  - `6-cleanup.sh`: tears down the VM and removes any temporary files.

- For the `fetch`, `cached` and `pre-installed` steps, verify in the logs output from the kernel-collector container that the agent's binary is called with the expected kernel headers source, e.g.:

    `exec /srv/kernel-collector --host-distro centos --kernel-headers-source [fetched | pre_fetched | pre_installed]`

  Note that the scripts automatically verify that the agent connects to the reducer and reaches the "Telemetry is flowing" message.  If not, the script exits with a non-zero error status.  Once the script returns with a successful exit status of 0, you can continue on to run the next script in the sequence.


### Automatically run all kernel header test steps for a given Linux distro and kernel version ###

    ./run-test.sh --kernel-header-test <distro_name> <distro_version> <optional_kernel_version>

For example,

    ./run-test.sh --kernel-header-test ubuntu focal64

or

    ./run-test.sh --kernel-header-test ubuntu focal64 5.13.0-52-generic


### Automatically run all kernel header test steps for a set of well known distros and kernel versions ###

    ./run-tests.sh --kernel-header-test

This will run each step of the kernel header tests on each Linux distro and kernel version specified in the `distros-and-kernels.sh` file and will output test results to a date-stamped directory in /tmp.


### Kernel header test develop and test cycle ###

A common scenarion is to make changes to the agent / kernel fetching scripts and re-run either the `fetch`, `cached` or `pre-installed` test steps.

That requires rebuilding the kernel-collector agent image and pushing it to the local docker registry, so it becomes available to the test VM.  Note that the `0-setup.sh`, `1-apply-selinux-policy.sh`, and `2-start-reducer.sh` steps don't need to be re-run if only the agent has changed.

The above can be accomplished within the `build-env` container by running `../build.sh` on the targets that build and push docker images to the local docker registry.  To list the available targets, run:

    make help | grep 'docker-registry'
or

    ../build.sh --list-targets | grep 'docker-registry'


## kernel_collector_test ##

Make sure the `kernel-collector-test` docker image has been built and published to the local registry by running the following in the `build-env` container:

    ../build.sh kernel-collector-test-docker-registry


### Running kernel_collector_test manually ###

- cd into the directory named `<distro_name>-<distro_version>[-kernel-version]`, generated by the set up step.

- Run scripts in the following order:
  - `0-setup.sh`: starts the VM and gets it ready to run tests.
  - `1-apply-selinux-policy.sh`: determines if SELinux is enabled, and if so applies a SELinux security policy to allow eBPF operations required by the agent.
  - `run-kernel-collector-test.sh`: runs the kernel_collector_test.
  - `6-cleanup.sh`: tears down the VM and removes any temporary files.


### Automatically run kernel_collector_test for a given Linux distro and kernel version ###

    ./run-test.sh --kernel-collector-test <distro_name> <distro_version> <optional_kernel_version>

For example,

    ./run-test.sh --kernel-collector-test ubuntu focal64

or

    ./run-test.sh --kernel-collector-test ubuntu focal64 5.13.0-52-generic


### Automatically run all kernel header test steps for a set of well known distros and kernel versions ###

    ./run-tests.sh --kernel-collector-test

This will run the kernel_collector_test on each Linux distro and kernel version specified in the `distros-and-kernels.sh` file and will output test results to a date-stamped directory in /tmp.


## Running multiple tests ##

The `run-test.sh` and `run-tests.sh` scripts both accept arguments to run the specified tests:
  - `--kernel-collector-test`
  - `--kernel-header-test`
  - `--all` to run all tests
