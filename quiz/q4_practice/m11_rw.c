/* Source: Module 11, Slides 14-18 */
#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
//#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LOOP_MAX 5
#define TRUE 1
#define FALSE 0

typedef int boolean_t;

/* --- utils 實作 --- */
void fractSleep(double time_sec) {
    struct timespec timeSpec;
    timeSpec.tv_sec = (time_t)time_sec;
    timeSpec.tv_nsec = (long)((time_sec - timeSpec.tv_sec) * 1000000000);
    nanosleep(&timeSpec, NULL);
}

void printWithTime(const char *msg) {
    time_t t;
    struct tm *tm_info;
    char buffer[26];
    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 26, "%H:%M:%S", tm_info);
    printf("[%s] %s", buffer, msg);
}
/* ----------------- */

/* 共享資源 */
int sharedData = 0;

/* 定義讀寫鎖 */
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
/* 定義互斥鎖 (用於對照組) */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* 是否使用讀寫鎖的旗標 */
boolean_t userwlock = FALSE;

/* 模擬寫入操作 */
void doWrite(int id) {
    int temp = sharedData;
    fractSleep(0.5); /* 寫入通常比較慢 */
    sharedData = temp + 1;
    char buf[64];
    sprintf(buf, "Writer %d updated data to %d\n", id, sharedData);
    printWithTime(buf);
}

/* 模擬讀取操作 */
void doRead(int id) {
    char buf[64];
    /* 讀取通常很快，且多人可以同時讀 */
    sprintf(buf, "Reader %d read data: %d\n", id, sharedData);
    printWithTime(buf);
    fractSleep(0.1); 
}

/* 寫入者執行緒 */
void *writerMain(void *arg) {
    int id = *((int *)arg);
    int loopCount;
    for (loopCount = 0; loopCount < LOOP_MAX; loopCount++) {
        if (userwlock) {
            /* 取得寫入鎖 (獨佔) */
            pthread_rwlock_wrlock(&rwlock);
        } else {
            pthread_mutex_lock(&mutex);
        }

        doWrite(id);

        if (userwlock) {
            pthread_rwlock_unlock(&rwlock);
        } else {
            pthread_mutex_unlock(&mutex);
        }
        fractSleep(1.0); /* 休息一下 */
    }
    free(arg);
    return NULL;
}

/* 讀取者執行緒 */
void *readerMain(void *arg) {
    int id = *((int *)arg);
    int loopCount;
    for (loopCount = 0; loopCount < LOOP_MAX * 2; loopCount++) {
        if (userwlock) {
            /* 取得讀取鎖 (共享) */
            pthread_rwlock_rdlock(&rwlock);
        } else {
            pthread_mutex_lock(&mutex);
        }

        doRead(id);

        if (userwlock) {
            pthread_rwlock_unlock(&rwlock);
        } else {
            pthread_mutex_unlock(&mutex);
        }
        fractSleep(0.2); /* 頻繁讀取 */
    }
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t writers[2], readers[5];
    int i;

    if ((argc > 1) && (strcmp(argv[1], "rw") == 0)) {
        userwlock = TRUE;
        printf("\nRunning with RWLock (Readers can run in parallel)\n\n");
    } else {
        printf("\nRunning with Mutex (Readers block each other)\n\n");
    }

    /* 建立 2 個寫入者 */
    for (i = 0; i < 2; i++) {
        int *id = malloc(sizeof(int)); *id = i;
        pthread_create(&writers[i], NULL, writerMain, id);
    }

    /* 建立 5 個讀取者 */
    for (i = 0; i < 5; i++) {
        int *id = malloc(sizeof(int)); *id = i;
        pthread_create(&readers[i], NULL, readerMain, id);
    }

    /* 等待所有執行緒 */
    for (i = 0; i < 2; i++) pthread_join(writers[i], NULL);
    for (i = 0; i < 5; i++) pthread_join(readers[i], NULL);

    return 0;
}