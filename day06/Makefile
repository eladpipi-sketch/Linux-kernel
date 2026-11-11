# day06: "make" builds everything here against ~/os/linux.
obj-m := hello.o crash.o list.o
# Let crash.c fault the plain way: KASAN would catch its bug first.
KASAN_SANITIZE_crash.o := n
KDIR ?= $(HOME)/os/linux

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
