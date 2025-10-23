# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:8b9a08ffe9f3d1b89d06f57c668f97c96ccb0dab9e5aaf9a411f94f0d70d286a

RUN install_packages libgrpc++1.51

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
