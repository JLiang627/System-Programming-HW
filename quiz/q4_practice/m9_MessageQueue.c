/* Source: Module 9, Slides 9-10 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 80 

/* 定義訊息結構 (Slide 中使用了指標但隱含了此結構) */
struct msgbuf {
    long mtype;
    char mtext[1]; /* 使用彈性陣列宣告，實際大小由 malloc 決定 */
};

int main() {
    struct msgbuf *sendmsg;
    struct msgbuf *recvmsg;
    int msgqid;
    int rval = 0;

    /* 分配記憶體：long (type) + 資料長度 */
    sendmsg = (struct msgbuf *) malloc(sizeof(long) + BUFSIZE); 
    recvmsg = (struct msgbuf *) malloc(sizeof(long) + BUFSIZE); 

    if (sendmsg == NULL || recvmsg == NULL) {
        fprintf(stderr, "Out of virtual memory.\n");
        exit(1);
    }

    /* 建立 Message Queue (IPC_PRIVATE 代表建立新的 key) */
    msgqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600); 

    if (msgqid == -1) {
        perror("msgget");
        exit(1);
    }

    system("ipcs -q"); /* 顯示目前的 IPC 狀態 (Linux 使用 -q 只看 queue) */ 

    /* 準備發送訊息 */
    sendmsg->mtype = 1;
    strcpy(sendmsg->mtext, "Test messages."); 
    /* 發送訊息 */
    if (msgsnd(msgqid, sendmsg, strlen(sendmsg->mtext) + 1, 0) == -1) { 
        perror("msgsnd");
        rval = 1;
        goto out;
    }

    /* 接收訊息 (type 1) */
    if (msgrcv(msgqid, recvmsg, BUFSIZE, 1, 0) == -1) {
        perror("msgrcv");
        rval = 1;
        goto out;
    }

    printf("\nReceived message: %s\n\n", recvmsg->mtext); 

out:
    /* 移除 Message Queue */
    msgctl(msgqid, IPC_RMID, NULL); 
    system("ipcs -q"); /* 再次確認已移除 */ 

    free(sendmsg);
    free(recvmsg);
    exit(rval);
}