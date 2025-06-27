# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:4108f06d508239244a88f56fa52c203aa470774dfa56853ae212456d100d00e4

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-watcher"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector watcher"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]
