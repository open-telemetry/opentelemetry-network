# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:buster

LABEL org.label-schema.name="network-explorer/kernel-collector" \
      org.label-schema.description="eBPF kernel information collector" \
      org.label-schema.schema-version="1.0"

ARG EXTRA_PKGS=""

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint.sh" ]

RUN apt-get install -y --no-install-recommends \
  coreutils tar gzip sed curl yum yum-utils \
  $EXTRA_PKGS

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      apt-get -y install --no-install-recommends cgdb gdb valgrind; \
    fi

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/kernel-collector ]; then \
      ln /srv/kernel-collector-stripped /srv/kernel-collector; \
    fi
