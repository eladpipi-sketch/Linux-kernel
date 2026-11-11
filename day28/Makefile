# day28: "make" builds everything here against ~/os/linux.
obj-m := race.o rcu_lab.o lockorder.o
KDIR ?= $(HOME)/os/linux

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
