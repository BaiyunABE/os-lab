#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct message {
    long msg_type;
    char msg_text[100];
};

#define MSG_KEY 1234
#define MSG_TYPE 1

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

int Msgget(key_t key, int msgflg) {
    int rv = msgget(key, msgflg);
    if (rv == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Msgctl(int msqid, int cmd, struct msqid_ds *buf) {
    int rv = msgctl(msqid, cmd, buf);
    if (rv == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    return rv;
}

int Msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
    int rv = msgsnd(msqid, msgp, msgsz, msgflg);
    if (rv == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    return rv;
}

ssize_t Msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
    ssize_t rv = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
    if (rv == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    return rv;
}

void sender(int msgid) {
    struct message msg;
    int msg_count = 5;
    
    printf("sender start (PID: %d)\n", getpid());
    
    for (int i = 1; i <= msg_count; i++) {
        msg.msg_type = MSG_TYPE;
        snprintf(msg.msg_text, sizeof(msg.msg_text),
            "message %d from %d", i, getpid());
        Msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
        printf("SEND: %s\n", msg.msg_text);
        sleep(1);
    }
    
    msg.msg_type = MSG_TYPE;
    strcpy(msg.msg_text, "END");
    msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);
    printf("sender: send completed\n");
}

void receiver(int msgid) {
    struct message msg;
    int running = 1;
    
    printf("receiver start (PID: %d)\n", getpid());
    printf("receiver: wait for message...\n");
    
    while (running) {
        Msgrcv(msgid, &msg, sizeof(msg.msg_text), MSG_TYPE, 0);
        
        printf("RECV: %s\n", msg.msg_text);
        
        if (strncmp(msg.msg_text, "END", 3) == 0) {
            running = 0;
        }
    }
    
    printf("receiver: receive completed, start to remove message queue\n");
    Msgctl(msgid, IPC_RMID, 0);
    printf("receiver: remove message queue completed\n");
}

int main() {
    pid_t pid;
    int msgid;
    
    printf("=== System V message queue ===\n");
    
    msgid = Msgget(MSG_KEY, IPC_CREAT | 0666);
    printf("main process: create message queue success, ID: %d\n", msgid);
    
    struct msqid_ds buf;
    Msgctl(msgid, IPC_STAT, &buf);
    printf("message queue info:\n");
    printf("  mode: %o\n", buf.msg_perm.mode);
    printf("  number of messages: %lu\n", buf.msg_qnum);
    printf("  maximum bytes: %lu\n", buf.msg_qbytes);
    
    pid = Fork();
    if (pid == 0) {
        receiver(msgid);
        exit(EXIT_SUCCESS);
    } else {
        sleep(1);
        sender(msgid);
        Wait(NULL);
        printf("main process: all done\n");
    }
    
    return 0;
}
