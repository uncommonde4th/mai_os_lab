#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>


typedef struct {
    int array_count;
    int array_size;
    double **arrays;
    double *result;
    int start_index;
    int end_index;
} ThreadArgs;

void *thread_func(void *arguments) {
    ThreadArgs *args = (ThreadArgs*)arguments;

    for (int i = args->start_index; i < args->end_index; i++) {
        double sum = 0.0;
        for (int j = 0; j < args->array_count; j++) {
            sum += args->arrays[j][i];
        }
        args->result[i] = sum;
    }

    return NULL;
}

void sequential_func(double **arrays, double *result, int array_count, int array_size) {
    for (int i = 0; i < array_size; i++) {
        result[i] = 0.0;
        for (int j = 0; j < array_count; j++) {
            result[i] += arrays[j][i];
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Использование: ./main <количество_массивов> <размер_массивов> <количество_потоков>\n");
        return 1;
    }

    int array_count = atoi(argv[1]);
    int array_size = atoi(argv[2]);
    int threads_count = atoi(argv[3]);

    double **arrays = (double**)malloc(sizeof(double*) * array_count);
    double *sequential_res = (double*)malloc(array_size * sizeof(double));
    double *parallel_res = (double*)malloc(array_size * sizeof(double));

    srand(time(NULL));
    for (int i = 0; i < array_count; i++) {
        arrays[i] = malloc(sizeof(double) * array_size);
        for (int j = 0; j < array_size; j++) {
            arrays[i][j] = rand() % 10000;
        }
    }

    struct timespec end, start;
