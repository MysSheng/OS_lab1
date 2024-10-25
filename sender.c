#include "sender.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

void send(message_t message, mailbox_t* mailbox_ptr) {
    if (mailbox_ptr->flag == 1) {
        //Message Passing
        msgsnd(mailbox_ptr->storage.msqid, &message, sizeof(message.msg_text), 0);
    } else if (mailbox_ptr->flag == 2) {
        //Shared Memory
        strncpy(mailbox_ptr->storage.shm_addr, message.msg_text, MAX_MSG_SIZE);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./sender <1 for Message Passing, 2 for Shared Memory> <input file>\n");
        exit(1);
    }

    int mechanism = atoi(argv[1]);
    char* input_file = argv[2];
    mailbox_t mailbox;
    message_t message;
    struct timespec start, end;

    mailbox.flag = mechanism;

    if (mechanism == 1) {
        key_t key = ftok("messagefile", 65);
        mailbox.storage.msqid = msgget(key, 0666 | IPC_CREAT);
    } else if (mechanism == 2) {
        key_t key = ftok("shmfile", 65);
        int shmid = shmget(key, MAX_MSG_SIZE, 0666 | IPC_CREAT);
        mailbox.storage.shm_addr = (char*)shmat(shmid, NULL, 0);
    }

    //Initialize semaphores
    sem_t* sem_send = sem_open("/sem_send", O_CREAT, 0666, 1); // sender starts first
    sem_t* sem_recv = sem_open("/sem_recv", O_CREAT, 0666, 0); // receiver starts later

    double total_time = 0;

    //讀檔
    FILE* file = fopen(input_file, "r");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    printf("\033[34mMessage Passing:\033[0m\n");
    while (fgets(message.msg_text, MAX_MSG_SIZE, file) != NULL) {
        sem_wait(sem_send);
        //將檔案收到的文字訊息發包送出
        message.msg_type = 1;
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        total_time +=(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("\033[34mSending Message:\033[0m %s\n", message.msg_text);
        sem_post(sem_recv);
    }
    printf("\033[31mEnd of input file! exit!\033[0m\n");
    printf("Total time taken in sending msg: %lf s\n", total_time);
    //離開的訊息
    strcpy(message.msg_text, "EXIT");
    sem_wait(sem_send);
    send(message, &mailbox);
    sem_post(sem_recv);  // Allow receiver to finish
    fclose(file);
    // Cleanup semaphores
    sem_close(sem_send);
    sem_close(sem_recv);
    sem_unlink("/sem_send");
    sem_unlink("/sem_recv");
    // Cleanup
    if (mechanism == 2) {
        shmdt(mailbox.storage.shm_addr);  // Detach shared memory
    }
    return 0;
}

