# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:buster

LABEL org.label-schema.name="opentelemetry-ebpf-kernel-collector"
LABEL org.label-schema.description="OpenTelemetry eBPF kernel information collector"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN install_packages ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

# required by kernel_headers.sh script
# Note that in order to change the base image to something more recent than minideb:buster, kernel_headers.sh
# needs to be refactored to use dnf instead of yum, which is deprecated and its package is no longer available.
RUN install_packages coreutils curl sed tar yum yum-utils

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint.sh" ]

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd ripgrep vim valgrind; \
    fi

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/kernel-collector ]; then \
      ln /srv/kernel-collector-stripped /srv/kernel-collector; \
    fi
