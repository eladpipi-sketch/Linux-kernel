// box: a container in 50 lines. clone() starts a shell in new UTS, PID, mount
// and network namespaces, with a fresh /proc, inside the cgroup /sys/fs/cgroup/box.
#include <sched.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

static char child_stack[1024 * 1024];

static int child(void*)
{
    sethostname("box", 3);                                // only our UTS namespace sees it
    mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr);  // keep our mounts to ourselves
    mount("proc", "/proc", "proc", 0, nullptr);           // a /proc for the new PID namespace

    FILE* procs = fopen("/sys/fs/cgroup/box/cgroup.procs", "w");
    if (procs) {
        fputs("0", procs);                                // 0 = "move the writer"
        fclose(procs);
    } else {
        perror("join /sys/fs/cgroup/box");
    }

    execl("/bin/sh", "sh", nullptr);
    perror("execl");
    return 1;
}

int main()
{
    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD;
    pid_t pid = clone(child, child_stack + sizeof(child_stack), flags, nullptr);
    if (pid < 0) {
        perror("clone (are you root?)");
        return 1;
    }
    printf("box: started pid %d on the outside\n", pid);
    waitpid(pid, nullptr, 0);
    printf("box: done\n");
}
