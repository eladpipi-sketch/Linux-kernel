// count_lines: counts the lines of a file without calling read().
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("usage: %s <file>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    struct stat info;
    fstat(fd, &info);
    const char* data = (const char*)mmap(nullptr, info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    long lines = 0;
    for (off_t i = 0; i < info.st_size; ++i)
        if (data[i] == '\n')
            ++lines;
    printf("%ld lines\n", lines);
    munmap((void*)data, info.st_size);
    close(fd);
}
