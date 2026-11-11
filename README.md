# Linux Internals: 14 Hard Days — the lab

Code and scripts for Part 2 of "Linux Internals" (Days 21–34). You build and boot your
own Linux 7.3 in QEMU, then write modules and drivers for each major part of the
kernel. Every file here was built and run on Linux 7.3, GCC 13.3 and QEMU 8.2.2 on
Ubuntu 24.04.

Clone it as `~/os/lab`; the scripts expect that path:

```
git clone -b ccr-77cd2602-bgjf5z https://github.com/eladpipi-sketch/Linux-kernel.git ~/os/lab
```

| Folder | Day | What's in it |
| --- | --- | --- |
| `setup/` | 21 | `build-kernel.sh` (clone v7.3, configure with `lab.config`, compile), `make-rootfs.sh` and `init` (the VM's BusyBox user space), `run.sh` (boot it, with `~/os/lab` shared as `/lab`) |
| `day22/` | 22 | `hello.c` (a module with a parameter), `crash.c` (a NULL dereference, to read an oops), `list.c` (`kmalloc`, `list_head`, `container_of`) |
| `day23/` | 23 | `getppid_test.cpp`, `syscall_test.cpp`, `hello-syscall.diff` (adds system call 473) |
| `day24/` | 24 | `tasks.c` (walks every `task_struct`), `worker.c` (a kernel thread) |
| `day25/` | 25 | `pagewalk.c` (walks a process's page tables), `pagemap_test.cpp` |
| `day26/` | 26 | `slabs.c` (slab, buddy and vmalloc allocators), `labmap.c` and `labmap_test.cpp` (maps a kernel page into a process) |
| `day27/` | 27 | `kbuf.c` (`/dev/kbuf`: read, write, ioctl, poll, sysfs, debugfs), `kbuf.h`, `kbuf_test.cpp`, `epoll_test.cpp` |
| `day28/` | 28 | `race.c` (a data race, fixed with an atomic and a spinlock), `rcu_lab.c`, `lockorder.c` (a deadlock lockdep predicts) |
| `day29/` | 29 | `edu.c`: a PCI driver for QEMU's `edu` card (MMIO, MSI, threaded IRQ, DMA) |
| `day30/` | 30 | `labdisk.c`: a blk-mq RAM disk |
| `day31/` | 31 | `labfs.c`: a small file system built on libfs |
| `day32/` | 32 | `labnet.c` (a network driver that answers ping), `labfw.c` (a netfilter hook) |
| `day33/` | 33 | `gadget.sh` (a vendor-specific USB device, built with configfs on `dummy_hcd`), `usb_echo.c` (its driver) |
| `day34/` | 34 | `buggy.c` (bugs for KASAN), `leak.c` (leaks for kmemleak) |
| `pico/` | after 34 | `usb_test.cpp`: a libusb test for a Raspberry Pi Pico acting as the same USB device |

In each `dayNN/`, `make` builds the modules against `~/os/linux` (set `KDIR=` to use
another tree) and the C++ test programs with `g++ -std=c++14 -static`, because the
VM has no C++ runtime.

Start:

```
~/os/lab/setup/build-kernel.sh      # 15-30 min
~/os/lab/setup/make-rootfs.sh
cd ~/os/lab/day22 && make
~/os/lab/setup/run.sh               # in the VM: cd /lab/day22; insmod hello.ko
```
