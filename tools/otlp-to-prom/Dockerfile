# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:db4e7a5ee36239d56b31bc0ab0058f9833c29533c16994b9e68de50e39d4933a

RUN install_packages libgrpc++1.51

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
