#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xe

kernel_headers_info_path="$1"
kernel_version="$(uname -r)"

kernel_headers_usr_src_base_path="/usr/src"
kernel_headers_lib_modules_base_path="/lib/modules"

host_dir="${EBPF_NET_HOST_DIR:-/hostfs}"
host_etc_dir="${host_dir}/etc"
host_yum_vars_dir="${host_etc_dir}/yum/vars"
host_cache_dir="${host_dir}/cache/flowmill"
host_usr_src_dir="${host_dir}${kernel_headers_usr_src_base_path}"
host_lib_modules_dir="${host_dir}${kernel_headers_lib_modules_base_path}"
host_kernel_headers_dir="${host_lib_modules_dir}/${kernel_version}"
host_cache_kernel_headers_dir="${host_cache_dir}/kernel-headers"
host_cache_kernel_headers_archive="${host_cache_kernel_headers_dir}/${kernel_version}.tar.gz"

kernel_headers_lib_modules_path="${kernel_headers_lib_modules_base_path}/${kernel_version}"
kernel_headers_beacon_path=( \
  "build/include/linux/tcp.h"
  "source/include/linux/tcp.h"
)

entrypoint_error=""
kernel_headers_source="unknown"

function check_kernel_headers_installed {
  base_dir="${kernel_headers_lib_modules_path}"
  if [[ -n "$1" ]]; then
    base_dir="$1"
  fi

  for header_file in "${kernel_headers_beacon_path[@]}"; do
    if [[ -e "${base_dir}/${header_file}" ]]; then
      return 0
    fi
  done
  return 1
}

function detect_distro {
  debian_os_file="${host_etc_dir}/debian_version"
  os_release_file="${host_etc_dir}/os-release"
  system_release_file="${host_etc_dir}/system-release"

  if [[ -e "${debian_os_file}" ]]; then
    echo "debian"
    return
  fi

  if [[ -e "${os_release_file}" ]]; then
    os_id="$(grep '^ID=' "${os_release_file}" 2> /dev/null | sed -e 's/ID=\(.*\)/\1/g' -e 's/"//g')"
    case "${os_id}" in
      debian | ubuntu | centos | amazon | rhel)
        echo "${os_id}"
        return
        ;;
      amzn)
        # Amazon Linux 2
        echo "amazon"
        return
        ;;
      cos)
        echo "gcp_cos"
        return
        ;;
    esac
  fi

  if [[ -e "${system_release_file}" ]]; then
    if grep 'CentOS' "${system_release_file}" > /dev/null 2> /dev/null; then
      echo "centos"
      return
    elif grep 'RHEL' "${system_release_file}" > /dev/null 2> /dev/null; then
      echo "rhel"
      return
    elif grep 'Amazon Linux' "${system_release_file}" > /dev/null 2> /dev/null; then
      echo "amazon"
      return
    fi
  fi

  echo "unknown"
}

host_distro="$(detect_distro)"

function install_apt_kernel_headers {
  kernel_headers_pkg_name="linux-headers-${kernel_version}"

  sources_list="${host_etc_dir}/apt/sources.list"
  if [[ ! -e "${sources_list}" ]]; then
    entrypoint_error="kernel_headers_missing_repo"
    echo "host apt repo sources.list file not found under '${sources_list}'"
    return 1
  fi

  apt_cmd_args=( \
    --no-install-recommends
    -o "Dir::Etc::sourcelist=${sources_list}"
  )

  sources_list_d="${host_etc_dir}/apt/sources.list.d"
  if [[ -e "${sources_list_d}" ]]; then
    apt_cmd_args+=(-o "Dir::Etc::sourceparts=${sources_list_d}")
  fi

  trusted_gpg="${host_etc_dir}/apt/trusted.gpg"
  if [[ -e "${trusted_gpg}" ]]; then
    apt_cmd_args+=(-o "Dir::Etc::trusted=${trusted_gpg}")
  fi

  trusted_gpg_d="${host_etc_dir}/apt/trusted.gpg.d"
  if [[ -e "${trusted_gpg_d}" ]]; then
    apt_cmd_args+=(-o "Dir::Etc::trustedparts=${trusted_gpg_d}")
  fi

  if ! apt-get update "${apt_cmd_args[@]}"; then
    entrypoint_error="kernel_headers_misconfigured_repo"
    return 1
  fi

  apt-get install --yes --no-install-recommends \
    "${apt_cmd_args[@]}" \
    "${kernel_headers_pkg_name}" \
    || return 1
}

function install_yum_kernel_headers {
  [[ -e "${kernel_headers_usr_src_base_path}" ]] && rm -rf "${kernel_headers_usr_src_base_path}"
  [[ -e "${kernel_headers_lib_modules_base_path}" ]] && rm -rf "${kernel_headers_lib_modules_base_path}"

  kernel_headers_pkg_name="kernel-devel-${kernel_version}"

  yum_conf="${host_etc_dir}/yum.conf"
  if [[ ! -e "${yum_conf}" ]]; then
    entrypoint_error="kernel_headers_missing_repo"
    echo "host yum.conf file not found under '${yum_conf}'"
    return 1
  fi

  # `yum.conf` and friends can use some placeholder variables that `yum` itself
  # figures out from the OS. Given that we're not running on the host
  # environment - we only have access to the host's file system - we need to
  # work around by explicitly replacing the variables, which we do with `sed`

  sed_args=( \
    -e "s/\\\$basearch/x86_64/g"
    -e "s/\\\$arch/x86_64/g"
  )

  yum_repo_files=()

  os_release_file="${host_etc_dir}/os-release"
  system_release_file="${host_etc_dir}/system-release"
  cpe_release_file="${host_etc_dir}/system-release-cpe"
  release_version_found='false'
  if [[ -e "${os_release_file}" ]]; then
    yum_repo_files+=("os-release")
    os_version="$(grep '^VERSION_ID=' "${os_release_file}" 2> /dev/null \
      | sed -e 's/VERSION_ID=\(.*\)/\1/g' -e 's/"//g')"

    if [[ -n "${os_version}" ]]; then
      sed_args+=(-e "s/\\\$releasever/${os_version}/g")
      release_version_found='true'
    fi
  fi

  if [[ -e "${cpe_release_file}" ]]; then
    yum_repo_files+=("system-release-cpe")

    if [[ "${release_version_found}" != 'true' ]]; then
      os_version="$(< "${cpe_release_file}" cut -d ':' -f5)"

      if [[ -n "${os_version}" ]]; then
        sed_args+=(-e "s/\\\$releasever/${os_version}/g")
        release_version_found='true'
      fi
    fi
  fi

  if [[ -e "${system_release_file}" ]]; then
    yum_repo_files+=("system-release")
  fi

  sed "${sed_args[@]}" "${yum_conf}" > "/etc/yum.conf"
  yum_repo_files+=("yum.conf")

  # need the certs referred to by the yum repo list
  ln -s "${host_etc_dir}/pki" "/etc/pki"

  # if exists, get the /etc/yum/vars so yum can fill $ variables
  if [[ -e "${host_yum_vars_dir}" ]]; then
    # it should exist on our container, but let's make sure
    mkdir -p /etc/yum
    yum_repo_files+=("yum")
    
    # the vars directory should exist, remove it if it does
    rm -rf /etc/yum/vars || true

    # link the host vars directory to /etc/yum/vars
    ln -s "${host_yum_vars_dir}" /etc/yum/vars
  fi

  yum_repos_dir="${host_etc_dir}/yum.repos.d"
  if [[ -d "${yum_repos_dir}" ]]; then
    cp -R "${yum_repos_dir}" "/etc/yum.repos.d"
    yum_repo_files+=("yum.repos.d")
    for repo in /etc/yum.repos.d/*; do
      sed -i "${sed_args[@]}" "${repo}"
    done
  fi

  repo_files_path=()
  for file in "${yum_repo_files[@]}"; do
    if [[ -e "${host_etc_dir}/${file}" ]]; then
        repo_files_path+=("${host_etc_dir}/${file}")
    fi
    if [[ -e "/etc/${file}" ]]; then
        repo_files_path+=("/etc/${file}")
    fi
  done

  if ! yum list > /dev/null; then
    entrypoint_error="kernel_headers_misconfigured_repo"
    
    # show the repo list for debugging later
    yum repolist -v || true

    # dump repo settings in a compact way
    printf "repo settings (decode with \`| tr '|' '\\\\n' | base64 -d | tar xzv\`)\\n"
    tar cz --dereference --hard-dereference "${repo_files_path[@]}" \
      | base64 | paste -d '|' -s \
      || true
    
    return 1
  fi

  # show available kernel headers versions for debuging purposes
  yum list | grep kernel-devel || true

  # we can't `yum install` here - it would install the whole base system
  # given that we're installing packages from a foreign OS. Instead we
  # download the RPM package and install it without its dependencies

  download_dir="$(mktemp -d yum-pkg-XXXXX)"
  if ! yumdownloader "--destdir=${download_dir}" "${kernel_headers_pkg_name}"; then
    # dump repo settings in a compact way
    printf "repo settings (decode with \`| tr '|' '\\\\n' | base64 -d | tar xzv\`)\\n"
    tar cz --dereference --hard-dereference "${repo_files_path[@]}" \
      | base64 | paste -d '|' -s \
      || true

    return 1
  fi

  rpm_file="${download_dir}/${kernel_headers_pkg_name}.rpm"
  [[ -e "${rpm_file}" ]] || return 1

  rpm -i --nodeps "${rpm_file}" || return 1

  installed_kernel_headers_dir="${kernel_headers_usr_src_base_path}/kernels/${kernel_version}"
  kernel_headers_dir="${kernel_headers_lib_modules_base_path}/${kernel_version}"
  mkdir -p "${kernel_headers_dir}"

  for link_name in build source; do
    ln -s "${installed_kernel_headers_dir}" "${kernel_headers_dir}/${link_name}" || return 1
  done
}

function install_gcp_cos_kernel_headers {
  kernel_headers_pkg_name="linux-headers-${kernel_version}"

  # GCP ChromeOS kernel headers download
  gcp_metadata_header="Metadata-Flavor: Google"
  gcp_metadata_base_url="http://metadata.google.internal/computeMetadata/v1/instance"

  gcp_image="$(curl -vvv -H "${gcp_metadata_header}" "${gcp_metadata_base_url}/image" || true)"
  gcp_curl_status="$?"
  if [[ "${gcp_curl_status}" -ne 0 ]] && [[ -z "${gcp_image}" ]]; then return 1; fi

  gcp_image_version="$(basename "${gcp_image}")"

  declare -a sed_args
  if [[ "cos" == "${gcp_image_version:0:3}" ]]; then
    sed_args+=(-e 's/cos-\([^-]*\)-\([^-]*\)-\([^-]*\)-\([^-]*\)-\([^-]*\)/\3.\4.\5/g')
  elif [[ "gke" == "${gcp_image_version:0:3}" ]]; then
    sed_args+=(-e 's/gke.*-\([0-9]\+\)-\([0-9]\+\)-\([0-9]\+\)-.*/\1.\2.\3/g')
  else
    echo "unknown GCP image: '${gcp_image}'"
    return 1
  fi

  gcp_cos_version="$(echo "${gcp_image_version}" | sed "${sed_args[@]}")"
  if [[ -z "${gcp_cos_version}" ]]; then return 1; fi

  gcp_cos_base_url="https://storage.googleapis.com/cos-tools/${gcp_cos_version}"
  gcp_cos_tmp_dir="$(mktemp -d "cos-${gcp_cos_version}-XXXXX" --tmpdir)"

  base_file='kernel-headers.tgz'
  http_status_code=""
  for file in "${base_file}" "${base_file}.md5"; do
    http_status_code="$(curl -vvv --write-out "%{http_code}" \
      "${gcp_cos_base_url}/${file}" \
      -o "${gcp_cos_tmp_dir}/${file}" \
    )"

    if [[ "${http_status_code}" != "200" ]]; then
      break
    fi
  done

  if [[ "${http_status_code}" != "200" ]]; then
    echo "unable to download kernel headers for GCP's COS ${gcp_cos_version}"
    return 1
  fi

  if [[ "$(md5sum "${gcp_cos_tmp_dir}/${base_file}" | cut -d ' ' -f 1)" \
    != "$(cat "${gcp_cos_tmp_dir}/${base_file}.md5")" \
  ]]; then
    echo "kernel headers failed checksum verification for GCP's COS version ${gcp_cos_version}"
    return 1
  fi

  tar xzvf "${gcp_cos_tmp_dir}/${base_file}" -C / | wc -l
  kernel_headers_usr_src_path="${kernel_headers_usr_src_base_path}/linux-headers-${kernel_version}/"

  if [[ ! -e "${kernel_headers_lib_modules_path}" ]]; then
    mkdir -p "${kernel_headers_lib_modules_path}"
    ln -s "${kernel_headers_usr_src_path}" "${kernel_headers_lib_modules_path}/build"
    ln -s "${kernel_headers_usr_src_path}" "${kernel_headers_lib_modules_path}/source"
  fi
}

function cache_kernel_headers {
  echo "caching kernel headers at ${host_cache_kernel_headers_archive}..."
  mkdir -p "${host_cache_kernel_headers_dir}"

  if [[ -e "${host_cache_kernel_headers_archive}.new" ]]; then
    rm -rf "${host_cache_kernel_headers_archive}.new"
  fi

  # To avoid caching an invalid archive, we create the archive under a
  # different name and rename it if successful. tar will return a non-zero
  # result if it fails to follow some symlinks but still succeeds at creating
  # the archive. In order to work around that, we ignore the result for archive
  # creation and check the integrity of the archive after it is created
  tar czf "${host_cache_kernel_headers_archive}.new" \
    --dereference --hard-dereference \
    -C "${kernel_headers_lib_modules_base_path}" \
    "${kernel_version}" \
    || true

  if tar tzf "${host_cache_kernel_headers_archive}.new" > /dev/null; then
    mv "${host_cache_kernel_headers_archive}.new" "${host_cache_kernel_headers_archive}"
    tar tzf "${host_cache_kernel_headers_archive}" | wc -l
  else
    echo "failed to cache kernel headers at ${host_cache_kernel_headers_archive}"
  fi
}

function use_cached_kernel_headers {
  if [[ ! -e "${host_cache_kernel_headers_archive}" ]]; then
    if [[ -d "${host_cache_kernel_headers_dir}" ]]; then
      # clean up older headers that we don't need
      rm -rf  "${host_cache_kernel_headers_dir}"
    fi

    return 1
  fi

  [[ -e "${kernel_headers_lib_modules_base_path}" ]] && rm -rf "${kernel_headers_lib_modules_base_path}"
  mkdir -p "${kernel_headers_lib_modules_base_path}"

  tar xzf "${host_cache_kernel_headers_archive}" \
    -C "${kernel_headers_lib_modules_base_path}"
}

function install_kernel_headers {
  [[ -e "${kernel_headers_usr_src_base_path}" ]] && rm -rf "${kernel_headers_usr_src_base_path}"
  [[ -e "${kernel_headers_lib_modules_base_path}" ]] && rm -rf "${kernel_headers_lib_modules_base_path}"

  case "${host_distro}" in
    debian | ubuntu)
      install_apt_kernel_headers
      ;;
    centos | rhel | amazon)
      install_yum_kernel_headers
      ;;
    gcp_cos)
      install_gcp_cos_kernel_headers
      ;;
    *)
      entrypoint_error="unsupported_distro"
      return 1
      ;;
  esac

  check_kernel_headers_installed \
    && cache_kernel_headers \
    || return 1
}

function use_host_kernel_headers {
  [[ -e "${kernel_headers_usr_src_base_path}" ]] && rm -rf "${kernel_headers_usr_src_base_path}"
  [[ -e "${kernel_headers_lib_modules_base_path}" ]] && rm -rf "${kernel_headers_lib_modules_base_path}"

  [[ -e "${host_usr_src_dir}" ]] && ln -s "${host_usr_src_dir}" "${kernel_headers_usr_src_base_path}"
  [[ -e "${host_lib_modules_dir}" ]] && ln -s "${host_lib_modules_dir}" "${kernel_headers_lib_modules_base_path}"

  if check_kernel_headers_installed "${host_kernel_headers_dir}"; then
    mkdir -p "${kernel_headers_lib_modules_base_path}"
    ln -s "${host_kernel_headers_dir}" "${kernel_headers_lib_modules_base_path}"

    if [[ -d "${kernel_headers_usr_src_base_path}" ]]; then
      rm -rf "${kernel_headers_usr_src_base_path}"
    fi
    ln -s "${host_usr_src_dir}" "${kernel_headers_usr_src_base_path}"
  fi

  check_kernel_headers_installed && return 0

  return 1
}

function resolve_kernel_headers {
  if check_kernel_headers_installed || use_host_kernel_headers; then
    kernel_headers_source="pre_installed"
  elif use_cached_kernel_headers; then
    kernel_headers_source="pre_fetched"
  elif [[ "${EBPF_NET_KERNEL_HEADERS_AUTO_FETCH}" == "false" ]]; then
    kernel_headers_source="dont_fetch"
    entrypoint_error="kernel_headers_fetch_refuse"
    echo "no kernel headers found, not auto-fetching as requested"
  else
    echo "no kernel headers found, attempting to auto-fetch..."
    if install_kernel_headers; then
      kernel_headers_source="fetched"
    elif [[ -z "${entrypoint_error}" ]]; then
      entrypoint_error="kernel_headers_fetch_error"
      echo "failed to auto-fetch kernel headers"
    fi
  fi
}

resolve_kernel_headers

cat > "${kernel_headers_info_path}" << EOF
export entrypoint_error='${entrypoint_error}'
export host_distro='${host_distro}'
export kernel_headers_source='${kernel_headers_source}'
EOF
