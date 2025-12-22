#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

typedef struct {
    int data[25];
    int count;
    int write_complete;
    int read_complete;
} SharedData;

#define SHM_SIZE sizeof(SharedData)
#define SHM_KEY 0x1234

pid_t Fork(void) {
    pid_t rv = fork();
    if (rv == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return rv;
}

pid_t Wait(int *wstatus) {
    pid_t pid = wait(wstatus);
    if (rv == -1) {
        perror("wait");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int Shmget(key_t key, size_t size, int shmflg) {
    int rv = shmget(key, size, shmflg);
    if (rv == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Shmctl(int shmid, int cmd, struct shmid_ds *buf) {
    int rv = shmctl(shmid, cmd, buf);
    if (rv == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    return rv;
}

void *Shmat(int shmid, const void *shmaddr, int shmflg) {
    void *rv = shmat(shmid, shmaddr, shmflg);
    if (rv == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Shmdt(const void *shmaddr) {
    int rv = shmdt(shmaddr);
    if (rv == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    return rv;
}

void writer_process() {
    printf("========== writer start ==========\n");

    int shmid = Shmget(SHM_KEY, SHM_SIZE, 0666);
    printf("writer: get shared memory success, ID: %d\n", shmid);
    
    SharedData *shared_data = (SharedData *)Shmat(shmid, NULL, 0);
    printf("writer: attach success\n");
    
    shared_data->count = 0;
    shared_data->write_complete = 0;
    shared_data->read_complete = 0;
    
    printf("\nwriter: start to write...\n");
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            shared_data->data[shared_data->count] = i * 100 + j;
            shared_data->count++;
        }
        
        printf("writer: write data %d, current number: %d\n", i + 1, shared_data->count);
        printf("writer: data: ");
        for (int j = 0; j < shared_data->count; j++) {
            printf("%d ", shared_data->data[j]);
        }
        printf("\n");
        
        sleep(1);
    }
    
    shared_data->write_complete = 1;
    printf("\nwriter: write done!\n");
    
    printf("writer: wait for reader...\n");
    while (!shared_data->read_complete) {
        sleep(1);
    }
    
    Shmdt(shared_data);
    printf("writer: detach success\n");
    
    printf("========== writer end ==========\n");
}

void reader_process() {
    printf("========== reader start ==========\n");
    
    int shmid = Shmget(SHM_KEY, SHM_SIZE, 0666);
    printf("reader: get shared memory success, ID: %d\n", shmid);
    
    SharedData *shared_data = (SharedData *)Shmat(shmid, NULL, 0);
    printf("reader: attach success\n");
    
    printf("\nreader: start to read...\n");
    
    int last_count = 0;
    while (1) {
        if (shared_data->count > last_count) {
            printf("reader: new data! current number: %d\n", shared_data->count);
            printf("reader: all data: ");
            for (int i = 0; i < shared_data->count; i++) {
                printf("%d ", shared_data->data[i]);
            }
            printf("\n");
            last_count = shared_data->count;
        }
        
        if (shared_data->write_complete && last_count == shared_data->count) {
            break;
        }
        
        sleep(1);
    }
    
    printf("\nreader: read end! %d datas\n", shared_data->count);
    
    shared_data->read_complete = 1;
    
    Shmdt(shared_data);
    printf("reader: detach success\n");
    
    printf("========== reader end ==========\n");
}

int main() {

    int shmid = Shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    printf("main process: create shared memory success, ID: %d\n", shmid);

    pid_t pid = Fork();
    
    if (pid == 0) {
        reader_process();
    } else {
        writer_process();
        Wait(NULL);
        Shmctl(shmid, IPC_RMID, NULL);
        printf("main process: remove shared memory success\n");
    }

    return 0;
}