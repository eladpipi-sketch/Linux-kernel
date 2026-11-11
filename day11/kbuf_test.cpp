#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include "kbuf.h"

int main()
{
    int fd = open("/dev/kbuf", O_RDWR);
    if (fd < 0) {
        perror("open /dev/kbuf");
        return 1;
    }
    printf("KBUF_IOC_USED = %#lx\n", (unsigned long)KBUF_IOC_USED);

    write(fd, "hello", 5);
    int used = 0;
    ioctl(fd, KBUF_IOC_USED, &used);
    printf("used after write: %d\n", used);

    ioctl(fd, KBUF_IOC_CLEAR);
    ioctl(fd, KBUF_IOC_USED, &used);
    printf("used after clear: %d\n", used);

    if (ioctl(fd, 0x1234) < 0)
        perror("ioctl 0x1234");
    close(fd);
}
