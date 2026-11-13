# day21: "make" builds the module for arm64, against ~/os/linux-arm64.
obj-m := blinker.o
KDIR ?= $(HOME)/os/linux-arm64
ARM64 := ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) $(ARM64) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) $(ARM64) clean
