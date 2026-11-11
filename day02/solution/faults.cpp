// faults: how many page faults does it take to touch 64 MB?
#include <sys/mman.h>
#include <sys/resource.h>
#include <cstdint>
#include <cstdio>

long minor_faults()
{
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_minflt;
}

void touch(const char* label, size_t step, bool huge)
{
    const size_t size = 64 << 20;
    const size_t two_mb = 2 << 20;
    // Map 2 MB extra so the region can start on a 2 MB boundary, as huge pages need.
    char* raw = (char*)mmap(nullptr, size + two_mb, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char* start = (char*)(((uintptr_t)raw + two_mb - 1) & ~(uintptr_t)(two_mb - 1));
    if (huge)
        madvise(start, size, MADV_HUGEPAGE);

    long before = minor_faults();
    for (size_t i = 0; i < size; i += step)
        start[i] = 1;
    printf("%-24s %6ld faults\n", label, minor_faults() - before);
    munmap(raw, size + two_mb);
}

int main()
{
    touch("every 4 KB:", 4096, false);
    touch("every 8 KB:", 8192, false);
    touch("every 4 KB, huge pages:", 4096, true);
}
