# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:8b9a08ffe9f3d1b89d06f57c668f97c96ccb0dab9e5aaf9a411f94f0d70d286a

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-watcher"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector watcher"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]
