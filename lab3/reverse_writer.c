#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

#define BUF_SIZE 4096

typedef struct {
    uint32_t size;
    char data[BUF_SIZE - 4];
} block_t;

void reverse_chars(char *str, int len) {
    if (len <= 0) return;
    int last = len - 1;
    if (str[last] == '\n') last--;
    if (last < 0) return;
    int swap_count = (last + 1) / 2;
    for (int i = 0; i < swap_count; i++) {
        char temp = str[i];
        str[i] = str[last - i];
        str[last - i] = temp;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        const char err_msg[] = "Ошибка! Недостаточно аргументов\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        return 1;
    }

    int output_fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) {
        const char err_msg[] = "Ошибка! Не удалось открыть файл для записи\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        return 2;
    }

    int shm_fd = shm_open(argv[2], O_RDONLY, 0600);
    if (shm_fd == -1) {
        const char err_msg[] = "Ошибка! Не удалось открыть shared memory\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        close(output_fd);
        return 3;
    }

    block_t *shared_block = mmap(NULL, BUF_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_block == MAP_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось отобразить shared memory\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        close(shm_fd);
        close(output_fd);
        return 4;
    }

    sem_t *sem_ready = sem_open(argv[3], 0);
    if (sem_ready == SEM_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось открыть семафор ready\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        munmap(shared_block, BUF_SIZE);
        close(shm_fd);
        close(output_fd);
        return 5;
    }

    sem_t *sem_done = sem_open(argv[4], 0);
    if (sem_done == SEM_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось открыть семафор done\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        sem_close(sem_ready);
        munmap(shared_block, BUF_SIZE);
        close(shm_fd);
        close(output_fd);
        return 6;
    }

    char local_buffer[BUF_SIZE];

    while (1) {
        sem_wait(sem_done);
        uint32_t data_size = shared_block->size;
        if (data_size == 0) break;
        
        if (data_size > sizeof(local_buffer)) {
            data_size = sizeof(local_buffer) - 1;
        }
        memcpy(local_buffer, shared_block->data, data_size);
        
        reverse_chars(local_buffer, data_size);
        write(output_fd, local_buffer, data_size);
        
        if (data_size > 0 && local_buffer[data_size - 1] != '\n') {
            write(output_fd, "\n", 1);
        }
        
        sem_post(sem_ready);
    }

    close(output_fd);
    munmap(shared_block, BUF_SIZE);
    close(shm_fd);
    sem_close(sem_ready);
    sem_close(sem_done);
    return 0;
}
