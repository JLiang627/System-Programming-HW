/* Source: Module 11, Slides 7-13 */
#define _POSIX_C_SOURCE 199309L /* 為了 nanosleep */
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define LOOP_MAX 20 /* 每個執行緒累加的次數 */
#define TRUE 1
#define FALSE 0

typedef int boolean_t;

/* 延遲函數：暫停指定的秒數 (支援小數點) */
void fractSleep(double time_sec) {
    struct timespec timeSpec;
    timeSpec.tv_sec = (time_t)time_sec;
    timeSpec.tv_nsec = (long)((time_sec - timeSpec.tv_sec) * 1000000000);
    nanosleep(&timeSpec, NULL);
}

/* 印出帶有時間戳記的訊息 */
void printWithTime(const char *msg) {
    time_t t;
    struct tm *tm_info;
    char buffer[26];
    time(&t);
    tm_info = localtime(&t);
    strftime(buffer, 26, "%H:%M:%S", tm_info);
    printf("[%s] %s", buffer, msg);
}
/* -------------------------------------------- */

/* 全域變數 */
int commonCounter = 0; /* 所有執行緒共享的計數器 */

/* Mutex 鎖初始化 */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* 延遲時間，用來製造競爭條件 (Contention) */
double delay = 0.2;

/* 是否開啟同步功能的旗標 */
boolean_t synchronized = FALSE;

/* 執行緒的主程式 */
void *threadMain(void *dummy) {
    int loopCount;
    int temp;
    char buffer[80];

    for (loopCount = 0; loopCount < LOOP_MAX; loopCount++) {
        
        /* 如果開啟同步模式，進入 Critical Section 前要上鎖 */
        if (synchronized) {
            pthread_mutex_lock(&mutex);
        }

        /* --- Critical Section 開始 --- */
        
        /* 讀取全域變數 */
        temp = commonCounter;
        
        /* 故意延遲，讓 Race Condition 容易發生 */
        fractSleep(delay);
        
        /* 修改並寫回 */
        commonCounter = temp + 1;

        /* 印出狀態 */
        sprintf(buffer, "Counter: %d\n", commonCounter);
        printWithTime(buffer);

        /* --- Critical Section 結束 --- */

        /* 解鎖 */
        if (synchronized) {
            pthread_mutex_unlock(&mutex);
        }

        /* 在兩次累加中間休息一下，製造交錯執行的機會 */
        fractSleep(delay / 2.0); 
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t threadID1;
    pthread_t threadID2;

    /* 檢查命令列參數，如果有 "sync" 就開啟同步模式 */
    if ((argc > 1) && (strcmp(argv[1], "sync") == 0)) {
        synchronized = TRUE;
        printf("\nRunning with sync ENABLED\n\n");
    } else {
        printf("\nRunning with sync DISABLED (Expect errors)\n\n");
    }

    /* 建立兩個執行緒 */
    if (pthread_create(&threadID1, NULL, threadMain, NULL) != 0 ||
        pthread_create(&threadID2, NULL, threadMain, NULL) != 0) {
        perror("Error starting thread(s)");
        exit(errno);
    }

    /* 等待執行緒結束 */
    pthread_join(threadID1, NULL);
    pthread_join(threadID2, NULL);

    printf("\nFinal commonCounter: %d (Expected: %d)\n", commonCounter, LOOP_MAX * 2);

    return 0;
}