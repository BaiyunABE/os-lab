#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

pid_t Fork(void) {
    pid_t rv = fork();
    if (rv == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Pipe(int pipefd[2]) {
    int rv = pipe(pipefd);
    if (rv == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Lockf(int fd, int cmd, off_t len) {
    int rv = lockf(fd, cmd, len);
    if (rv == -1) {
        perror("lockf");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int main() {
    int fd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    int length;
    
    Pipe(fd);
    
    pid = Fork();
    
    if (pid == 0) {
        printf("Child process (PID: %d) is writing...\n", getpid());
        
        close(fd[0]);
        
        Lockf(fd[1], F_LOCK, 0);
        printf("Child process: Pipe locked for writing\n");
        
        char message[] = "Hello from child process through unnamed pipe!";
        write(fd[1], message, strlen(message));
        printf("Child process: Data written to pipe\n");
        
        Lockf(fd[1], F_ULOCK, 0);
        printf("Child process: Pipe unlocked\n");
        
        close(fd[1]);
        exit(EXIT_SUCCESS);
        
    } else {
        printf("Parent process (PID: %d) is reading...\n", getpid());
        
        close(fd[1]);
        
        wait(NULL);
        
        memset(buffer, 0, BUFFER_SIZE);
        length = read(fd[0], buffer, BUFFER_SIZE);
        printf("Parent process received %d bytes: %s\n", length, buffer);

        close(fd[0]);
    }
    
    return EXIT_SUCCESS;
}