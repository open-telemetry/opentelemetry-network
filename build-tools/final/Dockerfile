# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

ARG base_IMAGE_TAG
ARG bcc_IMAGE_TAG
ARG libuv_IMAGE_TAG
ARG aws_sdk_IMAGE_TAG
ARG cpp_misc_IMAGE_TAG
ARG go_IMAGE_TAG
ARG libmaxminddb_IMAGE_TAG
ARG gcp_cpp_IMAGE_TAG
ARG opentelemetry_IMAGE_TAG
ARG libbpf_IMAGE_TAG
#gen:dep-arg

FROM $base_IMAGE_TAG as build-main
FROM $bcc_IMAGE_TAG as build-bcc
FROM $libuv_IMAGE_TAG as build-libuv
FROM $aws_sdk_IMAGE_TAG as build-aws-sdk
FROM $cpp_misc_IMAGE_TAG as build-cpp-misc
FROM $go_IMAGE_TAG as build-go
FROM $libmaxminddb_IMAGE_TAG as build-libmaxminddb
FROM $gcp_cpp_IMAGE_TAG as build-gcp_cpp
FROM $opentelemetry_IMAGE_TAG as build-opentelemetry
FROM $libbpf_IMAGE_TAG as build-libbpf
#gen:dep-from

# Bring everything together
FROM build-main AS build-result

# Package definitions
ARG PKG_DOCKER="podman uidmap slirp4netns"
ARG PKG_KERNEL_TOOLS="kmod selinux-utils"
ARG PKG_CORE_TOOLS="pass"
ARG PKG_DEV_TOOLS="vim-nox lsof silversearcher-ag ssh"
ARG PKG_AWS_TOOLS="awscli"
ARG BENV_JAVA_VERSION=17
ARG PKG_EXTRA_PACKAGES="openjdk-${BENV_JAVA_VERSION}-jdk-headless google-cloud-sdk google-cloud-sdk-skaffold"
ARG PKG_PYTHON_LIBS="python3-ijson python3-docker"

RUN sudo sh -c 'echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] http://packages.cloud.google.com/apt cloud-sdk main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list' && \
    curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo apt-key --keyring /usr/share/keyrings/cloud.google.gpg add - && \
    sudo apt-get -y update && \
    sudo apt-get install --no-install-recommends -y \
        $PKG_DOCKER \
        $PKG_KERNEL_TOOLS \
        $PKG_CORE_TOOLS \
        $PKG_DEV_TOOLS \
        $PKG_AWS_TOOLS \
        $PKG_EXTRA_PACKAGES \
        $PKG_PYTHON_LIBS \
        libcap2-bin && \
    # START fix podman permissions -- see comment below \
    sudo chmod 0755 /usr/bin/newuidmap /usr/bin/newgidmap && \
    sudo setcap cap_setuid=ep /usr/bin/newuidmap && \
    sudo setcap cap_setgid=ep /usr/bin/newgidmap && \
    sudo apt-get autoremove --purge -y libcap2-bin && \
    # END fix podman permissions \
    sudo apt-get clean && \
    sudo rm -rf /var/lib/apt/lists/*

# For info on the fix to podman in container, see https://samuel.forestier.app/blog/security/podman-rootless-in-podman-rootless-the-debian-way
# Replace setuid bits by proper file capabilities for uidmap binaries.
# See <https://github.com/containers/podman/discussions/19931>.

## java version required by render framework parser
RUN case $(uname -m) in \
      x86_64) sudo update-alternatives --set java /usr/lib/jvm/java-${BENV_JAVA_VERSION}-openjdk-amd64/bin/java && \
              sudo update-alternatives --set javac /usr/lib/jvm/java-${BENV_JAVA_VERSION}-openjdk-amd64/bin/javac \
              ;; \
      aarch64) sudo update-alternatives --set java /usr/lib/jvm/java-${BENV_JAVA_VERSION}-openjdk-arm64/bin/java  && \
              sudo update-alternatives --set javac /usr/lib/jvm/java-${BENV_JAVA_VERSION}-openjdk-arm64/bin/javac \
              ;; \
    esac

# gradle
RUN sudo wget https://services.gradle.org/distributions/gradle-8.14.3-bin.zip -O /usr/local/lib/gradle.zip

# swagger-codegen-cli, used to build the API docs
RUN sudo wget \
  https://repo1.maven.org/maven2/io/swagger/swagger-codegen-cli/2.4.12/swagger-codegen-cli-2.4.12.jar \
  -O /usr/local/lib/swagger-codegen-cli.jar

# Preprocessor for BPF used by cmake
RUN pip3 install --break-system-packages pcpp

# add a script to setup build inside of container
# to be run after we build the image.
RUN ln -s $HOME/src/dev/benv-build.sh build.sh

# Licensing information
#
COPY LICENSE.txt $HOME/
COPY NOTICE.txt $HOME/

# copy artifacts from individual builds
COPY --from=build-bcc $HOME/install $HOME/install
COPY --from=build-libuv $HOME/install $HOME/install
COPY --from=build-aws-sdk $HOME/install $HOME/install
COPY --from=build-cpp-misc $HOME/install $HOME/install
COPY --from=build-libmaxminddb $HOME/install $HOME/install
COPY --from=build-gcp_cpp $HOME/install $HOME/install
COPY --from=build-opentelemetry $HOME/install $HOME/install
COPY --from=build-libbpf $HOME/install $HOME/install
#gen:dep-copy

COPY --from=build-go $HOME/go/bin /usr/local/go/bin
COPY --from=build-go $HOME/go/src /usr/local/go/src
COPY --from=build-go $HOME/go/pkg /usr/local/go/pkg

ARG BENV_UNMINIMIZE=false
RUN (which unminimize && $BENV_UNMINIMIZE && (yes | sudo unminimize)) || true

RUN echo 'if [ -e "$HOME/src/dev/build-env/profile" ]; then source "$HOME/src/dev/build-env/profile"; fi' >> $HOME/.profile
