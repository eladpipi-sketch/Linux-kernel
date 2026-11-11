// One system call, so you can watch it from inside the kernel.
#include <unistd.h>
#include <cstdio>

int main()
{
    printf("my parent is %d\n", getppid());
}
