# day22: "make" builds the eBPF program (with clang) and its loader (with g++).
# Needs: sudo apt install -y clang libbpf-dev bpftrace
all: syscalls.bpf.o syscalls

# -g adds BTF, the type information libbpf needs to create the maps
%.bpf.o: %.bpf.c
	clang -O2 -g -target bpf -I/usr/include/$(shell uname -m)-linux-gnu -c $< -o $@

syscalls: syscalls.cpp
	g++ -std=c++14 -Wall -o $@ $< -lbpf

clean:
	rm -f syscalls *.bpf.o
