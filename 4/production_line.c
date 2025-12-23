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

void Sem_init(sem_t *sem, int pshared, unsigned int value) {
    int rv = sem_init(sem, pshared, value);
    if (rv == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}

void Sem_wait(sem_t *sem) {
    int rv = sem_wait(sem);
    if (rv == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}

void Sem_post(sem_t *sem) {
    int rv = sem_post(sem);
    if (rv == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

void* workerA(void*) {
    while (1) {
        Sem_wait(&mutexE);
        Sem_wait(&mutexB);
        if (empty >= M1 && empty >= M1 + (N2 - countB)) {
            Sem_post(&mutexB);

            empty -= M1;
            Sem_post(&mutexE);
            
            Sem_wait(&mutexA);
            countA += M1;
            Sem_post(&mutexA);

            printf("A: put %d A\n", M1);
        } else {
            Sem_post(&mutexE);
            Sem_post(&mutexB);

            waitingA = true;

            if (waitingC) {
                waitingC = false;
                printf("A: wake C\n");
                Sem_post(&condC);
            }

            printf("A: wait...\n");
            Sem_wait(&condA);
            printf("A: continue\n");
        }
    }
    return NULL;
}

void* workerB(void*) {
    while (1) {
        Sem_wait(&mutexE);
        Sem_wait(&mutexA);
        if (empty >= M2 && empty >= M2 + (N1 - countA)) {
            Sem_post(&mutexA);

            empty -= M2;
            Sem_post(&mutexE);
            
            Sem_wait(&mutexB);
            countB += M2;
            Sem_post(&mutexB);
            
            printf("B: put %d B\n", M2);
        } else {
            Sem_post(&mutexE);
            Sem_post(&mutexA);
            
            waitingB = true;

            if (waitingC) {
                waitingC = false;
                printf("B: wake C\n");
                Sem_post(&condC);
            }

            printf("B: wait...\n");
            Sem_wait(&condB);
            printf("B: continue\n");
        }
    }
    return NULL;
}

void* workerC(void*) {
    unsigned long long C = 0;
    
    while (1) {
        if (countA >= N1 && countB >= N2) {
            Sem_wait(&mutexA);
            countA -= N1;
            Sem_post(&mutexA);

            Sem_wait(&mutexB);
            countB -= N2;
            Sem_post(&mutexB);
            
            Sem_wait(&mutexE);
            empty += (N1 + N2);
            Sem_post(&mutexE);

            printf("C: get %d A and %d B\n", N1, N2);
            
            if (waitingA) {
                waitingA = false;
                printf("C: wake A\n");
                Sem_post(&condA);
            }

            if (waitingB) {
                waitingB = false;
                printf("C: wake B\n");
                Sem_post(&condB);
            }

            C++;
            printf("C: produce C (%llu)\n", C);
            
        } else {
            waitingC = true;

            if (countA < N1 && waitingA) {
                waitingA = false;
                printf("C: wake A\n");
                Sem_post(&condA);
            }

            if (countB < N2 && waitingB) {
                waitingB = false;
                printf("C: wake B\n");
                Sem_post(&condB);
            }

            printf("C: wait...\n");
            Sem_wait(&condC);
            printf("C: continue\n");
        }
    }
    return NULL;
}

int main() {
    pthread_t threadA, threadB, threadC;
    
    Sem_init(&mutexA, 0, 1);
    Sem_init(&mutexB, 0, 1);
    Sem_init(&mutexE, 0, 1);
    Sem_init(&condA, 0, 0);
    Sem_init(&condB, 0, 0);
    Sem_init(&condC, 0, 0);
    
    Pthread_create(&threadA, NULL, workerA, NULL);
    Pthread_create(&threadB, NULL, workerB, NULL);
    Pthread_create(&threadC, NULL, workerC, NULL);

    while (1) {
        printf("%d %d %d\n", countA, countB, empty);
        sleep(1);
    }

    Pthread_join(threadA, NULL);
    Pthread_join(threadB, NULL);
    Pthread_join(threadC, NULL);
    
    return 0;
}