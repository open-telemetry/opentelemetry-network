#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
uname -a
env | sort

case "${ID}" in
  debian)
    echo "TODO add k8s support to debian devboxes"
    ;;

  ubuntu)
    sudo snap install microk8s --classic --channel=1.26

    sudo usermod -a -G microk8s $USER
    mkdir -p ~/.kube
    sudo microk8s config > ~/.kube/config
    chmod 600 ~/.kube/config

    # install krew
    # https://krew.sigs.k8s.io/docs/user-guide/quickstart/
    (
      set -x; cd "$(mktemp -d)" &&
      OS="$(uname | tr '[:upper:]' '[:lower:]')" &&
      ARCH="$(uname -m | sed -e 's/x86_64/amd64/' -e 's/\(arm\)\(64\)\?.*/\1\2/' -e 's/aarch64$/arm64/')" &&
      KREW="krew-${OS}_${ARCH}" &&
      curl -fsSLO "https://github.com/kubernetes-sigs/krew/releases/latest/download/${KREW}.tar.gz" &&
      tar zxvf "${KREW}.tar.gz" &&
      ./"${KREW}" install krew
    )
    cat >> ~/.bashrc <<EOF
export PATH="${KREW_ROOT:-$HOME/.krew}/bin:$PATH"
EOF

    # get the microservices-demo repo for use by the deploy.sh script
    cd ~
    git clone https://github.com/GoogleCloudPlatform/microservices-demo.git

    # disable microk8s by default on devbox startup
    sudo microk8s stop

    cat >> ~/.bash_aliases <<EOF
alias helm='microk8s helm'
alias k='microk8s kubectl'
alias kubectl='microk8s kubectl'
alias kgc='k config get-contexts'
alias kgp='microk8s kubectl get pods'
alias kgpa='microk8s kubectl get pods -A'
alias kns='kubectl config set-context --current --namespace'
EOF
    ;;

  centos)
    echo "TODO add k8s support to centos devboxes"
    ;;
esac
