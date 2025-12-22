#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

const int NUM_PROCESSES = 6;
const int TOTAL_PAGES = 3;
const int TOTAL_IOS = 3;

sem_t resource_mutex;
sem_t wait_sem;

int available_pages = TOTAL_PAGES;
int available_ios = TOTAL_IOS;
int waiting_count = 0;

typedef struct {
    int id;
    int iterations;
} process_data_t;

void posix_error(int code, char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(EXIT_FAILURE);
}

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
    int rv = pthread_create(thread, attr, start_routine, arg);
    if (rv != 0) {
        posix_error(rv, "pthread_create");
    }
}

void Pthread_join(pthread_t thread, void **retval) {
    int rv = pthread_join(thread, retval);
    if (rv != 0) {
        posix_error(rv, "pthread_join");
    }
}

void Sem_init(sem_t *sem, int pshared, unsigned int value) {
    int rv = sem_init(sem, pshared, value);
    if (rv == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}

void use_resources(int pid, int iteration) {
    printf("Process P%d (iteration %d) using 1 page and 1 I/O device\n", 
           pid + 1, iteration + 1);
}

bool try_acquire_resources(int pid, int iteration) {
    bool acquired = false;
    
    sem_wait(&resource_mutex);
    
    if (available_pages >= 1 && available_ios >= 1) {
        available_pages--;
        available_ios--;
        printf("Process P%d (iteration %d) acquired: 1 page and 1 I/O device\n",
               pid + 1, iteration + 1);
        printf("  Remaining: %d pages, %d I/O devices\n", 
               available_pages, available_ios);
        acquired = true;
    } else {
        waiting_count++;
        printf("Process P%d (iteration %d) waiting. Need: 1 page and 1 I/O device\n",
               pid + 1, iteration + 1);
        printf("  Available: %d pages, %d I/O devices\n", 
               available_pages, available_ios);
    }
    
    sem_post(&resource_mutex);
    return acquired;
}

void release_resources(int pid, int iteration) {
    sem_wait(&resource_mutex);
    
    available_pages++;
    available_ios++;
    printf("Process P%d (iteration %d) released: 1 page and 1 I/O device\n",
           pid + 1, iteration + 1);
    
    if (waiting_count > 0 && available_pages >= 1 && available_ios >= 1) {
        printf("  Waking up a waiting process\n");
        printf("  Remaining: %d pages, %d I/O devices\n", 
               available_pages, available_ios);
        available_pages--;
        available_ios--;
        waiting_count--;
        sem_post(&wait_sem);
    } else {
        printf("  Remaining: %d pages, %d I/O devices\n", 
               available_pages, available_ios);
    }
    
    sem_post(&resource_mutex);
}

void* process_function(void* arg) {
    process_data_t* data = (process_data_t*)arg;
    int pid = data->id;
    
    for (int iter = 0; iter < data->iterations; iter++) {
        bool acquired = false;
        
        while (!acquired) {
            acquired = try_acquire_resources(pid, iter);
            
            if (!acquired) {
                sem_wait(&wait_sem);
                sem_wait(&resource_mutex);
                printf("Process P%d (iteration %d) acquired resources after waiting\n",
                       pid + 1, iter + 1);
                printf("  Remaining: %d pages, %d I/O devices\n", 
                       available_pages, available_ios);
                sem_post(&resource_mutex);
                acquired = true;
            }
        }
        
        use_resources(pid, iter);
        
        release_resources(pid, iter);
    }
    
    free(data);
    return NULL;
}

void run_processes(int iterations_per_process) {
    pthread_t threads[NUM_PROCESSES];
    
    printf("=== Starting %d processes with %d iterations each ===\n", 
           NUM_PROCESSES, iterations_per_process);
    printf("System resources: %d pages, %d I/O devices\n\n", TOTAL_PAGES, TOTAL_IOS);
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        process_data_t* data = malloc(sizeof(process_data_t));
        data->id = i;
        data->iterations = iterations_per_process;
        
        Pthread_create(&threads[i], NULL, process_function, data);
    }
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        Pthread_join(threads[i], NULL);
    }
    
    printf("\n=== All processes completed ===\n");
    printf("Final resources: %d pages, %d I/O devices\n", 
           available_pages, available_ios);
}

int main() {
    printf("========================================\n");
    printf("Process Multi-Resource Allocation System\n");
    printf("========================================\n\n");
    
    Sem_init(&resource_mutex, 0, 1);
    Sem_init(&wait_sem, 0, 0);
    
    run_processes(3);
    
    return 0;
}
