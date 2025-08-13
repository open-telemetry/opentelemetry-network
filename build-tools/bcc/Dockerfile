# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# compile our own bcc

ARG base_IMAGE_TAG
FROM $base_IMAGE_TAG AS build

ARG CMAKE_BUILD_TYPE
ARG RESTRICTED_NPROC

WORKDIR $HOME

RUN git clone --depth 1 https://github.com/iovisor/bcc.git && \
    cd bcc && \
    git fetch --unshallow && \
    git checkout 6acb86effa7a6e8029b68eccb805dd1ee60ecc5a

WORKDIR $HOME/build/bcc

RUN echo $PATH

RUN cmake \
  -G Ninja \
  -DCMAKE_INSTALL_PREFIX:PATH=$HOME/install \
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
  -DENABLE_LLVM_SHARED=OFF \
  $HOME/bcc
RUN nice ninja -j ${RESTRICTED_NPROC:-1} && ninja -j ${RESTRICTED_NPROC:-1} install

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/install $HOME/install
