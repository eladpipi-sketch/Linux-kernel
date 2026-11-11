// address_space: where code, globals, heap and stack live. Run it in the
// background and read /proc/<pid>/maps while it sleeps.
#include <unistd.h>
#include <cstdio>

int global_value = 1;
void some_function() {}

int main()
{
    int local_value = 2;
    int* heap_value = new int(3);
    printf("pid    %d\n", getpid());
    printf("code   %p\n", (void*)&some_function);
    printf("global %p\n", (void*)&global_value);
    printf("heap   %p\n", (void*)heap_value);
    printf("stack  %p\n", (void*)&local_value);
    sleep(30);
    delete heap_value;
}
