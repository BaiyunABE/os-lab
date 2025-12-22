#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

const int M1 = 2;
const int M2 = 1;
const int N1 = 4;
const int N2 = 3;
const int N = 12;

int countA = 0;
int countB = 0;
int empty = N;

sem_t mutexA;
sem_t mutexB;
sem_t mutexE;

sem_t condA;
sem_t condB;
sem_t condC;

volatile bool waitingA = false;
volatile bool waitingB = false;
volatile bool waitingC = false;

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

void* workerA(void*) {
    while (1) {
        sem_wait(&mutexE);
        sem_wait(&mutexB);
        if (empty >= M1 && empty >= M1 + (N2 - countB)) {
            sem_post(&mutexB);

            empty -= M1;
            sem_post(&mutexE);
            
            sem_wait(&mutexA);
            countA += M1;
            sem_post(&mutexA);

            printf("A: put %d A\n", M1);
            
            if (waitingC) {
                waitingC = false;
                printf("A: wake C\n");
                sem_post(&condC);
            }
            
            if (waitingB) {
                waitingB = false;
                printf("A: wake B\n");
                sem_post(&condB);
            }

        } else {
            sem_post(&mutexE);
            sem_post(&mutexB);

            waitingA = true;
            printf("A: wait...\n");
            sem_wait(&condA);
            printf("A: continue\n");
        }
    }
    return NULL;
}

void* workerB(void*) {
    while (1) {
        sem_wait(&mutexE);
        sem_wait(&mutexA);
        if (empty >= M2 && empty >= M2 + (N1 - countA)) {
            sem_post(&mutexA);

            empty -= M2;
            sem_post(&mutexE);
            
            sem_wait(&mutexB);
            countB += M2;
            sem_post(&mutexB);
            
            printf("B: put %d B\n", M2);

            if (waitingC) {
                waitingC = false;
                printf("B: wake C\n");
                sem_post(&condC);
            }

            if (waitingA) {
                waitingA = false;
                printf("B: wake A\n");
                sem_post(&condA);
            }

        } else {
            sem_post(&mutexE);
            sem_post(&mutexA);

            waitingB = true;
            printf("B: wait...\n");
            sem_wait(&condB);
            printf("B: continue\n");
        }
    }
    return NULL;
}

void* workerC(void*) {
    int C = 0;
    
    while (1) {
        sem_wait(&mutexA);
        sem_wait(&mutexB);
        if (countA >= N1 && countB >= N2) {
            countA -= N1;
            sem_post(&mutexA);

            countB -= N2;
            sem_post(&mutexB);
            
            sem_wait(&mutexE);
            empty += (N1 + N2);
            sem_post(&mutexE);

            printf("C: get %d A and %d B\n", N1, N2);
            
            if (waitingA) {
                waitingA = false;
                printf("C: wake A\n");
                sem_post(&condA);
            }

            if (waitingB) {
                waitingB = false;
                printf("C: wake B\n");
                sem_post(&condB);
            }

            C++;
            printf("C: produce C (%d)\n", C);
            
        } else {
            sem_post(&mutexB);
            sem_post(&mutexA);

            waitingC = true;
            printf("C: wait...\n");
            sem_wait(&condC);
            printf("C: continue\n");
        }
    }
    return NULL;
}

int main() {
    pthread_t threadA, threadB, threadC;
    
    sem_init(&mutexA, 0, 1);
    sem_init(&mutexB, 0, 1);
    sem_init(&mutexE, 0, 1);
    sem_init(&condA, 0, 0);
    sem_init(&condB, 0, 0);
    sem_init(&condC, 0, 0);
    
    Pthread_create(&threadA, NULL, workerA, NULL);
    Pthread_create(&threadB, NULL, workerB, NULL);
    Pthread_create(&threadC, NULL, workerC, NULL);

    Pthread_join(threadA, NULL);
    Pthread_join(threadB, NULL);
    Pthread_join(threadC, NULL);
    
    return 0;
}