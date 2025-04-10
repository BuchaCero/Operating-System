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
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data));
    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shm->reader_count = 0;
    shm->data = 0;

    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *rw_mutex = sem_open(SEM_RW, O_CREAT, 0666, 1);

    for (int i = 0; i < 50; ++i) {
        sem_wait(rw_mutex);
        shm->data += 10;
        printf("Writer writes: %d\n", shm->data);
        usleep(200000);
        sem_post(rw_mutex);

        usleep(100000);
    }

    munmap(shm, sizeof(shared_data));
    shm_unlink(SHM_NAME);
    sem_close(mutex); sem_close(rw_mutex);
    sem_unlink(SEM_MUTEX); sem_unlink(SEM_RW);
    return 0;
}
