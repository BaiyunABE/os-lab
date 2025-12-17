#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <sched.h>	
#include <errno.h>



void delay() {
  int delay_ms = (rand() % 291) + 10; // 10-300ms
  printf("delay %dms\n", delay_ms);
  usleep(delay_ms * 1000);
}

int child(void* arg) {
  printf("process(PID=%d) start, parameter: %s\n", getpid(), (char*)arg);
  
  for (int i = 1; i <= 4; i++) {
    delay();
    printf("step %d\n", i);
  }
  return 0;
}

void test_fork();
void test_clone();
void test_exec();
void test_kill();

int main(int argc, char* argv[]) {
  srand(time(NULL));
  
  if (argc > 1 && strcmp(argv[1], "exec_mode") == 0) {
    printf("=== process(PID=%d) start exec child ===\n", getpid());
    child("exec");
    return 0;
  }
  printf("main process PID=%d start\n", getpid());
  test_fork();
  test_clone();
  test_exec();
  test_kill();
  
  printf("\n=== all test done ===\n");
  printf("all syscalls:\n");
  printf("1. fork()     - create a child process\n");
  printf("2. clone()    - create a child process\n");
  printf("3. execve()   - replaces the current process image with a new process image\n");
  printf("4. waitpid()  - wait for process to change state\n");
  printf("5. exit()     - cause normal process termination\n");
  printf("6. kill()     - send signal to a process\n");
  printf("7. getpid()   - get process identification\n");
  
  return 0;
}


void test_fork() {
  printf("\n=== test fork ===\n");
  
  pid_t pid = fork();
  
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0) {
    printf("child process(PID=%d) start\n", getpid());
    
    for (int i = 1; i <= 3; i++) {
      delay();
      printf("fork step %d\n", i);
    }
    
    printf("fork exit\n");
    exit(EXIT_SUCCESS);
  }
  else {
    delay();
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      printf("child process(PID=%d) is exit, return %d\n", pid, WEXITSTATUS(status));
    }
  }
}

void test_clone() {
  printf("\n=== test clone ===\n");
  
  char stack[1024*1024]; // 1MB
  char* arg = "clone";
  

  printf("\n1. default parameter(like fork):\n");
  pid_t pid = clone(
    child, 
    stack + sizeof(stack), 
    SIGCHLD, 
    arg);
  
  if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      printf("child process(PID=%d) is exit, return %d\n", pid, WEXITSTATUS(status));
    }
  }
  
  printf("\n2. set CLONE_FILES|CLONE_FS:\n");
  pid = clone(
    child, 
    stack + sizeof(stack), 
    CLONE_FILES | CLONE_FS | SIGCHLD, 
    arg);
  
  if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    int status;
    waitpid(pid, &status, 0);
  }
  
  printf("\n3. set CLONE_VFORK:\n");
  pid = clone(
    child, 
    stack + sizeof(stack), 
    CLONE_VFORK | SIGCHLD, 
    arg);
  
  if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    int status;
    waitpid(pid, &status, 0);
  }
  
  printf("\n4. set CLONE_VM(share memory)\n");
  pid = clone(
    child, 
    stack + sizeof(stack), 
    CLONE_VM | CLONE_FILES | CLONE_FS | SIGCHLD,
    arg);
  
  if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    int status;
    waitpid(pid, &status, 0);
  }
}

void test_exec() {
  printf("\n=== test exec ===\n");
  
  pid_t pid = fork();
  if (pid == 0) {
    printf("child process(PID=%d) start\n", getpid());
    
    char* args[] = {"./process_test", "exec_mode", NULL};
    char* env[] = {"EXEC_MODE=1", NULL};
    
    delay();
    
    printf("child process exec\n");
    execve("./process_test", args, env);
    
    perror("execve");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    
    sleep(1);
    
    printf("process(PID=%d) send signal SIGTERM to child process(PID=%d)\n", getpid(), pid);
    if (kill(pid, SIGTERM) == 0) {
      printf("kill success\n");
    } else {
      perror("kill");
    }
    
    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
      printf("child process(PID=%d) is killed by signal %d\n", pid, WTERMSIG(status));
    }
  }
}

void test_kill() {
  printf("\n=== test kill ===\n");
  
  pid_t pid = fork();
  
  if (pid == 0) {
    printf("child process(PID=%d) start\n", getpid());
    int counter = 0;
    while (1) {
      printf("child process(PID=%d) is running\n", getpid());
      delay();
      
      if (counter >= 100) {
        printf("child process(PID=%d) exit\n", getpid());
        exit(EXIT_SUCCESS);
      }
    }
  } else if (pid > 0) {
    printf("process(PID=%d) create child process(PID=%d)\n", getpid(), pid);
    
    sleep(1);
    
    printf("parent process kill child process(PID=%d)\n", pid);
    kill(pid, SIGKILL);
    
    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
      printf("child process(PID=%d) is killed by signal %d\n", pid, WTERMSIG(status));
    }
  }
}
