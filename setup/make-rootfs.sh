#!/bin/bash
# The VM's whole user space: one static BusyBox binary plus the init script.
set -e
mkdir -p ~/os/vm/rootfs/{bin,dev,proc,sys,root,tmp,mnt,lab}
cp /bin/busybox ~/os/vm/rootfs/bin/
cp ~/os/lab/setup/init ~/os/vm/rootfs/init
chmod +x ~/os/vm/rootfs/init
cd ~/os/vm/rootfs
find . | cpio -o -H newc --quiet | gzip > ~/os/vm/initramfs.cpio.gz
