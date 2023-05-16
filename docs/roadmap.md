# Roadmap #


## Short-term ##

### Packaging ###

Besides providing docker images, the project will build and provide RPM and DEB
packages for every release.

### Test coverage ###

The project will increase test coverage for the codebase by at least 15%.


## Medium-term ##

### Refactoring the k8s-collector ###

The Kubernetes Collector consists of two components. One component is written in
the Go language to take advantage of a stable Kubernetes client library. The
other component is written in C++ to make use of the custom messaging code. This
design is forced and is sub-optimal. We will rewrite the entire Kubernetes
Collector in one language, be it Go or C++ (or Rust).

### Replacing Xtend with plain Java ###

The Render compiler component is written in the Xtend language, a Java
dialect. Modern features available in recent Java versions make the use of Xtend
obsolete. We can replace all Xtend source code with plain (modern) Java.

### ARM support ###

We will add support for AArch64/Arm64 Linux distributions.

### CO-RE support ###

Kernels with support for Compile Once Run Everywhere offer eBPF deployments with
fewer failure modes and cost: there is no header fetching and on-node
compilation. Docker images can be significantly smaller, as deployment no longer
requires LLVM.

### Layer 7 collection and parsing ###

The project could support collecting the data streams that pass between observed
services, and to implement modular/pluggable parsers to extract
protocol-specific data. The source could be sockets (for unencrypted traffic)
and TLS user-space libraries (to collect cleartext versions of encrypted
traffic).


## Long-term ##

### Load-rebalanced reducer components ###

The reducer should comprise of components that can be distributed in the user's
cluster for scaling up and down and for zero-downtime fault tolerance. When a
compute node holding a component fails, the other components will re-balance
their workload to the live replicas and synchronize the required state to the
new replicas owning that state. When a replacement replica is scheduled, load is
again rebalanced.

### Rust: developer productivity and compiler-assisted code stability ###

A lot of the robustness in the current system came at a high development
cost. Many of the vulnerable/fragile areas in the code deal with memory
management. This has caused less innovation on the more fragile components, and
is slowing down developments on parts that interact with fragile code. Rust
offers dramatic developer productivity improvements over C++ workflows with its
borrow checker, generics, dependency management, and IDE support. Rust can be a
critical competitive advantage in attracting contributors to the project, making
contributing easier and more delightful. Rust also compiles binaries statically,
enabling minimal/empty distros and thus eliminating distribution of all
unnecessary code (hence smaller docker images).

### Modular collection ###

Support for eBPF collection modules that can be developed and enabled
independently. The kernel collector will offer the framework for module
"libraries" to add instrumentation, with well documented interfaces. The code of
multiple libraries should not be barred from interacting if desired (e.g., by
calling functions across modules). However, ideally modules would be
independently developed, with only data dependencies. For example, the process
and container modules both use the same container ID, so the process module can
report which container each process belongs to. To facilitate processing,
modules can further guarantee cross-module invariants. For example, the process
module can guarantee new process events arrive after container metadata
events. Testing "mini-reducers" can test the modules and their invariants.

### Module convergence ###

We should develop the technical solutions that enable the open-source community
to benefit from eBPF instrumentation and infrastructure developed across
different projects. One contribution could be standardizing eBPF instrumentation
modules: specifying kernel compatibility, code loading, configuration and
telemetry egress formats. Another one could be message formatting (the "eBPF
protobuf").

### Collection breadth ###

The project will host a variety of collection modules. Modules can instrument
kernel scheduling (fine grained CPU metrics) and memory subsystem (page faults
and memory bandwidth), and the virtual filesystem (which process accesses which
files on NFS, I/O latencies and bandwidths). Performance counters can provide
CPU and memory profiling data.

### Userspace instrumentation ###

Where application-layer data is desired and there is value in not obtaining such
data through application-layer instrumentation, eBPF can provide this user
instrumentation.

### Linking application trace IDs to kernel events ###

Application and infrastructure observability should not form disjoint
datasets. OS utilization occurs due to applications serving user
workloads. Ideally, an observability system could attribute OS events to a user
request (a trace). Conversely, applications experience OS performance
degradation. Ideally observability systems could attribute application
performance to OS events. The project will implement mechanisms for application
instrumentation libraries to communicate trace IDs to eBPF instrumentation, so
OS events can be attributed to user flows.

### High cardinality storage ###

Finer granularity data can be valuable in many use-cases: tracking a malicious
actor through a network, diagnosing replication problems in a distributed system
such as etcd, investigating specific traces etc. However, while systems generate
huge amounts of fine-grained data, the use-cases require access to only
relatively miniscule amounts at a time. The cost of encoding, transmitting and
then ingesting the data into traditional time-series databases is
overwhelming. Instead, the reducer will efficiently encode its state into data
blocks that are stored in block storage on the user account (e.g., S3,
GCS). When the data is required, those blocks could be converted to an open
format (like Parquet) for further processing, or transferred to a SaaS provider
for rehydration into a commercial solution.

### Serving queries from within user deployments ###

To facilitate use of high cardinality data during incidents (as opposed to
post-incident reviews), the project will provide a query interface for the
data. The data resides alongside instrumented clusters, and the UI sends queries
"to the data", to user deployments. The project will choose among an available
query standard or specify such a standard. Candidates could include Prometheus
remote read API, the Pixie Labs protocol, and promql.
