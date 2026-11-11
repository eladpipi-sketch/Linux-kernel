#!/bin/bash
# Get Linux 7.3 and build it with the lab options (lab.config).
set -e
cd ~/os
[ -d linux ] || git clone --depth 1 --branch v7.3 https://github.com/torvalds/linux.git
cd linux
make defconfig kvm_guest.config
scripts/kconfig/merge_config.sh -m .config ~/os/lab/setup/lab.config
make olddefconfig
make -j"$(nproc)"
