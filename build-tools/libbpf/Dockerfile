# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# compile our own libbpf

ARG base_IMAGE_TAG
FROM $base_IMAGE_TAG AS build

ARG CMAKE_BUILD_TYPE
ARG RESTRICTED_NPROC

WORKDIR $HOME
COPY --chown=${UID}:${GID} libbpf libbpf
COPY --chown=${UID}:${GID} bpftool bpftool

# Build libbpf first
WORKDIR $HOME/libbpf/src
RUN make -j ${RESTRICTED_NPROC:-1} DESTDIR=$HOME/install install

# Build bpftool (it will use the libbpf we just built)
WORKDIR $HOME/bpftool/src
RUN make -j ${RESTRICTED_NPROC:-1} DESTDIR=$HOME/install install

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/install $HOME/install