// Ask /proc/self/pagemap where one page of our memory lives in RAM.
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>

static void report(const char* when, void* p)
{
    uint64_t entry = 0;
    int fd = open("/proc/self/pagemap", O_RDONLY);
    pread(fd, &entry, sizeof(entry), ((uintptr_t)p / 4096) * sizeof(entry));
    close(fd);
    bool present = entry >> 63;                    // bit 63: page is in RAM
    uint64_t pfn = entry & ((1ULL << 55) - 1);     // bits 0-54: page frame number
    printf("%-13s present=%d pfn=%#llx\n", when, present, (unsigned long long)pfn);
}

int main()
{
    char* page = (char*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("pid %d, page at %p\n", getpid(), (void*)page);
    report("before touch:", page);
    page[0] = 1;                                   // first write: page fault
    report("after touch:", page);
    fflush(stdout);
    sleep(600);                                    // stay alive for pagewalk.ko
}
