#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <errno.h>

#define SHM_PATH "/myshm_pc_stack"
#define STACK_SIZE 3

struct shmbuf {
    sem_t mutex;     // 互斥鎖 (負責保護 Critical Section)
    sem_t sem_empty; // 計數信號量：記錄「剩餘空位」
    sem_t sem_full;  // 計數信號量：記錄「現有資料量」
    int top;         // 堆疊頂端索引
    char stack[STACK_SIZE];
};

void errorHandler(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

#endif