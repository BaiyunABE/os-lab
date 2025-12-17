#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

char shared_message[100] = "init";

void* communication_thread1(void* arg) {
    char* thread_name = (char*)arg;
    
    usleep(rand() % 100 * 1000);
    
    printf("  %s: read shared message: %s\n", thread_name, shared_message);
    strcat(shared_message, " -> thread 1");
    printf("  %s: revise shared message to: %s\n", thread_name, shared_message);
    
    usleep(rand() % 100 * 1000);
    
    char* result = malloc(50);
    snprintf(result, 50, "%s communication success", thread_name);
    pthread_exit((void*)result);
}

void* communication_thread2(void* arg) {
    char* thread_name = (char*)arg;
    
    usleep(rand() % 100 * 1000);
    
    printf("  %s: read shared message: %s\n", thread_name, shared_message);
    strcat(shared_message, " -> thread 2");
    printf("  %s: revise shared message to: %s\n", thread_name, shared_message);
    
    usleep(rand() % 100 * 1000);
    
    char* result = malloc(50);
    snprintf(result, 50, "%s communication success", thread_name);
    pthread_exit((void*)result);
}

void thread_communication() {
    printf("\n\n============== thread communication ==============\n");
    printf("init shared message:\n");
    printf("  shared message: %s\n", shared_message);
    
    pthread_t comm_thread1, comm_thread2;
    int rc;
    void* thread1_result;
    void* thread2_result;
    
    rc = pthread_create(&comm_thread1, NULL, communication_thread1, (void*)"thread 1");
    if (rc) {
        printf("failed to create thread 1, errno: %d\n", rc);
        return;
    }
    
    rc = pthread_create(&comm_thread2, NULL, communication_thread2, (void*)"thread 2");
    if (rc) {
        printf("failed to create thread 2, errno: %d\n", rc);
        return;
    }
    
    pthread_join(comm_thread1, &thread1_result);
    pthread_join(comm_thread2, &thread2_result);
    
    printf("\ncommunication end:\n");
    printf("  thread 1 return: %s\n", (char*)thread1_result);
    printf("  thread 2 return: %s\n", (char*)thread2_result);
    
    free(thread1_result);
    free(thread2_result);
    
    printf("  shared message: %s\n", shared_message);
}

int main() {
    srand(time(NULL));
    thread_communication();
    return 0;
}
