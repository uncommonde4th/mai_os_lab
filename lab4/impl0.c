#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

void write_str(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

float cos_derivative(float a, float dx) {
    if (dx == 0) {
        write_str("Ошибка! dx не должен равняться нулю.");
        return NAN;
    }

    float result = (cos(a + dx) - cos(a)) / dx;

    return result;
}

char *convert(int x) {
    if (x < 0) {
        write_str("Ошибка! convert не поддерживает отрицательные числа.");
        return NULL;
    }

    if (x == 0) {
        char *result = (char *)malloc(sizeof(char) * 2);
        if (result == NULL) {
            write_str("Ошибка! Не удалось выделить память под результат convert.");
            return NULL;
        }
        result[0] = '0';
        result[1] = '\0';

        return result;
    }

    int length = 0;   
    long temp = x;

    while (temp > 0) {
        temp /= 2;
        length++;
    }

    char *result = calloc(length + 1, sizeof(char));
    if (result == NULL) {
        write_str("Ошибка! Не удалось выделить память под результат convert.");
        return NULL;
    }

    for (int i = length - 1; i >= 0; i--) {
        result[i] = (x % 2) + '0';
        x /= 2;
    }

    return result;
}
