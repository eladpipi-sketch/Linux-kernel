# day27: "make" builds everything here against ~/os/linux.
obj-m := kbuf.o
PROGS := kbuf_test epoll_test
KDIR ?= $(HOME)/os/linux

all: $(PROGS)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

# static: the VM has no C++ runtime
$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -static -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f $(PROGS)
