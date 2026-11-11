# day09: "make" builds everything here against ~/os/linux.
obj-m := pagewalk.o
PROGS := pagemap_test
KDIR ?= $(HOME)/os/linux

all: $(PROGS)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

# static: the VM has no C++ runtime
$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -static -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f $(PROGS)
