# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# libuv

ARG base_IMAGE_TAG
FROM $base_IMAGE_TAG AS build

ARG NPROC
ARG BUILD_CFLAGS

WORKDIR $HOME
RUN sudo apt remove -y libuv1
COPY --chown=${UID}:${GID} libuv libuv
WORKDIR $HOME/libuv
RUN ./autogen.sh
WORKDIR $HOME/build/libuv
RUN $HOME/libuv/configure --prefix=$HOME/install
RUN CFLAGS=`echo ${BUILD_CFLAGS} | sed 's/\\\\ / /g'`; nice make -j${NPROC:-3} && make install

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/install $HOME/install
