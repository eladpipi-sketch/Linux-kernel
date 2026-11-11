// race: two threads add one million each to a shared counter.
//   ./race          no lock: updates get lost
//   ./race mutex    a std::mutex around ++counter
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

long counter = 0;
std::mutex counter_mutex;
bool use_mutex = false;

void add_million()
{
    for (int i = 0; i < 1000000; ++i) {
        if (use_mutex) {
            std::lock_guard<std::mutex> lock(counter_mutex);
            ++counter;
        } else {
            ++counter;
        }
    }
}

int main(int argc, char* argv[])
{
    use_mutex = argc > 1 && strcmp(argv[1], "mutex") == 0;
    std::thread first(add_million);
    std::thread second(add_million);
    first.join();
    second.join();
    printf("counter = %ld (expected 2000000)\n", counter);
}
