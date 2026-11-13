# This day's exercise, solved: make, then insmod blinker.ko in the arm64 VM.
obj-m := blinker.o
KDIR ?= $(HOME)/os/linux-arm64
ARM64 := ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) $(ARM64) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) $(ARM64) clean
