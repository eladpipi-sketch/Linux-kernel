# This day's exercise, solved: make, then insmod the .ko in the VM.
obj-m := kbuf.o
PROGS := nb_read
KDIR ?= $(HOME)/os/linux

all: $(PROGS)
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

# static: the VM has no C++ runtime
$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -static -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
	rm -f $(PROGS)
