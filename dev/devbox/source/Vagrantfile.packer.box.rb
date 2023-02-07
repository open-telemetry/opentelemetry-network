# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # disable default vagrant share.
  config.vm.synced_folder '.', '/vagrant', disabled: true

  config.vm.provider "virtualbox" do |vm|
    vm.gui = false
    vm.memory = "16384"
  end

  config.vm.boot_timeout = 600
  config.ssh.insert_key = true
end
