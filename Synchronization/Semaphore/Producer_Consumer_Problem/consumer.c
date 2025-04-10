#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_NAME "/myshm"
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"
#define BUFFER_SIZE 3

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_data;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    shared_data *shm_ptr = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *mutex = sem_open(SEM_MUTEX, 0);
    sem_t *empty = sem_open(SEM_EMPTY, 0);
    sem_t *full = sem_open(SEM_FULL, 0);

    for (int i = 0; i < 10; ++i) {
        sem_wait(full);
        sem_wait(mutex);

        int item = shm_ptr->buffer[shm_ptr->out];
        printf("Consumed: %d\n", item);
        shm_ptr->out = (shm_ptr->out + 1) % BUFFER_SIZE;

        sem_post(mutex);
        sem_post(empty);

        sleep(2);
    }

    munmap(shm_ptr, sizeof(shared_data));
    shm_unlink(SHM_NAME);

    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);

    return 0;
}