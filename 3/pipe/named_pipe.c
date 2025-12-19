#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define FIFO_NAME "/tmp/my_named_pipe"

pid_t Fork(void) {
    pid_t rv = fork();
    if (rv == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Mkfifo(const char *pathname, mode_t mode) {
    int rv = mkfifo(pathname, mode);
    if (rv == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Open(const char *pathname, int flags) {
    int rv = open(pathname, flags);
    if (rv == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return rv;
}

ssize_t Read(int fd, void *buf, size_t count) {
    ssize_t rv = read(fd, buf, count);
    if (rv == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    return rv;
}

ssize_t Write(int fd, const void *buf, size_t count) {
    ssize_t rv = write(fd, buf, count);
    if (rv == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int main() {
    int fifo_fd;
    char buffer[BUFFER_SIZE];
    
    int pid = Fork();
    
    if (pid == 0) {
        printf("Reader process (PID: %d) starting...\n", getpid());
        
        Mkfifo(FIFO_NAME, 0666);
        
        printf("Reader: Opening named pipe for reading...\n");
        fifo_fd = Open(FIFO_NAME, O_RDONLY);
        printf("Reader: Named pipe opened successfully\n");
        
        printf("Reader: Waiting for data from writer...\n");
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = Read(fifo_fd, buffer, BUFFER_SIZE);
        
        if (bytes_read > 0) {
            printf("Reader: Received %d bytes from writer: %s\n", bytes_read, buffer);
        } else if (bytes_read == 0) {
            printf("Reader: Writer has closed the pipe\n");
        }
        
        close(fifo_fd);
        
        unlink(FIFO_NAME);
        printf("Reader: Named pipe removed\n");
        
        exit(EXIT_SUCCESS);
    } else {
        usleep(100000);
        
        printf("Writer process (PID: %d) starting...\n", getpid());
        
        printf("Writer: Opening named pipe for writing...\n");
        fifo_fd = Open(FIFO_NAME, O_WRONLY);
        printf("Writer: Named pipe opened successfully\n");
        
        char message[] = "Hello from writer process through named pipe!";
        strcpy(buffer, message);
        
        printf("Writer: Writing data to named pipe...\n");
        int bytes_written = Write(fifo_fd, buffer, strlen(buffer));
        
        printf("Writer: Successfully wrote %d bytes to pipe\n", bytes_written);
        
        close(fifo_fd);
        printf("Writer: Writer process completed\n");
    }
    
    return EXIT_SUCCESS;
}