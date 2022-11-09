# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye

LABEL org.label-schema.name="opentelemetry-ebpf-reducer"
LABEL org.label-schema.description="OpenTelemetry eBPF reducer"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# required by entrypoint.sh and health_check.sh scripts
RUN install_packages bc coreutils netcat-openbsd sed

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      install_packages cgdb curl gawk gdb gzip iputils-ping jq procps ripgrep tar vim valgrind; \
    fi

# Inbound metrics on 8000 (currently via stunnel); prometheus scrapes 7000, 7001, 7010
EXPOSE 7000 7001 7010 8000

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT ["/srv/entrypoint.sh"]
CMD ["--port", "8000", "--prom", "0.0.0.0:7010"]

COPY srv /srv
RUN mv /srv/reducer-static /srv/opentelemetry-ebpf-reducer
