#!/bin/bash -xe
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


source /etc/os-release

###########################
# enable core dumps

if [[ "${ID}" == "ubuntu" ]]
then
  sudo sed -i 's/enabled=1/enabled=0/' /etc/default/apport
fi

sudo bash -c 'cat >> /etc/security/limits.conf <<EOF
*               soft    core            unlimited
EOF'

sudo bash -c 'cat > /etc/sysctl.d/90-core-pattern.conf <<EOF
kernel.core_pattern = core-%e-%s-%u-%g-%p-%t
EOF'
