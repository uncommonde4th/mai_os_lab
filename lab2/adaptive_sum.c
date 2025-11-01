#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "array_sum.h"

#define MIN_WORK_PER_THREAD 1000

void adaptive_sum(double **arrays, double *result, int array_count, int array_size, int *chosen_threads) {
    int total_work = array_count * array_size;
    int available_cores = sysconf(_SC_NPROCESSORS_ONLN);
    
    if (total_work < MIN_WORK_PER_THREAD) {
        *chosen_threads = 1;
        sequential_func(arrays, result, array_count, array_size);
    } else {
        int threads_to_use = total_work / MIN_WORK_PER_THREAD;
        
        if (threads_to_use < 1) { threads_to_use = 1; }
        if (threads_to_use > available_cores) { threads_to_use = available_cores; }
        if (threads_to_use > array_size) { threads_to_use = array_size; }
        
        *chosen_threads = threads_to_use;
        
        pthread_t* threads = (pthread_t*)malloc(threads_to_use * sizeof(pthread_t));
        ThreadArgs* thread_args = (ThreadArgs*)malloc(threads_to_use * sizeof(ThreadArgs));
        
        int el_per_thread = array_size / threads_to_use;
        
        for (int i = 0; i < threads_to_use; i++) {
            thread_args[i].start_index = i * el_per_thread;
            thread_args[i].end_index = (i == threads_to_use - 1) ? array_size : (i + 1) * el_per_thread;
            thread_args[i].array_count = array_count;
            thread_args[i].array_size = array_size;
            thread_args[i].arrays = arrays;
            thread_args[i].result = result;
            
            pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
        }
        
        for (int i = 0; i < threads_to_use; i++) {
            pthread_join(threads[i], NULL);
        }
        
        free(threads);
        free(thread_args);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Использование: ./adaptive_sum <количество_массивов> <размер_массивов>\n");
        return 1;
    }

    int array_count = atoi(argv[1]);
    int array_size = atoi(argv[2]);

    double **arrays = (double**)malloc(sizeof(double*) * array_count);
    double *result = (double*)malloc(array_size * sizeof(double));

    srand(time(NULL));
    for (int i = 0; i < array_count; i++) {
        arrays[i] = (double*)malloc(sizeof(double) * array_size);
        for (int j = 0; j < array_size; j++) {
            arrays[i][j] = rand() % 10000;
        }
    }

    struct timespec end, start;
    int chosen_threads;

    clock_gettime(CLOCK_MONOTONIC, &start);
    adaptive_sum(arrays, result, array_count, array_size, &chosen_threads);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double adaptive_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("Адаптивная стратегия:\n");
    printf("Количество массивов: %d\n", array_count);
    printf("Размер массивов: %d\n", array_size);
    printf("Выбранная стратегия: %s\n", chosen_threads == 1 ? "последовательная" : "параллельная");
    printf("Количество потоков: %d\n", chosen_threads);
    printf("Время выполнения: %.2f мс\n", adaptive_time);
    
    for (int i = 0; i < array_count; i++) {
        free(arrays[i]);
    }
    free(arrays);
    free(result);

    return 0;
}
