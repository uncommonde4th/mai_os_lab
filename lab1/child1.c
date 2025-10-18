#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void reverse(char *s, int len){
        int end;
        if (s[len - 1] == '\n') {
                end = len - 2;
        } else {
                end = len - 1;
        }
        for (int i =0; i < end / 2; i++) {
                char tmp = s[i];
                s[i] = s[end - i];
                s[end - i] = tmp;
        }
}

int main() {
        int fd = open("child1_out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) _exit(2);

        char buf[256];
        int i = 0;
        char c;

        while (read(STDIN_FILENO, &c, 1) > 0) {
                buf[i++] = c;
                if (c == '\n'){
                        reverse(buf, i);
                        write(fd, buf, i);
                        i = 0;
                }
        }
        close(fd);
        return 0;
}
