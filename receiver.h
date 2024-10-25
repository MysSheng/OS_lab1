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
    //識別訊息類別
    long msg_type;
    char msg_text[MAX_MSG_SIZE];
} message_t;

typedef struct {
    int flag;  //實作方法 1是passing 2是shared
    union {
        int msqid;  //message passing 的id
        char* shm_addr;  //shared memory 要的共享位置
    } storage;
} mailbox_t;

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr);

