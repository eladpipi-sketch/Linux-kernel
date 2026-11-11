// fd: open a file and keep it open, so you can find it in /proc/<pid>/fd.
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

int main()
{
    int fd = open("/etc/os-release", O_RDONLY);
    printf("pid %d got fd %d\n", getpid(), fd);
    sleep(30);
    close(fd);
}
