// alloc: malloc takes small blocks from the heap (brk) and gives big ones
// their own mapping (mmap).
#include <cstdlib>

int main()
{
    void* small_block = malloc(100);
    void* big_block = malloc(1024 * 1024);
    free(big_block);                    // munmap: straight back to the kernel
    free(small_block);
}
