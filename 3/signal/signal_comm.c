#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

int Kill(pid_t pid, int sig) {
    int rv = kill(pid, sig);
    if (rv == -1) {
        perror("kill");
        exit(EXIT_FAILURE);
    }
    return rv;
}

void child1_handler(int sig) {
    printf("Child Process 1 is Killed by Parent!\n");
    exit(EXIT_SUCCESS);
}

void child2_handler(int sig) {
    printf("Child Process 2 is Killed by Parent!\n");
    exit(EXIT_SUCCESS);
}

void parent_handler(int sig) {
    printf("\nParent received interrupt signal\n");
}

int main() {
    signal(SIGINT, parent_handler);
    
    pid_t pid1 = Fork();
    if (pid1 == 0) {
        signal(16, child1_handler);
        printf("Child Process 1 is created, PID = %d\n", getpid());
        pause();
        exit(EXIT_SUCCESS);
    }
    
    pid_t pid2 = Fork();
    if (pid2 == 0) {
        signal(17, child2_handler);
        printf("Child Process 2 is created, PID = %d\n", getpid());
        pause();
        exit(EXIT_SUCCESS);
    }
    
    printf("Parent Process created two children:\n");
    printf("  Child 1 PID: %d\n", pid1);
    printf("  Child 2 PID: %d\n", pid2);
    printf("\nPress Ctrl+C (or DEL) to send signals to children...\n");

    pause();
    
    printf("\nSending signals to children...\n");
    Kill(pid1, 16);
    Kill(pid2, 17);
    
    Wait(NULL);
    Wait(NULL);
    
    printf("Parent Process is Killed!\n");
    return EXIT_SUCCESS;
}