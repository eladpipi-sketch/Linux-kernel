# Your modules. Each day adds a line here.
obj-m += hello.o
obj-m += crash.o
obj-m += list.o
obj-m += kbuf.o
obj-m += edu.o

KDIR := $(HOME)/os/linux

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
