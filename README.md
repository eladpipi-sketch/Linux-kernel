# Linux Internals: 20 More Days — companion code

The tested, end-of-day versions of the code in "Linux Internals: 20 More Days"
(Days 21–40: kernel modules and drivers). Built and run on Ubuntu 24.04 with
Linux 7.3, GCC 13 and QEMU 8.2.

| Path | Day | What it is |
| --- | --- | --- |
| `vm/init` | 22 | PID 1 of the QEMU virtual machine |
| `vm/run.sh` | 22 | packs your files into an initramfs and boots your kernel |
| `drivers/Makefile` | 23 | builds every module against `~/os/linux` |
| `drivers/hello.c` | 23 | first module, with a parameter |
| `drivers/crash.c` | 24 | a deliberate NULL pointer bug, to read an oops |
| `drivers/list.c` | 25 | `kmalloc`, `list_head`, `container_of` |
| `drivers/kbuf.c`, `kbuf.h`, `kbuf_test.cpp` | 26–30 | `/dev/kbuf`, a tiny pipe in the kernel, plus its ioctl test |
| `drivers/edu.c` | 31–36 | PCI driver for QEMU's `edu` device: MMIO, interrupts, DMA |

To use it, copy `vm/` and `drivers/` into `~/os` (`cp vm/init ~/os/vm/rootfs/init`
for the init script), then follow the days. The guide builds each file up step
by step; these are the finished versions to compare against.
