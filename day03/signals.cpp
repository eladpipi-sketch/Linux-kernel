// signals: catch SIGINT and SIGTERM, and exit cleanly.
#include <unistd.h>
#include <csignal>
#include <cstdio>

volatile sig_atomic_t received = 0;

void on_signal(int number) { received = number; }

int main()
{
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    printf("pid %d: press Ctrl+C, or run   kill -TERM %d\n", getpid(), getpid());
    while (received == 0)
        sleep(1);
    printf("got signal %d, exiting cleanly\n", (int)received);
}
