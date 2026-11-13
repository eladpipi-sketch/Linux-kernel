#!/bin/bash
# Boot the arm64 kernel on QEMU's "virt" board (Day 21). ~/os/lab is /lab, as on x86.
# Extra arguments go on the kernel command line, as with run.sh.
#   DTB=lab.dtb                            boot with your device tree, not QEMU's
#   QEMU_EXTRA="-M dumpdtb=$PWD/virt.dtb"  save QEMU's device tree and exit
# Quit: type exit in the VM, or press Ctrl-a then x.
cd ~/os
exec qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1G -smp 2 -nographic -no-reboot \
    -kernel linux-arm64/arch/arm64/boot/Image \
    -initrd vm-arm64/initramfs.cpio.gz \
    ${DTB:+-dtb "$DTB"} \
    -append "console=ttyAMA0 panic=-1 $*" \
    -virtfs local,path="$HOME/os/lab",mount_tag=lab,security_model=none \
    $QEMU_EXTRA
