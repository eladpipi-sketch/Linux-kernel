// syscall_cost: one system call against one ordinary function call.
#include <unistd.h>
#include <chrono>
#include <cstdio>

__attribute__((noinline)) int plain_function(int x) { return x + 1; }

template <typename F> long long ns_per_call(F call)
{
    const int calls = 1000000;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < calls; ++i)
        call();
    auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() / calls;
}

int main()
{
    printf("%lld ns per getppid()\n", ns_per_call([] { getppid(); }));
    printf("%lld ns per function call\n", ns_per_call([] { plain_function(1); }));
}
