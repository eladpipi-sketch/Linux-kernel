// threads: three threads that sleep, so you can look at them in /proc.
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <vector>

int main()
{
    printf("pid %d\n", getpid());
    std::vector<std::thread> workers;
    for (int i = 0; i < 3; ++i)
        workers.emplace_back([] { sleep(30); });
    for (auto& worker : workers)
        worker.join();
}
