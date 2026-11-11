// fault_test: map 4 pages of /dev/labmap, but write to pages 0 and 2 only.
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

int main()
{
    int fd = open("/dev/labmap", O_RDWR);
    if (fd < 0) {
        perror("open /dev/labmap");
        return 1;
    }
    char* shared = (char*)mmap(nullptr, 4 * 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    puts("mapped 4 pages, touching pages 0 and 2");
    strcpy(shared, "page 0 was here");
    strcpy(shared + 2 * 4096, "page 2 was here");
    munmap(shared, 4 * 4096);
    close(fd);
}
