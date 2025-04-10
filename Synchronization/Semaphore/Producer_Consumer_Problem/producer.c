#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>

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
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data));
    shared_data *shm_ptr = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    shm_ptr->in = 0;
    shm_ptr->out = 0;

    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, BUFFER_SIZE);
    sem_t *full = sem_open(SEM_FULL, O_CREAT, 0666, 0);

    for (int i = 0; i < 10; ++i) {
        sem_wait(empty);
        sem_wait(mutex);

        shm_ptr->buffer[shm_ptr->in] = i;
        printf("Produced: %d\n", i);
        shm_ptr->in = (shm_ptr->in + 1) % BUFFER_SIZE;

        sem_post(mutex);
        sem_post(full);

        sleep(1);
    }

    return 0;
}