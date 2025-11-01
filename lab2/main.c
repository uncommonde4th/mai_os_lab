#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "array_sum.h"

#define PER_THREAD 1000
#define MAX_THREADS 16


/*typedef struct {
    int array_count;
    int array_size;
    double **arrays;
    double *result;
    int start_index;
    int end_index;
} ThreadArgs;*/

/*void *thread_func(void *arguments) {
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
}*/

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Использование: ./array_sum <количество_массивов> <размер_массивов> <количество_потоков>\n");
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
        arrays[i] = (double*)malloc(sizeof(double) * array_size);
        for (int j = 0; j < array_size; j++) {
            arrays[i][j] = rand() % 10000;
        }
    }

    struct timespec end, start;

    clock_gettime(CLOCK_MONOTONIC, &start);
    sequential_func(arrays, sequential_res, array_count, array_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double sequential_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t* threads = (pthread_t*)malloc(threads_count * sizeof(pthread_t));
    ThreadArgs* thread_args = (ThreadArgs*)malloc(threads_count * sizeof(ThreadArgs));
    
    for (int i = 0; i < threads_count; i++) {
        int el_per_thread = array_size / threads_count;

        thread_args[i].start_index = i * el_per_thread;
        thread_args[i].end_index = (i == threads_count - 1) ? array_size : (i + 1) * el_per_thread;
        thread_args[i].array_count = array_count;
        thread_args[i].array_size = array_size;
        thread_args[i].arrays = arrays;
        thread_args[i].result = parallel_res;

        pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double parallel_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
        
    int match = 1;
    for (int i = 0; i < array_size; i++) {
        if (sequential_res[i] != parallel_res[i]) {
            match = 0;
            break;
        }
    }
    /*
    printf("Количество массивов: %d\n", array_count);
    printf("Размер массивов: %d\n", array_size);
    printf("Количество потоков: %d\n", threads_count);
    */
    printf("Последовательная версия: %.2f мс\n", sequential_time);
    printf("Параллельная версия: %.2f мс\n", parallel_time);
    printf("Ускорение: %.2f раз\n", sequential_time / parallel_time);
    printf("Эффективность: %.5f\n", (sequential_time / parallel_time) / threads_count);
    // printf("Результаты: %s\n", match ? "совпадают" : "не совпадают");
    
    for (int i = 0; i < array_count; i++) {
        free(arrays[i]);
    }
    free(arrays);
    free(sequential_res);
    free(parallel_res);
    free(threads);
    free(thread_args);

    return 0;
}

