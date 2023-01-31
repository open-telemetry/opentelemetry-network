# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # monorepo clone
  flowmill_src = "#{ENV['EBPF_NET_SRC']}"
  if (not flowmill_src.empty?) and File.exist?(flowmill_src)
    config.vm.synced_folder "#{ENV['EBPF_NET_SRC']}", "/home/vagrant/src", type: "sshfs"
  end

  # benv build artifacts
  flowmill_benv_out = "/tmp/flowmill-benv-out"
  if File.exist?(flowmill_benv_out)
    config.vm.synced_folder flowmill_benv_out, "/home/vagrant/out", type: "sshfs"
  end

  # disable default vagrant share.
  config.vm.synced_folder '.', '/vagrant', disabled: true

  config.vm.provider "virtualbox" do |vm|
    vm.gui = false
    vm.memory = "16384"
  end

  config.vm.boot_timeout = 600
  config.ssh.insert_key = true
end
