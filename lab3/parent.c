#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>

#define BUF_SIZE 4096

char SHM_NAME_1[] = "shm_a";
char SHM_NAME_2[] = "shm_b";
char SEM_READY_1[] = "sem_rdy_a";
char SEM_READY_2[] = "sem_rdy_b";
char SEM_DONE_1[] = "sem_done_a";
char SEM_DONE_2[] = "sem_done_b";

typedef struct {
    uint32_t size;
    char data[BUF_SIZE - 4];
} block_t;

void *area1 = NULL;
void *area2 = NULL;
sem_t *sem_ready1 = NULL;
sem_t *sem_done1 = NULL;
sem_t *sem_ready2 = NULL;
sem_t *sem_done2 = NULL;

void cleanup();
int setup_resources(char *shm_name, char *sem_ready_name, char *sem_done_name, void **shm_ptr, sem_t **sem_ready, sem_t **sem_done);

int main(int argc, char* argv[]) {
    char base_path[1024];
    {
        ssize_t path_len = readlink("/proc/self/exe", base_path, sizeof(base_path) - 1);
        if (path_len == -1) {
            const char err_msg[] = "Ошибка! Не удалось получить путь к программе\n";
            write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
            exit(EXIT_FAILURE);
        }
        while (base_path[path_len] != '/') --path_len;
        base_path[path_len] = '\0';
    }

    char output1[512];
    char output2[512];
    
    ssize_t len1 = read(STDIN_FILENO, output1, sizeof(output1) - 1);
    if (len1 > 0) {
        if (output1[len1 - 1] == '\n') output1[--len1] = '\0';
        else output1[len1] = '\0';
    } else {
        const char err_msg[] = "Ошибка! Не удалось прочитать имя первого файла\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        exit(EXIT_FAILURE);
    }
    
    ssize_t len2 = read(STDIN_FILENO, output2, sizeof(output2) - 1);
    if (len2 > 0) {
        if (output2[len2 - 1] == '\n') output2[--len2] = '\0';
        else output2[len2] = '\0';
    } else {
        const char err_msg[] = "Ошибка! Не удалось прочитать имя второго файла\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (setup_resources(SHM_NAME_1, SEM_READY_1, SEM_DONE_1, &area1, &sem_ready1, &sem_done1) != 0
        || setup_resources(SHM_NAME_2, SEM_READY_2, SEM_DONE_2, &area2, &sem_ready2, &sem_done2) != 0) {
        cleanup();
        exit(EXIT_FAILURE);
    }

    pid_t proc1 = fork();
    if (proc1 == 0) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, "reverse_writer");
        char *const proc_args[] = {"reverse_writer", output1, SHM_NAME_1, SEM_READY_1, SEM_DONE_1, NULL};
        execv(full_path, proc_args);
        const char err_msg[] = "Ошибка! Не удалось запустить reverse_writer\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        cleanup();
        exit(EXIT_FAILURE);
    } else if (proc1 == -1) {
        const char err_msg[] = "Ошибка! Не удалось создать первый процесс\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        cleanup();
        exit(EXIT_FAILURE);
    }
    
    pid_t proc2 = fork();
    if (proc2 == 0) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, "reverse_writer");
        char *const proc_args[] = {"reverse_writer", output2, SHM_NAME_2, SEM_READY_2, SEM_DONE_2, NULL};
        execv(full_path, proc_args);
        const char err_msg[] = "Ошибка! Не удалось запустить reverse_writer\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        cleanup();
        exit(EXIT_FAILURE);
    } else if (proc2 == -1) {
        const char err_msg[] = "Ошибка! Не удалось создать второй процесс\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        cleanup();
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    unsigned line_num = 1;
    ssize_t bytes_read;
    
    while (1) {
        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            const char err_msg[] = "Ошибка! Не удалось прочитать входные данные\n";
            write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
            cleanup();
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0) break;

        block_t *target_block;
        sem_t *ready_sem;
        sem_t *done_sem;

        if (line_num % 2 == 1) {
            target_block = (block_t *)area1;
            ready_sem = sem_ready1;
            done_sem = sem_done1;
        } else {
            target_block = (block_t *)area2;
            ready_sem = sem_ready2;
            done_sem = sem_done2;
        }
        
        sem_wait(ready_sem);
        target_block->size = (uint32_t)bytes_read;
        memcpy(target_block->data, buffer, bytes_read);
        sem_post(done_sem);
        
        line_num++;
    }

    if (sem_wait(sem_ready1) != -1) {
        ((block_t *)area1)->size = 0;
        sem_post(sem_done1);
    }
    if (sem_wait(sem_ready2) != -1) {
        ((block_t *)area2)->size = 0;
        sem_post(sem_done2);
    }

    waitpid(proc1, NULL, 0);
    waitpid(proc2, NULL, 0);
    cleanup();
    return 0;
}

int setup_resources(char *shm_name, char *sem_ready_name, char *sem_done_name, void **shm_ptr, sem_t **sem_ready, sem_t **sem_done) {
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (shm_fd == -1) {
        const char err_msg[] = "Ошибка! Не удалось создать shared memory\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        return -1;
    }

    if (ftruncate(shm_fd, BUF_SIZE) == -1) {
        const char err_msg[] = "Ошибка! Не удалось установить размер shared memory\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        close(shm_fd);
        return -1;
    }

    *shm_ptr = mmap(NULL, BUF_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (*shm_ptr == MAP_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось отобразить shared memory\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        close(shm_fd);
        return -1;
    }
    
    *sem_ready = sem_open(sem_ready_name, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
    if (*sem_ready == SEM_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось создать семафор ready\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        munmap(*shm_ptr, BUF_SIZE);
        close(shm_fd);
        return -1;
    }

    *sem_done = sem_open(sem_done_name, O_RDWR | O_CREAT | O_TRUNC, 0600, 0);
    if (*sem_done == SEM_FAILED) {
        const char err_msg[] = "Ошибка! Не удалось создать семафор done\n";
        write(STDERR_FILENO, err_msg, sizeof(err_msg) - 1);
        sem_close(*sem_ready);
        munmap(*shm_ptr, BUF_SIZE);
        close(shm_fd);
        return -1;
    }

    close(shm_fd);
    return 0;
}

void cleanup() {
    if (area1 != NULL) munmap(area1, BUF_SIZE);
    if (area2 != NULL) munmap(area2, BUF_SIZE);

    if (sem_ready1 != NULL) sem_close(sem_ready1);
    if (sem_done1 != NULL) sem_close(sem_done1);
    if (sem_ready2 != NULL) sem_close(sem_ready2);
    if (sem_done2 != NULL) sem_close(sem_done2);

    shm_unlink(SHM_NAME_1);
    shm_unlink(SHM_NAME_2);
    sem_unlink(SEM_READY_1);
    sem_unlink(SEM_DONE_1);
    sem_unlink(SEM_READY_2);
    sem_unlink(SEM_DONE_2);
}
