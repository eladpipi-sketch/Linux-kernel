// syscalls: load syscalls.bpf.o into the kernel, attach it to every system
// call, wait, then read the map it filled: system calls per program.
// Run as root: sudo ./syscalls [seconds]
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char** argv)
{
    int seconds = argc > 1 ? atoi(argv[1]) : 5;

    bpf_object* obj = bpf_object__open_file("syscalls.bpf.o", nullptr);
    if (!obj || bpf_object__load(obj) != 0) {           // the verifier runs here
        fprintf(stderr, "can't load syscalls.bpf.o: see libbpf's messages above\n");
        return 1;
    }
    bpf_program* prog = bpf_object__find_program_by_name(obj, "count_syscall");
    bpf_link* link = bpf_program__attach(prog);         // now it runs on every system call
    if (!link) {
        fprintf(stderr, "can't attach the program\n");
        return 1;
    }
    printf("counting every program's system calls for %d seconds...\n", seconds);
    sleep(seconds);

    // Walk the map: each key is a program name, each value its count.
    int map = bpf_object__find_map_fd_by_name(obj, "counts");
    std::vector<std::pair<unsigned long long, std::string>> rows;
    char key[16], next_key[16];
    const char* prev = nullptr;                         // nullptr asks for the first key
    while (bpf_map_get_next_key(map, prev, next_key) == 0) {
        unsigned long long count = 0;
        bpf_map_lookup_elem(map, next_key, &count);
        rows.emplace_back(count, next_key);
        memcpy(key, next_key, sizeof(key));
        prev = key;
    }
    std::sort(rows.rbegin(), rows.rend());              // most calls first
    for (size_t i = 0; i < rows.size() && i < 10; ++i)
        printf("%10llu  %s\n", rows[i].first, rows[i].second.c_str());

    bpf_link__destroy(link);                            // detach
    bpf_object__close(obj);                             // and unload
}
