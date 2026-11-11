# day23: "make" builds everything here against ~/os/linux.
PROGS := getppid_test syscall_test
KDIR ?= $(HOME)/os/linux

all: $(PROGS)

# static: the VM has no C++ runtime
$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -static -o $@ $<

clean:
	rm -f $(PROGS)
