// deadlock: two threads take two locks in opposite orders.
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

std::mutex first;
std::mutex second;

void lock_first_then_second()
{
    std::lock_guard<std::mutex> a(first);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> b(second);
}

void lock_second_then_first()
{
    std::lock_guard<std::mutex> b(second);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> a(first);
}

int main()
{
    printf("pid %d\n", getpid());
    std::thread t1(lock_first_then_second);
    std::thread t2(lock_second_then_first);
    t1.join();
    t2.join();
    puts("done");                       // never printed
}
