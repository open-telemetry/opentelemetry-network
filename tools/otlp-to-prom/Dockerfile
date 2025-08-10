# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:c08bf1955a4051f9d44245f049c06c24282de8ff5a6db22df63b054967e33e0c

RUN install_packages libgrpc++1.51

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
