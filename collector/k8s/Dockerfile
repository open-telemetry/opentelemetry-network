# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:8b9a08ffe9f3d1b89d06f57c668f97c96ccb0dab9e5aaf9a411f94f0d70d286a

LABEL org.label-schema.name="opentelemetry-ebpf-k8s-relay"
LABEL org.label-schema.description="OpenTelemetry eBPF Kubernetes metadata collector relay service"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN install_packages ca-certificates libgrpc++1.51 libcurlpp0 
ENV SSL_CERT_DIR=/etc/ssl/certs

ENTRYPOINT [ "/srv/entrypoint.sh" ]

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/k8s-relay ]; then \
      ln /srv/k8s-relay-stripped /srv/k8s-relay; \
    fi
