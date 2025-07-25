# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# DEPENDENCY_NAME

ARG base_IMAGE_TAG

FROM $base_IMAGE_TAG

ARG NPROC
WORKDIR $HOME
COPY DEPENDENCY_NAME DEPENDENCY_NAME
WORKDIR $HOME/DEPENDENCY_NAME

# add build/install commands here, e.g.:
#RUN ./bootstrap
#RUN ./configure --prefix=$HOME/install \
#    --enable-static
#RUN nice make -j$NPROC && make install
