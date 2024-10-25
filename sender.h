#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>

#define MAX_MSG_SIZE 1024

typedef struct {
    long msg_type;  // Message type for message queue
    char msg_text[MAX_MSG_SIZE];  // Message content
} message_t;

typedef struct {
    int flag;  // 1 for Message Passing, 2 for Shared Memory
    union {
        int msqid;  // For message passing
        char* shm_addr;  // For shared memory
    } storage;
} mailbox_t;

void send(message_t message, mailbox_t* mailbox_ptr);

