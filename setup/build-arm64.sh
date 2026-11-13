#!/bin/bash
# Day 21: Linux 7.3 for QEMU's arm64 "virt" board, and a user space for it.
# First: sudo apt install -y gcc-aarch64-linux-gnu qemu-system-arm device-tree-compiler
set -e
cd ~/os
[ -d linux-arm64 ] || git -C linux worktree add --detach ../linux-arm64 v7.3
cd linux-arm64
A="ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-"
# A virtual machine's drivers, built in; mod2noconfig drops every optional module
make $A defconfig virt.config mod2noconfig
# Built in too: 9p for the shared /lab folder, and the function tracer
scripts/config -e NET_9P -e NET_9P_VIRTIO -e 9P_FS -e FTRACE -e FUNCTION_TRACER
make $A olddefconfig
make $A -j"$(nproc)"

# The same user space as the x86 VM, with Ubuntu's arm64 build of BusyBox
mkdir -p ~/os/vm-arm64/rootfs/{bin,dev,proc,sys,root,tmp,mnt,lab}
cd ~/os/vm-arm64
deb=busybox-static_1.36.1-6ubuntu3_arm64.deb
[ -f $deb ] || wget -q http://ports.ubuntu.com/ubuntu-ports/pool/main/b/busybox/$deb
dpkg-deb -x $deb busybox-deb
cp busybox-deb/usr/bin/busybox rootfs/bin/
cp ~/os/lab/setup/init rootfs/init
cd rootfs
find . | cpio -o -H newc --quiet | gzip > ../initramfs.cpio.gz
