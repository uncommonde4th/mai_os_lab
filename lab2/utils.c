#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "array_sum.h"



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
