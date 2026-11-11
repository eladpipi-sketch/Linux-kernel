// writes: 1,000 log lines, with or without fsync() after each one.
//   ./writes        write() only
//   ./writes sync   write() + fsync() every time
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>

int main(int argc, char**)
{
    bool use_fsync = argc > 1;
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char line[] = "one line of log\n";
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; ++i) {
        write(fd, line, sizeof(line) - 1);
        if (use_fsync)
            fsync(fd);
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    printf("%lld ms\n", ms);
    close(fd);
}
