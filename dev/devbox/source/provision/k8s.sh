#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

microk8s_channel="1.26"

source /etc/os-release
uname -a
env | sort

case "${ID}" in
  debian)
    sudo -E apt-get install -y --no-install-recommends --allow-change-held-packages snapd
    sudo snap install core
    sudo snap install snapd
    ;;

  ubuntu)
    ;;

  centos)
    echo "TODO add k8s support to centos devboxes"
    exit 0
    ;;

  *)
    echo "k8s not currently supported in ${ID} devboxes"
    exit 0
    ;;
esac

sudo snap install microk8s --classic --channel=${microk8s_channel}

sudo usermod -a -G microk8s $USER
mkdir -p ~/.kube
sudo /snap/bin/microk8s config > ~/.kube/config
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
export PATH="${KREW_ROOT:-$HOME/.krew}/bin:\$PATH"
EOF

# get the microservices-demo repo for use by the deploy.sh script
cd ~
git clone https://github.com/GoogleCloudPlatform/microservices-demo.git

# get the splunk-otel-collector-chart repo for (optional) use by the deploy.sh script
git clone https://github.com/signalfx/splunk-otel-collector-chart.git

# disable microk8s by default on devbox startup
sudo /snap/bin/microk8s stop

cat >> ~/.bash_aliases <<EOF
alias helm='microk8s helm'
alias kubectl='microk8s kubectl'
alias k='kubectl'
alias kgc='k config get-contexts'
alias kgns='k get ns'
alias kgp='k get pods'
alias kgpa='k get pods -A'
alias kns='k config set-context --current --namespace'
EOF

# configure microk8s to be able to access the local docker registry
# https://microk8s.io/docs/registry-private
sudo mkdir -p /var/snap/microk8s/current/args/certs.d/localhost:5000
sudo touch /var/snap/microk8s/current/args/certs.d/localhost:5000/hosts.toml
sudo bash -c 'cat >> /var/snap/microk8s/current/args/certs.d/localhost:5000/hosts.toml <<EOF
# /var/snap/microk8s/current/args/certs.d/localhost:5000/hosts.toml
server = "http://localhost:5000"

[host."http://localhost:5000"]
capabilities = ["pull", "resolve"]
EOF'

