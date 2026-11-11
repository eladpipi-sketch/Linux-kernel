# This day's exercise, solved: make, then insmod the .ko in the VM.
obj-m := pagewalk.o
KDIR ?= $(HOME)/os/linux

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
