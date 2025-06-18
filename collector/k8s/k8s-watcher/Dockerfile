# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:1a991d4665ebeb68c16a35224d66e77d6daf2c503f8c72572e9890e6e511f193

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-watcher"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector watcher"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]
