#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


void reverse(char *s, int len){
        if (len <= 0) return;
        int end = len - 1;
        if (s[end] == '\n') {
                end--;
        }
        if (end < 0) return;
        int swaps = (end + 1) / 2;
        for (int i = 0; i < swaps; i++) {
                char tmp = s[i];
                s[i] = s[end - i];
                s[end - i] = tmp;
        }
}

int main(int argc, char *argv[]) {
        if (argc < 2) _exit(1);

        int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
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

        if (i > 0) {
                reverse(buf, i);
                write(fd, buf, i);
        }
        close(fd);
        return 0;
}
