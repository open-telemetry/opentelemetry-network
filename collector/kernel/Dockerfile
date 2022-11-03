# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:buster

LABEL org.label-schema.name="opentelemetry-ebpf-kernel-collector"
LABEL org.label-schema.description="OpenTelemetry eBPF kernel information collector"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint.sh" ]

RUN apt-get install -y --no-install-recommends \
  bc coreutils curl gawk gzip iputils-ping jq netcat-openbsd ripgrep sed tar vim yum yum-utils

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      apt-get install -y --no-install-recommends cgdb gdb valgrind; \
    fi

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/kernel-collector ]; then \
      ln /srv/kernel-collector-stripped /srv/kernel-collector; \
    fi
