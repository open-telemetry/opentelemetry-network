#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


function print_help {
  echo "usage: $0 binary-file [out-dir]" >&2
  echo >&2
  echo 'outputs stripped binary in ${BINARY_FILE}-stripped' >&2
  echo 'outputs debug symbols into symbols/${MODULE}/${BINARY_FILE}-stripped.sym' >&2
}

binary_file=""
export_symbols=false
upload_bucket=""
out_dir="."

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift

  case "${arg}" in
    --export)
      export_symbols=true
      ;;

    --upload)
      if [[ "$#" -lt 1 ]]; then
        print_help
        echo
        echo "ERROR: missing S3 bucket after '--upload'"
        exit 1
      fi
      upload_bucket="$1"; shift
      export_symbols=true
      ;;

    *)
      if [[ -z "${binary_file}" ]]; then
        binary_file="${arg}"
      elif [[ -z "${out_dir}" ]]; then
        out_dir="${arg}"
      else
        print_help
        echo
        echo "ERROR: unexpected arguments '${arg} $*'"
        exit 1
      fi
      ;;
  esac
done

if [[ -z "${binary_file}" ]] || [[ ! -e "${binary_file}" ]]; then
  print_help
  echo
  echo "ERROR: missing or inexistent binary file '${binary_file}'"
  exit 1
fi

module_name="$(basename "${binary_file}")"
stripped_file="${binary_file}-stripped"
symbol_file="${module_name}.sym"

module_id="$(dump_syms -i "${binary_file}" | grep MODULE | cut -d ' ' -f 4)"
symbol_dir="${module_name}/${module_id}"
symbol_path="symbols/${symbol_dir}/${symbol_file}"

cat > "debug-info.conf" <<EOF
export EBPF_NET_DEBUG_MODULE_NAME="${module_name}"
export EBPF_NET_DEBUG_MODULE_ID="${module_id}"
EOF

echo "stripping debug symbols from '${binary_file}' into '${stripped_file}'..." >&2
strip -s -o "${stripped_file}" "${binary_file}" >&2

if [[ "${export_symbols}" == true ]]; then
  mkdir -p "symbols/${symbol_dir}" >&2
  echo "exporting debug symbols from '${binary_file}' into '${symbol_path}'..." >&2
  dump_syms "${binary_file}" > "${symbol_path}"

  if [[ -n "${upload_bucket}" ]]; then
    if [[ -z "${AWS_ACCESS_KEY_ID}" ]] || [[ -z "${AWS_SECRET_ACCESS_KEY}" ]]; then
      echo "ERROR: AWS credentials missing - environment variables AWS_ACCESS_KEY_ID / AWS_SECRET_ACCESS_KEY not properly set up"
    fi

    echo "uploading debug symbols '${symbol_dir}' to S3 using AWS_ACCESS_KEY_ID='${AWS_ACCESS_KEY_ID}'"

    (set -x; aws s3api put-object \
      --bucket "${upload_bucket}" \
      --key "${symbol_dir}/${symbol_file}" \
      --body "${symbol_path}" \
    )
  fi
fi

echo "${stripped_file}"
