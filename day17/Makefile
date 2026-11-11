# day17: "make" builds everything here against ~/os/linux.
obj-m := usb_echo.o
KDIR ?= $(HOME)/os/linux

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
