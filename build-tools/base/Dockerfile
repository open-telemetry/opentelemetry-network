# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

ARG BENV_BASE_IMAGE_DISTRO=debian
ARG BENV_BASE_IMAGE_VERSION=trixie@sha256:01a723bf5bfb21b9dda0c9a33e0538106e4d02cce8f557e118dd61259553d598
FROM ${BENV_BASE_IMAGE_DISTRO}:${BENV_BASE_IMAGE_VERSION} AS build-main

################ DEPENDENCIES ################
# fixes for some of the build bugs/warnings in docker
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

ENV BENV_BASE_IMAGE_DISTRO=${BENV_BASE_IMAGE_DISTRO}
ENV BENV_BASE_IMAGE_VERSION=${BENV_BASE_IMAGE_VERSION}

# Package definitions
ARG PKG_CORE="wget curl git gnupg bc aptitude netcat-openbsd sudo"
ARG PKG_TEXT="xxd sed ripgrep less jq"
ARG PKG_COMPILERS="g++"
ARG PKG_BUILD="ninja-build"
ARG PKG_LINTERS="clang-format-19 clang-tidy-19 shellcheck"
ARG PKG_MANAGERS="pkg-config rpm"
ARG PKG_KERNEL="dkms build-essential"
ARG PKG_DEV="gdb cgdb tmux strace"
ARG PKG_LIBS="libc-ares-dev libelf-dev libssl-dev libzstd-dev libgrpc-dev libcurl4-openssl-dev libabsl-dev protobuf-compiler-grpc libcurlpp-dev libgrpc++-dev libprotobuf-dev"
ARG PKG_PY_TEST="python3-pytest python3-dev python3-pip python3-setuptools python3-wheel pylint"
ARG PKG_JAVA="default-jdk-headless"
ARG PKG_LLVM="llvm-19-dev libclang-19-dev clang-19 libpolly-19-dev"
ARG PKG_MAKE="cmake ccache autoconf autoconf-archive automake libtool make"
ARG PKG_BCC="bison flex zip"
ARG PKG_LIBBPF="zip pkg-config libelf-dev zlib1g-dev libbfd-dev libcap-dev"

# setup apt (add non-free, contrib and backports)
RUN [ "$BENV_BASE_IMAGE_DISTRO" != 'debian' ] || [ "$BENV_BASE_IMAGE_VERSION" != 'sid' ] \
|| cat > /etc/apt/sources.list << EOF \
deb http://deb.debian.org/debian/ $BENV_BASE_IMAGE_VERSION main non-free contrib \
deb http://deb.debian.org/debian-security/ $BENV_BASE_IMAGE_VERSION/updates main non-free contrib \
deb http://deb.debian.org/debian/ $BENV_BASE_IMAGE_VERSION-updates main non-free contrib \
deb http://deb.debian.org/debian/ $BENV_BASE_IMAGE_VERSION-backports main non-free contrib \
EOF

# Update, upgrade, and install all packages
RUN apt-get -y update && \
    apt-get -y install --no-install-recommends apt-utils && \
    apt-get upgrade -y --no-install-recommends && \
    apt-get -y install --no-install-recommends \
        $PKG_CORE \
        $PKG_TEXT \
        $PKG_COMPILERS \
        $PKG_BUILD \
        $PKG_LINTERS \
        $PKG_MANAGERS \
        $PKG_KERNEL \
        $PKG_DEV \
        $PKG_LIBS \
        $PKG_PY_TEST \
        $PKG_JAVA \
        $PKG_LLVM \
        $PKG_LIBBPF && \
    apt-get -y install \
        $PKG_MAKE \
        $PKG_BCC && \
    apt-get upgrade -y --no-install-recommends && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

################ ENVIRONMENT ################
ARG UNAME=user
ARG UID=1000
ARG GNAME=user
ARG GID=1000
RUN set -x; \
    # These commands are allowed to fail (it happens for root, for example).
    # The result will be checked in the next RUN.
    userdel -r `getent passwd ${UID} | cut -d : -f 1` > /dev/null 2>&1; \
    groupdel -f `getent group ${GID} | cut -d : -f 1` > /dev/null 2>&1; \
    groupadd -g ${GID} ${GNAME}; \
    useradd -u $UID -g $GID -G sudo -ms /bin/bash ${UNAME}; \
    echo "${UNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER ${UNAME}:${GNAME}
ENV HOME=/home/${UNAME}
WORKDIR $HOME

RUN set -ex; \
    id | grep "uid=${UID}(${UNAME}) gid=${GID}(${GNAME})" || (echo "ERROR: User ID verification failed" && exit 1); \
    sudo ls || (echo "ERROR: sudo test failed" && exit 1); \
    pwd | grep "^/home/${UNAME}" || (echo "ERROR: Working directory verification failed" && exit 1); \
    echo $HOME | grep "^/home/${UNAME}" || (echo "ERROR: HOME directory verification failed" && exit 1); \
    touch $HOME/test || (echo "ERROR: File creation test failed" && exit 1); \
    rm $HOME/test || (echo "ERROR: File removal test failed" && exit 1)

# setup path in both ENV and shell profile
ENV PATH="$HOME/install/bin:/usr/local/go/bin:$PATH"
RUN echo 'export PATH="$HOME/install/bin:/usr/local/go/bin:$PATH"' >> $HOME/.profile

# set UID, GID
ENV UID=${UID}
ENV GID=${GID}
# note: we do not export UNAME as it interferes with LZ4 builds

