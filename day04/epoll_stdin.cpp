// epoll_stdin: one thread waits for input with epoll, with a 3-second timeout.
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdio>

int main()
{
    int epoll_fd = epoll_create1(0);
    epoll_event interest = {};
    interest.events = EPOLLIN;
    interest.data.fd = STDIN_FILENO;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &interest);
    while (true) {
        epoll_event ready;
        int count = epoll_wait(epoll_fd, &ready, 1, 3000);
        if (count == 0) {
            puts("(nothing for 3 seconds)");
            continue;
        }
        char buffer[256];
        ssize_t bytes = read(ready.data.fd, buffer, sizeof(buffer));
        if (bytes <= 0)
            break;                      // Ctrl+D
        printf("got %zd bytes\n", bytes);
    }
    close(epoll_fd);
}
