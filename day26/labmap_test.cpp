// Map the kernel's page, read what the kernel wrote, write back. No read()/write().
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
    char* shared = (char*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("the kernel wrote: %s\n", shared);
    strcpy(shared, "hello from user space");    // a plain store, no system call
    munmap(shared, 4096);
    close(fd);
}
