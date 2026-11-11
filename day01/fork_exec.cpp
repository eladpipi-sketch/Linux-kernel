// fork_exec: the two steps every Unix program launch takes.
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>

int main()
{
    pid_t child = fork();               // now there are two copies of this program
    if (child == 0) {
        execlp("ls", "ls", "-l", nullptr);  // the child becomes ls
        perror("execlp");               // reached only if exec failed
        return 1;
    }
    int status = 0;
    waitpid(child, &status, 0);         // the parent sleeps until the child exits
    printf("child %d exited with %d\n", child, WEXITSTATUS(status));
}
