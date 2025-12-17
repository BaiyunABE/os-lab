#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void* thread_func1(void* arg);
void* thread_func2(void* arg);
void* thread_func3(void* arg);

void thread_management() {
    printf("\n============== create and manage thread ==============\n");
    
    pthread_t thread1, thread2, thread3;
    int rc;
    void* thread1_result;
    void* thread2_result;
    void* thread3_result;
    
    pthread_t main_thread = pthread_self();
    printf("main thread ID: %lu\n", (unsigned long)main_thread);
    
    rc = pthread_create(&thread1, NULL, thread_func1, (void*)"thread 1");
    if (rc) {
        printf("fail to create thread 1, errno: %d\n", rc);
        exit(-1);
    }
    printf("thread 1 is created, ID: %lu\n", (unsigned long)thread1);
    
    usleep(rand() % 200 * 1000);
    
    rc = pthread_create(&thread2, NULL, thread_func2, (void*)"thread 2");
    if (rc) {
        printf("fail to create thread 2, errno: %d\n", rc);
        exit(-1);
    }
    printf("thread 2 is created, ID: %lu\n", (unsigned long)thread2);
    
    usleep(rand() % 200 * 1000);
    
    rc = pthread_create(&thread3, NULL, thread_func3, (void*)"thread 3");
    if (rc) {
        printf("fail to create thread 3, errno: %d\n", rc);
        exit(-1);
    }
    printf("thread 3 is created, ID: %lu\n", (unsigned long)thread3);
    
    usleep(rand() % 200 * 1000);
    
    printf("\nmain thread start to cancel thread 3...\n");
    rc = pthread_cancel(thread3);
    if (rc == 0) {
        printf("cancel request is sent to thread 3\n");
    }
    
    usleep(rand() % 200 * 1000);
    
    printf("\nmain thread is waiting for all subthread...\n");
    

    rc = pthread_join(thread1, &thread1_result);
    if (rc == 0) {
        printf("thread 1 is end, return value: %s\n", (char*)thread1_result);
        free(thread1_result);
    }
    
    rc = pthread_join(thread2, &thread2_result);
    if (rc == 0) {
        printf("thread 2 is end, return value: %s\n", (char*)thread2_result);
        free(thread2_result);
    }
    
    rc = pthread_join(thread3, &thread3_result);
    if (rc == 0) {
        if (thread3_result == PTHREAD_CANCELED) {
            printf("thread 3 is canceled\n");
        } else {
            printf("thread 3 is end, return value: %s\n", (char*)thread3_result);
            free(thread3_result);
        }
    }
    
    printf("all thread is end\n");
}

 		
void* thread_func1(void* arg) {
    char* thread_name = (char*)arg;
    printf("  %s start, ID: %lu\n", thread_name, (unsigned long)pthread_self());
    

    for (int i = 0; i < 5; i++) {
        usleep(rand() % 100 * 1000);
        printf("  %s is working, %d/5\n", thread_name, i + 1);
    }
    
    printf("  %s will return\n", thread_name);
    
    char* result = malloc(50);
    snprintf(result, 50, "%s end using return", thread_name);
    
    return (void*)result;
}


void* thread_func2(void* arg) {
    char* thread_name = (char*)arg;
    printf("  %s start, ID: %lu\n", thread_name, (unsigned long)pthread_self());
    
    for (int i = 0; i < 5; i++) {
        usleep(rand() % 150 * 1000);
        printf("  %s is working, %d/5\n", thread_name, i + 1);
        
        if (i == 2) {
            printf("  %s will end before done\n", thread_name);
            
            char* result = malloc(50);
            snprintf(result, 50, "%s end using pthread_exit()", thread_name);
            
            pthread_exit((void*)result);
        }
    }
    
    char* result = malloc(50);
    snprintf(result, 50, "%s done", thread_name);
    return (void*)result;
}
	
void* thread_func3(void* arg) {
    char* thread_name = (char*)arg;
    printf("  %s start, ID: %lu\n", thread_name, (unsigned long)pthread_self());
    
    int i = 0;
    while (1) {
        usleep(rand() % 100 * 1000);
        printf("  %s is working, %d\n", thread_name, ++i);
    }
    
    char* result = malloc(50);
    snprintf(result, 50, "%s done", thread_name);
    return (void*)result;
}


int main() {
    srand(time(NULL));
    thread_management();
    return 0;
}
