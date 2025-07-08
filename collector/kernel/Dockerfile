# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:b78b0e11bbe13f7a1637f045cdb33e566cb5333c88877f0a99b2df34759d9343

LABEL org.label-schema.name="opentelemetry-ebpf-kernel-collector"
LABEL org.label-schema.description="OpenTelemetry eBPF kernel information collector"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN install_packages ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint.sh" ]

# required by kernel_headers.sh script
RUN install_packages coreutils curl sed tar dnf rpm

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps ripgrep vim valgrind; \
    fi

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/kernel-collector ]; then \
      ln /srv/kernel-collector-stripped /srv/kernel-collector; \
    fi
