#ifndef ARRAY_SUM_H
#define ARRAY_SUM_H

typedef struct {
    int array_count;
    int array_size;
    double **arrays;
    double *result;
    int start_index;
    int end_index;
} ThreadArgs;

void *thread_func(void *arguments);
void sequential_func(double **arrays, double *result, int array_count, int array_size);

#endif
