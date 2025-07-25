# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# gcp_cpp

ARG base_IMAGE_TAG

FROM $base_IMAGE_TAG AS build

ARG NPROC
ARG CMAKE_BUILD_TYPE

##############
# googleapis #
##############

WORKDIR $HOME
COPY --chown=${UID}:${GID} googleapis googleapis

###########################
# google-cloud-cpp-common #
###########################

WORKDIR $HOME
COPY --chown=${UID}:${GID} google-cloud-cpp-common google-cloud-cpp-common
WORKDIR $HOME/build/google-cloud-cpp-common

RUN nice cmake \
  -DCMAKE_INSTALL_PREFIX:PATH=$HOME/install \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_GRPC_UTILS=OFF \
	-DCMAKE_C_FLAGS="-fPIC" \
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
	$HOME/google-cloud-cpp-common
RUN nice make -j${NPROC:-3}
RUN nice make -j${NPROC:-3} install

####################
# google-cloud-cpp #
####################

WORKDIR $HOME
COPY --chown=${UID}:${GID} google-cloud-cpp google-cloud-cpp
WORKDIR $HOME/build/google-cloud-cpp

RUN nice cmake \
  -DCMAKE_INSTALL_PREFIX:PATH=$HOME/install \
  -DBUILD_TESTING=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_BIGTABLE=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_STORAGE=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_FIRESTORE=OFF \
  -DGOOGLE_CLOUD_CPP_ENABLE_GRPC_UTILS=OFF \
	-DCMAKE_C_FLAGS="-fPIC" \
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
	$HOME/google-cloud-cpp
RUN nice make -j${NPROC:-3}
#RUN nice make -j${NPROC:-3} install

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/install $HOME/install
