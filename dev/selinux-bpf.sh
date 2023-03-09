#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

if [ "$EUID" -ne 0 ]; then
   echo "This script must be run as root"
   exit 1
fi

result=$(which selinuxenabled) || true
if [[ "$result" == "" ]]
then
  echo "selinuxenabled command not found - exiting"
  exit 0
fi

if ! selinuxenabled
then
    echo "SELinux is not enabled - exiting"
    exit 0
fi

tmp_dir=$(mktemp -d -t EBPF_NET-XXXXX)

cat > "${tmp_dir}/spc_bpf_allow.te" <<END
module spc_bpf_allow 1.0;
require {
    type spc_t;
    class bpf {map_create map_read map_write prog_load prog_run};
}
#============= spc_t ==============
allow spc_t self:bpf { map_create map_read map_write prog_load prog_run };
END
checkmodule -M -m -o "${tmp_dir}/spc_bpf_allow.mod" "${tmp_dir}/spc_bpf_allow.te"
semodule_package -o "${tmp_dir}/spc_bpf_allow.pp" -m "${tmp_dir}/spc_bpf_allow.mod"
semodule -i "${tmp_dir}/spc_bpf_allow.pp"
