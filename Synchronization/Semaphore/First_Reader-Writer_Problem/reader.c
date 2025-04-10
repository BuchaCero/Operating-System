#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_NAME "/rw_shm"
#define SEM_MUTEX "/rw_mutex"
#define SEM_RW "/rw_rwlock"

typedef struct {
    int reader_count;
    int data;
} shared_data;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *mutex = sem_open(SEM_MUTEX, 0);
    sem_t *rw_mutex = sem_open(SEM_RW, 0);

    for (int i = 0; i < 50; ++i) {
        sem_wait(mutex);
        shm->reader_count++;
        if (shm->reader_count == 1) {
            sem_wait(rw_mutex);
        }
        sem_post(mutex);

        printf("Reader reads: %d\n", shm->data);
        usleep(100000);

        sem_wait(mutex);
        shm->reader_count--;
        if (shm->reader_count == 0) {
            sem_post(rw_mutex);
        }
        sem_post(mutex);

        usleep(100000);
    }

    munmap(shm, sizeof(shared_data));
    return 0;
}
