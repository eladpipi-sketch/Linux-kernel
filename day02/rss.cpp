// rss: memory you allocated but haven't touched isn't in RAM yet.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

void print_rss(const char* label)
{
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line))
        if (line.find("VmRSS:") == 0)
            printf("%-13s %s\n", label, line.c_str());
}

int main()
{
    const size_t size = 200 * 1024 * 1024;
    print_rss("start");
    char* buffer = (char*)malloc(size);
    print_rss("after malloc");
    memset(buffer, 1, size);
    print_rss("after memset");
    free(buffer);
}
