# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:a652b4481a6658273d278b246bb3553058e7083e7f3fbb3523b92e49e26db262

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-watcher"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector watcher"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]
