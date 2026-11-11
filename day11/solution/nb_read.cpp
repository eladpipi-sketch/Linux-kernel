// nb_read: read /dev/kbuf once, without waiting for data.
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

int main()
{
    int fd = open("/dev/kbuf", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open /dev/kbuf");
        return 1;
    }
    char buf[64];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
    } else {
        buf[n] = '\0';
        printf("read %zd bytes: %s", n, buf);
    }
    close(fd);
}
