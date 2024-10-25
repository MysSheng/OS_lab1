#include "receiver.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr) {
    if (mailbox_ptr->flag == 1) {
        //Message Passing，從郵箱找到id，由ptr存放接收到的訊息
        msgrcv(mailbox_ptr->storage.msqid, message_ptr, MAX_MSG_SIZE, 0, 0);
    } else if (mailbox_ptr->flag == 2) {
        //Shared Memory，從郵箱找到要複製的記憶體位址並將訊息指向該處
        strncpy(message_ptr->msg_text, mailbox_ptr->storage.shm_addr, MAX_MSG_SIZE);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./receiver <1 for Message Passing, 2 for Shared Memory>\n");
        exit(1);
    }
    //User選擇方法
    int mechanism = atoi(argv[1]);
    mailbox_t mailbox;
    message_t message;
    struct timespec start, end;
    //決定如何通訊
    mailbox.flag = mechanism;

    if (mechanism == 1) {
        //先獲取一個IPC鍵值，在透過get向系統索取這個key的標示符
        key_t key = ftok("messagefile", 65);
        mailbox.storage.msqid = msgget(key, 0666 | IPC_CREAT);
    } else if (mechanism == 2) {
        key_t key = ftok("shmfile", 65);
        int shmid = shmget(key, MAX_MSG_SIZE, 0666 | IPC_CREAT);
        //標示符拉近記憶體空間
        mailbox.storage.shm_addr = (char*)shmat(shmid, NULL, 0);
    }


    //Initialize semaphores
    sem_t* sem_send = sem_open("/sem_send", O_CREAT, 0666, 1); // sender starts first
    sem_t* sem_recv = sem_open("/sem_recv", O_CREAT, 0666, 0); // receiver starts later

    double total_time = 0;
    //接收
    printf("\033[34mMessage Passing\033[0m\n");
    while (1) {
        sem_wait(sem_recv);
        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_time +=(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        if (strcmp(message.msg_text, "EXIT") == 0) {
            printf("\033[31mSender Exit\033[0m\n");
            printf("Total time taken in receiving msg: %lf s\n", total_time);
            break;
        }
        printf("\033[34mReceived:\033[0m %s\n", message.msg_text);
        sem_post(sem_send);
    }

    sem_close(sem_send);
    sem_close(sem_recv);
    sem_unlink("/sem_send");
    sem_unlink("/sem_recv");
    //釋放
    if (mechanism == 1) {
        msgctl(mailbox.storage.msqid, IPC_RMID, NULL);
    } else if (mechanism == 2) {
        shmdt(mailbox.storage.shm_addr);
        shmctl(shmget(ftok("shmfile", 65), MAX_MSG_SIZE, 0666), IPC_RMID, NULL);
    }

    return 0;
}

