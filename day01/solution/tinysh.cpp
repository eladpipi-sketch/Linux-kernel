// tinysh: a shell in 40 lines. fork() makes a child, execvp() turns it into
// the program, waitpid() waits for it. cd must run in the shell itself.
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main()
{
    while (true) {
        printf("tinysh$ ");
        fflush(stdout);
        std::string line;
        if (!std::getline(std::cin, line))
            break;

        std::istringstream words(line);
        std::vector<std::string> args;
        for (std::string word; words >> word;)
            args.push_back(word);
        if (args.empty())
            continue;
        if (args[0] == "exit")
            break;
        if (args[0] == "cd") {
            if (chdir(args.size() > 1 ? args[1].c_str() : "/") != 0)
                perror("cd");
            continue;
        }

        std::vector<char*> argv;
        for (auto& arg : args)
            argv.push_back(&arg[0]);
        argv.push_back(nullptr);

        pid_t child = fork();
        if (child == 0) {
            execvp(argv[0], argv.data());
            perror(argv[0]);            // only if exec failed
            _exit(127);
        }
        int status = 0;
        waitpid(child, &status, 0);
        printf("[pid %d exited with %d]\n", child, WEXITSTATUS(status));
    }
}
