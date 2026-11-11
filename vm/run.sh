#!/bin/bash
# Boot your kernel in QEMU, with your modules and test programs in /root.
# Quit: type exit in the VM, or press Ctrl-a then x.
set -e
cd ~/os
cp drivers/*.ko drivers/*_test vm/rootfs/root/ 2>/dev/null || true
(cd vm/rootfs && find . | cpio -o -H newc --quiet | gzip) > vm/initramfs.cpio.gz

accel=""
if [ -w /dev/kvm ]; then
    accel="-enable-kvm -cpu host"
fi

qemu-system-x86_64 $accel -m 1G -smp 2 -nographic -no-reboot \
    -kernel linux/arch/x86/boot/bzImage \
    -initrd vm/initramfs.cpio.gz \
    -append "console=ttyS0 nokaslr panic=-1" \
    -device edu -s
