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

#define SHM_PATH "/myshm_stack"
#define STACK_SIZE 3

struct shmbuf {
    sem_t sem;
    int top;
    char stack[STACK_SIZE];
};

void errorHandler(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

#endif