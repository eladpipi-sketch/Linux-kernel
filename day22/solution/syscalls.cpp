// Day 22 exercise: attach both programs, then print each program's system
// calls and the bytes it wrote.
// Run as root: sudo ./syscalls [seconds]
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
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
    std::vector<bpf_link*> links;
    bpf_program* prog;
    bpf_object__for_each_program(prog, obj) {
        bpf_link* link = bpf_program__attach(prog);
        if (!link) {
            fprintf(stderr, "can't attach %s\n", bpf_program__name(prog));
            return 1;
        }
        links.push_back(link);
    }
    printf("counting every program's system calls for %d seconds...\n", seconds);
    sleep(seconds);

    // Walk the map: each key is a program name, each value its count.
    int map = bpf_object__find_map_fd_by_name(obj, "counts");
    int written = bpf_object__find_map_fd_by_name(obj, "written");
    std::vector<std::pair<unsigned long long, std::string>> rows;
    std::map<std::string, unsigned long long> bytes_written;
    char key[16], next_key[16];
    const char* prev = nullptr;                         // nullptr asks for the first key
    while (bpf_map_get_next_key(map, prev, next_key) == 0) {
        unsigned long long count = 0;
        bpf_map_lookup_elem(map, next_key, &count);
        rows.emplace_back(count, next_key);
        unsigned long long bytes = 0;                   // stays 0 if it never wrote
        bpf_map_lookup_elem(written, next_key, &bytes); // the same 16-byte key
        bytes_written[next_key] = bytes;
        memcpy(key, next_key, sizeof(key));
        prev = key;
    }
    std::sort(rows.rbegin(), rows.rend());              // most calls first
    printf("%10s  %13s  %s\n", "calls", "bytes written", "program");
    for (size_t i = 0; i < rows.size() && i < 10; ++i)
        printf("%10llu  %13llu  %s\n", rows[i].first, bytes_written[rows[i].second],
               rows[i].second.c_str());

    for (bpf_link* link : links)
        bpf_link__destroy(link);                        // detach
    bpf_object__close(obj);                             // and unload
}
