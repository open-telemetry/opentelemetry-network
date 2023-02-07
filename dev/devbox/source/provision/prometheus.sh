#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release
source .env
uname -a
env | sort

PROMETHEUS_BIN_URL="https://github.com/prometheus/prometheus/releases/download/v2.19.3/prometheus-2.19.3.linux-amd64.tar.gz"
PROMETHEUS_BIN_DIR="/usr/local/bin"
PROMETHEUS_ETC_DIR="/etc/prometheus"
PROMETHEUS_VAR_LIB_DIR="/var/lib/prometheus"

function install_prometheus {
  sudo groupadd --system prometheus
  sudo useradd -s /sbin/nologin --system -g prometheus prometheus

  tmp_dir="$(mktemp -d -t)"
  curl -L "${PROMETHEUS_BIN_URL}" \
    | tar --strip-components=1 --directory="${tmp_dir}" -xzv

  sudo mkdir -p "${PROMETHEUS_VAR_LIB_DIR}"
  sudo chown -R prometheus:prometheus "${PROMETHEUS_VAR_LIB_DIR}"

  for what in prometheus promtool tsdb; do
    sudo mv "${tmp_dir}/${what}" "${PROMETHEUS_BIN_DIR}/${what}"
    sudo chown prometheus:prometheus "${PROMETHEUS_BIN_DIR}/${what}"
  done

  sudo mkdir -p "${PROMETHEUS_ETC_DIR}"
  for what in consoles console_libraries; do
    sudo mv "${tmp_dir}/${what}" "${PROMETHEUS_ETC_DIR}/"
  done
  sudo cp "/tmp/_prometheus/prometheus.yml" "${PROMETHEUS_ETC_DIR}/prometheus.yml"
  sudo chown -R prometheus:prometheus "${PROMETHEUS_ETC_DIR}"

	sudo tee /etc/systemd/system/prometheus.service <<EOF
[Unit]
Description=Prometheus
Documentation=https://prometheus.io/docs/introduction/overview/
Wants=network-online.target
After=network-online.target

[Service]
Type=simple
Environment="GOMAXPROCS=1"
User=prometheus
Group=prometheus
ExecReload=/bin/kill -HUP \$MAINPID
ExecStart=${PROMETHEUS_BIN_DIR}/prometheus \
  --config.file=${PROMETHEUS_ETC_DIR}/prometheus.yml \
  --storage.tsdb.path=${PROMETHEUS_VAR_LIB_DIR} \
  --web.console.templates=${PROMETHEUS_ETC_DIR}/consoles \
  --web.console.libraries=${PROMETHEUS_ETC_DIR}/console_libraries \
  --web.listen-address=0.0.0.0:9090 \
  --web.external-url=

SyslogIdentifier=prometheus
Restart=always

[Install]
WantedBy=multi-user.target
EOF
}

case "${ID}" in
  debian | ubuntu)
    export DEBIAN_FRONTEND="noninteractive"
    sudo -E apt-get install -y --no-install-recommends --allow-change-held-packages prometheus
    sudo cp "/tmp/_prometheus/prometheus.yml" "${PROMETHEUS_ETC_DIR}/prometheus.yml"
    ;;

  centos)
    install_prometheus
    ;;
esac

sudo systemctl disable prometheus
