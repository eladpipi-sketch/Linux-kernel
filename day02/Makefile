# day02: "make" builds the programs here. They run on your machine, not in the VM.
PROGS := address_space alloc rss count_lines

all: $(PROGS)

$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -pthread -o $@ $<

clean:
	rm -f $(PROGS)
