#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main() {
        int pipe1[2];
        int pipe2[2];
        pid_t pid1;
        pid_t pid2;
        char input[256];
        int lineNum = 1;

        pipe(pipe1);
        pipe(pipe2);

        pid1 = fork();
        if (pid1 == 0) {
                close(pipe1[1]);
                dup2(pipe1[0], STDIN_FILENO);
                close(pipe1[0]);
                execlp("./child1", "child1", NULL);
                _exit(1);
        }

        pid2 = fork();
        if (pid2 == 0) {
                close(pipe2[1]);
                dup2(pipe2[0], STDIN_FILENO);
                close(pipe2[0]);
                execlp("./child2", "child2", NULL);
                _exit(1);
        }

        close(pipe1[0]);
        close(pipe2[0]);

        char c;
        int i = 0;
        while (1) {
                ssize_t n = read(STDIN_FILENO, &c, 1);
                if (n == 0) break;
                input[i++] = c;
                if (c == '\n'){
                        if (lineNum % 2 == 1) {
                                write(pipe1[1], input, i);
                        } else {
                                write(pipe2[1], input, i);
                        }
                        i = 0;
                        lineNum++;
                }
        }
        close(pipe1[1]);
        close(pipe2[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);

        return 0;
}
