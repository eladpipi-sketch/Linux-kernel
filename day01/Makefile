# day01: "make" builds the programs here. They run on your machine, not in the VM.
PROGS := fork_exec syscall_cost

all: $(PROGS)

$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -pthread -o $@ $<

clean:
	rm -f $(PROGS)
