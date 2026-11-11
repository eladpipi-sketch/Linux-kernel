# day03: "make" builds the programs here. They run on your machine, not in the VM.
PROGS := threads race signals deadlock

all: $(PROGS)

$(PROGS): %: %.cpp
	g++ -std=c++14 -Wall -pthread -o $@ $<

clean:
	rm -f $(PROGS)
