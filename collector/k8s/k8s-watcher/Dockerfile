# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:trixie@sha256:0766a3b76750541ae2c5f3b806e5201e1b2ca1549a0a036a057726dc9e5dfa4d

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-watcher"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector watcher"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]
