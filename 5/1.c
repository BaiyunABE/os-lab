#include <stdio.h>

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

int main() {
    unsigned long long sum;
    int index;

    struct apple test;
    struct orange test1;

    for (int i = 0; i < ORANGE_MAX_VALUE; i++) {
        test1.a[i] = 1;
        test1.b[i] = 1;
    }

    for (sum = 0; sum < APPLE_MAX_VALUE; sum++) {
        test.a += sum;
        test.b += sum;
    }
    sum = 0;
    for (index = 0; index < ORANGE_MAX_VALUE; index++) {
        sum += test1.a[index] + test1.b[index];
    }

    printf("%lld\n%lld\n%lld\n", test.a, test.b, sum);

    return 0;
}