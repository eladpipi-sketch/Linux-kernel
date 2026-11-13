# Linux Internals in 22 Days — the lab

Code and scripts for "Linux Internals in 22 Days". Days 1–4 look at the kernel
from outside, on your own Linux machine. Days 5–18 build and boot your own Linux
7.3 in QEMU and write modules and drivers for each major part of the kernel.
Days 19–22 add timers and workqueues, containers, a device-tree driver on an
emulated arm64 board, and eBPF. Every program, module and exercise solution here
was built and run on Linux 7.3, GCC 13.3 and QEMU 8.2.2 on Ubuntu 24.04 (Day 22
on the test machine's own kernel). The course itself is
`Linux_Internals_in_22_Days.pdf`.

Clone it as `~/os/lab`; the scripts expect that path:

```
git clone -b ccr-77cd2602-bgjf5z https://github.com/eladpipi-sketch/Linux-kernel.git ~/os/lab
```

| Folder | Day | What's in it |
| --- | --- | --- |
| `setup/` | 5, 21 | `build-kernel.sh` (clone v7.3, configure with `lab.config`, compile), `make-rootfs.sh` and `init` (the VM's BusyBox user space), `run.sh` (boot it, with `~/os/lab` shared as `/lab`); for Day 21, `build-arm64.sh` and `run-arm64.sh` (the same for QEMU's arm64 `virt` board) |
| `day01/` | 1 | processes and system calls: `fork_exec.cpp`, `syscall_cost.cpp` |
| `day02/` | 2 | memory: `address_space.cpp`, `alloc.cpp`, `rss.cpp`, `count_lines.cpp` |
| `day03/` | 3 | threads, locks, signals: `threads.cpp`, `race.cpp`, `signals.cpp`, `deadlock.cpp` |
| `day04/` | 4 | files and I/O: `fd.cpp`, `writes.cpp`, `epoll_stdin.cpp` |
| `day06/` | 6 | `hello.c` (a module with a parameter), `crash.c` (a NULL dereference, to read an oops), `list.c` (`kmalloc`, `list_head`, `container_of`) |
| `day07/` | 7 | `getppid_test.cpp`, `syscall_test.cpp`, `hello-syscall.diff` (adds system call 473) |
| `day08/` | 8 | `tasks.c` (walks every `task_struct`), `worker.c` (a kernel thread) |
| `day09/` | 9 | `pagewalk.c` (walks page tables by hand), `pagemap_test.cpp` |
| `day10/` | 10 | `slabs.c` (slab, buddy and vmalloc allocators), `labmap.c` and `labmap_test.cpp` (maps a kernel page into a process) |
| `day11/` | 11 | `kbuf.c` (`/dev/kbuf`: read, write, ioctl, poll, sysfs, debugfs), `kbuf.h`, `kbuf_test.cpp`, `epoll_test.cpp` |
| `day12/` | 12 | `race.c` (a data race, fixed with an atomic and a spinlock), `rcu_lab.c`, `lockorder.c` (a deadlock lockdep predicts) |
| `day13/` | 13 | `edu.c`: a PCI driver for QEMU's `edu` card (MMIO, MSI, threaded IRQ, DMA) |
| `day14/` | 14 | `labdisk.c`: a blk-mq RAM disk |
| `day15/` | 15 | `labfs.c`: a small file system built on libfs |
| `day16/` | 16 | `labnet.c` (a network driver that answers ping), `labfw.c` (a netfilter hook) |
| `day17/` | 17 | `gadget.sh` (a vendor-specific USB device, built with configfs on `dummy_hcd`), `usb_echo.c` (its driver) |
| `day18/` | 18 | `buggy.c` (bugs for KASAN), `leak.c` (leaks for kmemleak), `mystery.c` (the exercise: find its two bugs) |
| `day19/` | 19 | `deferred.c`: one job as a timer, a work item and an hrtimer, each reporting its context |
| `day20/` | 20 | `solution/box.cpp`: a container in 42 lines (namespaces plus a cgroup) |
| `day21/` | 21 | `lab.dts` (QEMU's device tree plus your node), `blinker.c` (a platform driver that blinks a GPIO line), built for arm64 |
| `day22/` | 22 | `syscalls.bpf.c` (an eBPF program) and `syscalls.cpp` (its libbpf loader), `getppid.bt` and `runqlat.bt` (bpftrace scripts) |
| `esp32/` | after 22 | `echo/echo.ino` (an ESP32-S3 as the same USB device, for the Arduino IDE), `usb_test.cpp` (talks to it with libusb) |

Every day has an exercise; `dayNN/solution/` holds a tested answer. Most are a
small change to that day's code, so `diff dayNN/x.c dayNN/solution/x.c` shows it.
Day 5's exercise needs no code, and Day 7's is a script of tracefs commands.

`make` in a day's folder builds what's there. In Days 1–4 that's ordinary C++
programs for your machine. From Day 6 on it builds kernel modules against
`~/os/linux` (set `KDIR=` for another tree), and C++ test programs with
`g++ -std=c++14 -static`, because the VM has no C++ runtime. Day 21 cross-builds
its module for arm64 against `~/os/linux-arm64`. Day 22 runs on your own machine:
clang builds the eBPF program and g++ its loader.

Start:

```
~/os/lab/setup/build-kernel.sh      # runs while you do Days 1-4
~/os/lab/setup/make-rootfs.sh
cd ~/os/lab/day01 && make
~/os/lab/setup/run.sh               # Day 5 onward; in the VM: cd /lab/day06
```
