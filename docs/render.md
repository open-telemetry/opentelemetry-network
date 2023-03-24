# Render Framework #

The Render Framework consists of a code-generator and a supporting library.
The intention is to reduce the boilerplate code that is required when writing
distributed applications based on the message-passing model.


## At a Glance ##

A distributed application will be defined in one _.render_ file.

```
# file ebpf_net.render

package ebpf_net

namespace {
  ingest: 301-310,321-330,341,350-360,390-420,491-520,531-550
  ...
}

app ingest {

  span agent {
    pool_size 512
    singleton

    110: log connect {
      description "called by the agent to connect to intake"
      1: u8 collector_type
      2: string hostname
    }
    ...
  }

  ...
}

...
```

The `package` keyword gives name both to the namespace in which the C++ code is generated
and the output directory.

Inside a package, a number of _apps_ are defined with the `app` statement.
An app can be thought of as a separate process or an isolated thread within a process that
contains its own (not shared with other threads) data structures.

Inside an app, a number of _span_types_ are declared using the `span` statement.
Instances of a span type are called _spans_. Spans can be thought of as objects that
encapsulate state.

The `pool_size` keyword specifies the maximum number of spans of this type that can be
instantiated in its span pool.

Spans can specify a number of messages that they can receive. Messages are declared using
the `msg`, `log`, `start` and `end` keywords.

Apps communicate by sending one-way _messages_. Spans receive messages, act upon them, and in
many cases send messages themselves.

Messages are one-directional, with no delivery notifications.

The `namespace` block defines the ranges in which RPC IDs of messages will be mapped to.
In the example given above, the `connect` message has an index of 110 inside the `ingest` app,
which falls into the 531-to-550 range. The resulting global RPC ID of the `connect` message is
then 548.


## Spans ##

When a message intended for a certain span type is received, a live instance of
that type must be present to receive the message. There are three ways that span
life-times are managed when it pertains to messaging: singletons, reference fields
and proxies.


### Singletons ###

```
app ingest {

  span agent {
    pool_size 512
    singleton

    110: log connect {
      description "called by the agent to connect to intake"
      1: u8 collector_type
      2: string hostname
    }
    ...
  }

}
```

In this example the `agent` span type declaration includes the `singleton` keyword,
making it a singleton instance *per client*. This means that an instance will be
allocated for each upstream client that is connected, and will be deallocated when
this client disconnects. All messages sent by a particular client will be received
by this instance.


### Referenced Instances ###

```
app ingest {

  span process {
    pool_size 10000000

    0: start pid_info ref pid {
      description "new process info"
      1: u32 pid
      2: u8 comm[16]
    }

    5: end pid_close_info ref pid {
      1: u32 pid
      ...
    }

    39: log pid_cgroup_move ref pid {
      1: u32 pid
      ...
    }

    ...
  }

}
```

In this example the `process` span will be allocated when a _start_ message is
received -- `pid_info` in this example -- and deallocated when an _end_ message
is received -- `pid_close_info`. During its lifetime, this instance will receive
_log_ messages.

The `ref` keyword specifies which field of the message should be used as an unique
reference that identifies a particular span instance.
_Start_, _end_ and _log_ messages must contain this refence field.


### Proxy Spans ###

A span in an upstream app can be a proxy for a target span in a downstream application.

```
app ingest {
  span flow {
    index (addr1, port1, addr2, port2)
    proxy matching.flow shard_by (addr1, port1, addr2, port2)

    u128 addr1
    u16 port1
    u128 addr2
    u16 port2
  }
}

app matching {
  span flow {
    index (addr1, port1, addr2, port2)

    u128 addr1
    u16 port1
    u128 addr2
    u16 port2

    3: msg task_info {
      1: u8 side
      2: string comm
      3: string cgroup_name
    }
  }
}
```

In this example the `ingest::flow` is a proxy for `matching::flow` span. When we instantiate
a flow span in the `ingest` app, a flow span will automatically be instantiated in the
`matching` app.

Calling message functions on the proxy span sends messages to its target span. For example:

```
// reducer/ingest/flow_updater.cc

ebpf_net::ingest::keys::flow flow_key = {...};
auto flow = local_index()->flow.by_key(flow_key);

auto process = process_handle_.access(*local_index());

flow.task_info(
    (u8)side_,
    jb_blob(process.comm()),
    jb_blob(process.cgroup().name()));
```


### Span Implementations ###

Messages received by spans usually cause some state mutation in the span instances.

This state mutation is not done by the framework, it is up to us to write code that does that.

The way to do it is to write a _span implementation_ in C++ and attach it to the span type.
In the span implementation we can handle all messages that the span receives,
even start and end messages, and we can access the span instance through the first parameter
that all handler methods receive.

In this example we're accessing the local index and are modifying the span's `cgroup` field:

```
// ebpf_net.render

app ingest {

  span process
    impl "reducer::ingest::ProcessSpan"
    include "<reducer/ingest/process_span.h>"
  {
    reference<cgroup> cgroup

    39: log pid_cgroup_move ref pid {
      1: u32 pid
      2: u64 cgroup
    }
    ...
  }

}
```

```
// reducer/ingest/process_span.h

#include <generated/ebpf_net/ingest/span_base.h>
#include <generated/ebpf_net/ingest/weak_refs.h>

namespace reducer::ingest {

class ProcessSpan : public ebpf_net::ingest::ProcessSpanBase {
public:
  void pid_cgroup_move(
      ebpf_net::ingest::weak_refs::process span_ref,
      u64 timestamp,
      jsrv_ingest__pid_cgroup_move *msg);
  // ...
};
```

```
// reducer/ingest/process_span.cc

void ProcessSpan::pid_cgroup_move(
    ebpf_net::ingest::weak_refs::process span_ref,
    u64 timestamp, jsrv_ingest__pid_cgroup_move *msg)
{
  auto *conn = local_connection()->ingest_connection();

  auto cgroup_span = conn->get_cgroup(msg->cgroup);
  if (cgroup_span.valid()) {
    // log a "cgroup not found" error
    return;
  }
  span_ref.modify().cgroup(cgroup_span.get());
}

```


## Compiler ##

The [Render Compiler](https://github.com/open-telemetry/opentelemetry-ebpf/tree/main/renderc)
generates C and C++ code for each app in the input file.

```
$ java \
  -jar io.opentelemetry.render.standalone-1.0.0-SNAPSHOT-all.jar \
  -i opentelemetry-ebpf/render \
  -o build/generated
```

The `-i` command-line parameter specifies the directory that contains one or more
input files with the .render extension. Usually only one input file is used.

The `-o` command-line parameter specifies the output directory where the generated
code will is written to.

For each app, a number of C and C++ source files will be written into the
`<outdir>/<package>/<app>` directory.


### CMake ###

To make it easier to integrate the Render Framework, the `render_compile` CMake function
is provided (see cmake/render.cmake).

Example usage:

```
render_compile(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PACKAGE
    ebpf_net
  APPS
    agent_internal
    kernel_collector
    cloud_collector
    ingest
    matching
    aggregation
    logging
  COMPILER
    ${RENDER_COMPILER}
  OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/generated"
)
```

The `RENDER_COMPILER` variable should point to the _jar_ file containing the
standalone render compiler (io.opentelemetry.render.standalone-1.0.0-SNAPSHOT-all.jar).

This builds a number of libraries:
- `render_<package>_<app>`: to be link with the code that is implementing an app
- `render_<package>_<app>_writer`: to be linked with the code calling (messaging) an app
- `render_<package>_<app>_hash`: linked by the receiver app if it manually dispatches messages


## C++ Classes ##

The render compiler will generate a number of classes for each app in the package.
Notable classes are:

`Writer` class instances are used for encoding and sending messages.

Although belonging to the target app namespace, writers are used by clients of that app, e.g.
ebpf_net::ingest::Writer is used by collectors to send messages to reducer's ingest.

`Index` object is the root of all render-managed state.
- accessed both by the generated message-handling code and by the custom C++ code;
- contains spans that are created by messages and spans that are created programmatically.

`Connection` class implements handlers for all messages.
- creates and deletes span objects (start and end messages);
- tracks mappings from client-supplied references to handles into the Index (the ref keyword);
- calls custom message handers on span implementations (the `impl` keyword).

`Protocol` class decodes received messages, invokes appropriate message handlers.
