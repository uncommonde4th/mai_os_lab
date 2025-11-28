#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "./cos_derivative.h"
#include "./convert.h"

static void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

static void write_float(float num) {
    char buff[32];
    snprintf(buff, sizeof(buff), "%f", num);
    write_str(buff);
}

int main() {
    int run = 1;
    char input_buff[64];

    while (run) {
        int cmd = 0;
        
        write_str("Доступные команды:\n");
        write_str("\t1 - Расчет производной функции cos(x) в точке a с приращением dx.\n");
        write_str("\t2 - Перевод числа x из десятичной системы счисления в другую.\n");
        write_str("\t-1 - Выход.\n");
        write_str("Команда: ");

        read(STDIN_FILENO, input_buff, sizeof(input_buff));
        cmd = atoi(input_buff);
        
        if (cmd == -1) { run = 0; }
        else if (cmd == 1) {
            float a, dx;

            write_str("Введите числа:\n");
            write_str("\ta: ");
            read(STDIN_FILENO, input_buff, sizeof(input_buff));
            a = atof(input_buff);
            write_str("\tdx: ");
            read(STDIN_FILENO, input_buff, sizeof(input_buff));
            dx = atof(input_buff);

            float result = cos_derivative(a, dx);
            write_str("\nРезультат: ");
            write_float(result);
            write_str("\n");
        } else if (cmd == 2) {
            int x;

            write_str("Введите число:\n");
            write_str("\tx: ");
            read(STDIN_FILENO, input_buff, sizeof(input_buff));
            x = atoi(input_buff);

            char *result = convert(x);
            write_str("\nРезультат: ");
            write_str(result);
            write_str("\n");
        } else {
            write_str("Неизвестная команда");
        }
    }

    return 0;
}




        
