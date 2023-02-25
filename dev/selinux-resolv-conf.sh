#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

if [ "$EUID" -ne 0 ]; then
   echo "This script must be run as root"
   exit 1
fi



##CentOS 7 uses NetworkManager for the network, the ones that dont have static (manually) configured network.
## So .. we need to chang the start of the docker unit file to include "NetworkManager-wait-online.service" in the "After=" line, 
## and the problem Error response from daemon: Get "https://quay.io/v2/": dial tcp: lookup quay.io on [::1]:53: read udp 
## [::1]:59407->[::1]:53: read: connection refused, doesn't happen anymore. 

sed -i '4s/$/ NetworkManager-wait-online.service/' /usr/lib/systemd/system/docker.service

sudo systemctl daemon-reload
sudo systemctl restart docker.service