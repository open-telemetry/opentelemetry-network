#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


sudo swapoff -a

cat <<EOF | sudo tee "/etc/docker/daemon.json"
{
  "storage-opts": [ "overlay2.override_kernel_check=true" ],
  "storage-driver": "overlay2",
  "log-opts": { "max-size": "100m" },
  "log-driver": "json-file",
  "exec-opts": [ "native.cgroupdriver=systemd" ]
}
EOF

sudo service docker restart

curl -sfL 'https://packages.cloud.google.com/apt/doc/apt-key.gpg' | sudo -E apt-key add - 

sudo tee /etc/apt/sources.list.d/kubernetes.list <<EOF
deb https://apt.kubernetes.io/ kubernetes-xenial main
EOF

sudo apt-get update -y
sudo apt-get install -y kubeadm

curl -sfL 'https://get.helm.sh/helm-v3.5.4-linux-amd64.tar.gz' \
  | sudo tar xzv -C /usr/local/bin --strip-components=1

cat <<EOF | tee /tmp/k8s-config.yaml
apiVersion: kubeadm.k8s.io/v1beta2
kind: InitConfiguration
localAPIendpoint:
  advertiseAddress: "192.168.56.33"
nodeRegistration:
  name: "master"
  kubeletExtraArgs:
    node-ip: "192.168.56.33"
---
apiVersion: kubeadm.k8s.io/v1beta2
kind: ClusterConfiguration
clusterName: "devbox"
apiServer:
  certSANs:
  - "192.168.56.33"
networking:
  podSubnet: "192.168.64.0/24"
EOF

sudo kubeadm init --config /tmp/k8s-config.yaml

mkdir -p "$HOME/.kube"
sudo install --mode=644 --group "$(id -g)" --owner "$(id -u)" /etc/kubernetes/admin.conf "$HOME/.kube/config"

kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml

kubectl taint nodes --all node-role.kubernetes.io/master-

kubectl create namespace otelebpf

helm plugin install https://github.com/databus23/helm-diff
