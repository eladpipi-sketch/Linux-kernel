// race, with a third mode:
//   ./race          no lock: updates get lost
//   ./race mutex    a std::mutex: sleeps in the kernel (futex) when contended
//   ./race spin     a spinlock: never enters the kernel, burns CPU instead
#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

long counter = 0;
std::mutex counter_mutex;
std::atomic_flag spin_lock = ATOMIC_FLAG_INIT;
const char* mode = "none";

void add_million()
{
    for (int i = 0; i < 1000000; ++i) {
        if (strcmp(mode, "mutex") == 0) {
            std::lock_guard<std::mutex> lock(counter_mutex);
            ++counter;
        } else if (strcmp(mode, "spin") == 0) {
            while (spin_lock.test_and_set(std::memory_order_acquire))
                ;                       // spin until the holder clears it
            ++counter;
            spin_lock.clear(std::memory_order_release);
        } else {
            ++counter;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
        mode = argv[1];
    std::thread first(add_million);
    std::thread second(add_million);
    first.join();
    second.join();
    printf("%s: counter = %ld (expected 2000000)\n", mode, counter);
}
