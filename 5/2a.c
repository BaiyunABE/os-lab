#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define ORANGE_MAX_VALUE 1000000
#define APPLE_MAX_VALUE 100000000
#define MSECOND 1000000

struct apple {
    unsigned long long a;
    unsigned long long b;
};

struct orange {
    int a[ORANGE_MAX_VALUE];
    int b[ORANGE_MAX_VALUE];
};

void set_cpu_affinity(pthread_t thread, int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        fprintf(stderr, "Failed to set CPU affinity for thread\n");
    }
}

void* calc_apple(void* arg) {
    struct apple* test = arg;
    for (unsigned long long sum = 0; sum < APPLE_MAX_VALUE; sum++) {
        test->a += sum;
        test->b += sum;
    }
    return NULL;
}

void* calc_orange(void* arg) {
    struct orange* test1 = arg;
    unsigned long long sum = 0;
    for (int index = 0; index < ORANGE_MAX_VALUE; index++) {
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

    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, calc_apple, (void*)&test);
    pthread_create(&thread2, NULL, calc_orange, (void*)&test1);

    set_cpu_affinity(thread1, 0);
    set_cpu_affinity(thread2, 1);

    unsigned long long sum;

    pthread_join(thread1, NULL);
    pthread_join(thread2, (void**)&sum);

    printf("%lld\n%lld\n%lld\n", test.a, test.b, sum);

    return 0;
}