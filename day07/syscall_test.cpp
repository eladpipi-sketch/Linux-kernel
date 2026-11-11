#include <unistd.h>
#include <sys/syscall.h>
#include <cerrno>
#include <cstdio>
#include <cstring>

int main()
{
    long result = syscall(473, 21);  // 473 = the number you gave hello in syscall_64.tbl
    if (result < 0)
        printf("syscall 473 failed: %s\n", strerror(errno));
    else
        printf("hello(21) returned %ld\n", result);
}
