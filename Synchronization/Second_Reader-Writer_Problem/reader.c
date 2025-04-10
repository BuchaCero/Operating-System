#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_NAME "/rw_shm"
#define SEM_RC_MUTEX "/rw_rc_mutex"
#define SEM_WC_MUTEX "/rw_wc_mutex"
#define SEM_R_CTRL "/rw_r_ctrl"
#define SEM_RSEM "/rw_rsem"
#define SEM_WSEM "/rw_wsem"

typedef struct {
    int reader_count;
    int writer_count;
    int data;
} shared_data;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *rc_mutex = sem_open(SEM_RC_MUTEX, 0);
    sem_t *wc_mutex = sem_open(SEM_WC_MUTEX, 0);
    sem_t *r_ctrl = sem_open(SEM_R_CTRL, 0);
    sem_t *rsem = sem_open(SEM_RSEM, 0);
    sem_t *wsem = sem_open(SEM_WSEM, 0);

    for (int i = 0; i < 50; ++i) {
        sem_wait(r_ctrl);
        sem_wait(rsem);
        sem_wait(rc_mutex);
        shm->reader_count++;
        if (shm->reader_count == 1) {
            sem_wait(wsem);
        }
        sem_post(rc_mutex);
        sem_post(rsem);
        sem_post(r_ctrl);
        printf("Reader reads: %d\n", shm->data);
        usleep(100000);
    
        sem_wait(rc_mutex);
        shm->reader_count--;
        if (shm->reader_count == 0) {
            sem_post(wsem);
        }
        sem_post(rc_mutex);
    
        usleep(100000);
    }

    munmap(shm, sizeof(shared_data));
    return 0;
}

