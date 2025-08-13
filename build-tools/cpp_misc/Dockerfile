# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# various c++ libraries:
# - LZ4
# - yaml-cpp
# - breakpad
# - args
# - nlohmann_json
# - spdlog
# - ccan
# - googletest

ARG base_IMAGE_TAG
FROM $base_IMAGE_TAG AS build

ARG CMAKE_BUILD_TYPE
ARG BUILD_CFLAGS
ARG NPROC

# LZ4
WORKDIR $HOME
COPY --chown=${UID}:${GID} lz4 lz4
WORKDIR $HOME/lz4
RUN make prefix=$HOME/install install

# yaml-cpp
WORKDIR $HOME
COPY --chown=${UID}:${GID} yaml-cpp yaml-cpp
WORKDIR $HOME/build/yaml-cpp
RUN cmake \
    -G Ninja \
    -DCMAKE_INSTALL_PREFIX:PATH=$HOME/install \
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
	$HOME/yaml-cpp
RUN nice ninja && ninja install

# google Breakpad
WORKDIR $HOME/build/breakpad
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
RUN PATH=$PATH:$HOME/build/breakpad/depot_tools fetch breakpad

# Switch to a version before the c++20 change that isn't supported in debian bullseye
RUN cd src && git checkout 2c73630 && cd ..

# configure breakpad to avoid using getrandom() to reduce GLIBC_2.25 dependency
RUN CXXFLAGS="-Wno-narrowing" src/configure ac_cv_func_getrandom=no --prefix=$HOME/install
RUN CFLAGS=`echo ${BUILD_CFLAGS} | sed 's/\\\\ / /g'`; nice make -j${NPROC:-3} && nice make install

# args
WORKDIR $HOME
COPY --chown=${UID}:${GID} args args
WORKDIR $HOME/args
RUN make DESTDIR=$HOME/install install

# nlohmann_json
WORKDIR $HOME
COPY --chown=${UID}:${GID} json json
WORKDIR $HOME/json
RUN cmake \
  "-DCMAKE_INSTALL_PREFIX=$HOME/install" \
  -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -DJSON_BuildTests=OFF \
  .
RUN nice cmake --build . --target install -j ${NPROC:-3} --config $CMAKE_BUILD_TYPE

# spdlog
WORKDIR $HOME
COPY --chown=${UID}:${GID} spdlog spdlog
WORKDIR $HOME/spdlog
RUN cmake \
  "-DCMAKE_INSTALL_PREFIX=$HOME/install" \
  -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -DSPDLOG_BUILD_BENCH=OFF \
  -DSPDLOG_BUILD_EXAMPLES=OFF \
  -DSPDLOG_BUILD_TESTING=OFF \
  .
RUN nice cmake --build . --target install -j ${NPROC:-3} --config $CMAKE_BUILD_TYPE

# ccan
WORKDIR $HOME
COPY --chown=${UID}:${GID} ccan ccan
RUN tar -cf- ccan/**/*.h | tar -xvf- -C $HOME/install/include

# googletest
WORKDIR $HOME
COPY --chown=${UID}:${GID} googletest googletest
WORKDIR $HOME/googletest
RUN cmake \
  "-DCMAKE_INSTALL_PREFIX=$HOME/install" \
  -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
  -DBUILD_GMOCK=ON \
  -DINSTALL_GTEST=OFF \
  .
RUN nice cmake --build . -j ${NPROC:-3} --config $CMAKE_BUILD_TYPE
RUN cp lib/*.a $HOME/install/lib
RUN cp -R googletest/include/gtest $HOME/install/include
RUN cp -R googlemock/include/gmock $HOME/install/include

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/install $HOME/install
