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
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_data));
    shared_data *shm = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shm->reader_count = 0;
    shm->writer_count = 0;
    shm->data = 0;

    sem_t *rc_mutex = sem_open(SEM_RC_MUTEX, O_CREAT, 0666, 1);
    sem_t *wc_mutex = sem_open(SEM_WC_MUTEX, O_CREAT, 0666, 1);
    sem_t *r_ctrl = sem_open(SEM_R_CTRL, O_CREAT, 0666, 1);
    sem_t *rsem = sem_open(SEM_RSEM, O_CREAT, 0666, 1);
    sem_t *wsem = sem_open(SEM_WSEM, O_CREAT, 0666, 1);

    for (int i = 0; i < 50; ++i) {
        sem_wait(wc_mutex);
        shm->writer_count++;
        if (shm->writer_count == 1) {
            sem_wait(rsem);
        }
        sem_post(wc_mutex);
        shm->data += 10;
        printf("Writer writes: %d\n", shm->data);
        usleep(200000);
        sem_wait(wc_mutex);
        shm->writer_count--;
        if (shm->writer_count == 0) {
            sem_post(rsem);
        }
        sem_post(wc_mutex);

        usleep(100000);
    }

    munmap(shm, sizeof(shared_data));
    shm_unlink(SHM_NAME);
    sem_close(rc_mutex);
    sem_close(wc_mutex);
    sem_close(r_ctrl);
    sem_close(rsem);
    sem_close(wsem);
    sem_unlink(SEM_RC_MUTEX);
    sem_unlink(SEM_WC_MUTEX);
    sem_unlink(SEM_R_CTRL);
    sem_unlink(SEM_RSEM);
    sem_unlink(SEM_WSEM);
    return 0;
}