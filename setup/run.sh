#!/bin/bash
# Boot the lab kernel. ~/os/lab appears inside the VM as /lab.
# Extra arguments go on the kernel command line, e.g.  run.sh initcall_debug
# Two optional variables, used after Day 18:
#   KERNEL=linux-rust/arch/x86/boot/bzImage    boot another kernel image
#   QEMU_EXTRA="-device pci-testdev"           add QEMU options
# Quit: type exit in the VM, or press Ctrl-a then x.
# acpi=noirq: route legacy PCI interrupts without ACPI, whose routing method
# runs very slowly in an emulated (non-KVM) VM.
cd ~/os
accel="-accel tcg,thread=multi"
if [ -w /dev/kvm ]; then
    accel="-enable-kvm -cpu host"
fi
exec qemu-system-x86_64 $accel -m 2G -smp 2 -nographic -no-reboot \
    -kernel "${KERNEL:-linux/arch/x86/boot/bzImage}" \
    -initrd vm/initramfs.cpio.gz \
    -append "console=ttyS0 nokaslr panic=-1 acpi=noirq $*" \
    -device edu,addr=4 \
    -virtfs local,path="$HOME/os/lab",mount_tag=lab,security_model=none \
    -s $QEMU_EXTRA
