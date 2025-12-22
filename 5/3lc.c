#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define ORANGE_MAX_VALUE 1000000
#define APPLE_MAX_VALUE 100000000
#define MSECOND 1000000

sem_t mutex;

struct apple {
    unsigned long long a;
    char c[64];
    unsigned long long b;
};

struct orange {
    int a[ORANGE_MAX_VALUE];
    int b[ORANGE_MAX_VALUE];
};

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

void* calc_a(void* arg) {
    struct apple* test = arg;
    unsigned long long sum;
    sem_wait(&mutex);
    for (sum = 0; sum < APPLE_MAX_VALUE; sum++) {
        test->a += sum;
    }
    sem_post(&mutex);
    return NULL;
}

void* calc_b(void* arg) {
    struct apple* test = arg;
    unsigned long long sum;
    sem_wait(&mutex);
    for (sum = 0; sum < APPLE_MAX_VALUE; sum++) {
        test->b += sum;
    }
    sem_post(&mutex);
    return NULL;
}

void* calc_orange(void* arg) {
    struct orange* test1 = arg;
    unsigned long long sum = 0;
    int index;
    for (index = 0; index < ORANGE_MAX_VALUE; index++) {
        sum += test1->a[index] + test1->b[index];
    }
    return (void*)sum;
}

int main() {
    struct apple test;
    struct orange test1;

    for (int i = 0; i < ORANGE_MAX_VALUE; i++) {
        test1.a[i] = 1;
        test1.b[i] = 1;
    }

    pthread_t thread1, thread2, thread3;

    unsigned long long sum;

    sem_init(&mutex, 0, 1);

    Pthread_create(&thread1, NULL, calc_a, (void*)&test);
    Pthread_create(&thread2, NULL, calc_b, (void*)&test);
    Pthread_create(&thread3, NULL, calc_orange, (void*)&test1);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, (void**)&sum);

    printf("%lld\n%lld\n%lld\n", test.a, test.b, sum);

    return 0;
}