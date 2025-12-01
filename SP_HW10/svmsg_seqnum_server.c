#include "error_functions.h"
#include "svmsg_seqnum_header.h"

static int serverId = -1;

static void removeQueue(void) {
    if (serverId != -1) {
        if (msgctl(serverId, IPC_RMID, NULL) == -1) {
            perror("msgctl - IPC_RMID failed");
        }
    }
    unlink(SERVER_KEY_PATH); 
}

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    struct responseMsg resp;
    ssize_t msgLen;
    size_t respSeq = 0;
    key_t SERVER_KEY;

    if (atexit(removeQueue) != 0)
        errExit("atexit");

    int fd = open(SERVER_KEY_PATH, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        if (errno != EEXIST) {
            errExit("open");
        }
    } else {
        close(fd);
    }

    SERVER_KEY = ftok(SERVER_KEY_PATH, SERVER_KEY_ID);
    if (SERVER_KEY == -1) errExit("ftok");

    /* Create server message queue */
    serverId = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL |
                      S_IRUSR | S_IWUSR | S_IWGRP);
    if (serverId == -1){
        if (errno == EEXIST) {
            serverId = msgget(SERVER_KEY, S_IWUSR);
            if (serverId == -1) errExit("msgget - get existing ID");
            
            printf("[Server] 發現舊佇列，進行移除 (ID: %d)\n", serverId);
            if (msgctl(serverId, IPC_RMID, NULL) == -1)
                errExit("msgctl - IPC_RMID failed for old queue");
            
            serverId = msgget(SERVER_KEY, IPC_CREAT | IPC_EXCL |
                              S_IRUSR | S_IWUSR | S_IWGRP);
            if (serverId == -1)
                errExit("msgget - recreate failed");

        } else {
            errExit("msgget - initial failure");
        }
    }
        
    printf("[Server] server準備好接收資料\n");

    /* Read requests, handle each in a separate child process */
    for (;;) {
        msgLen = msgrcv(serverId, &req, REQ_MSG_SIZE, REQ_MTYPE, 0);
        if (msgLen == -1) {
            errMsg("msgrcv");           /* Some other error */
            break;                      /* ... so terminate loop */
        }

        resp.seqNum = respSeq;
        respSeq += req.seqLen;
        resp.mtype = req.pid;
        /* Parent loops to receive next client request */
        
        if (msgsnd(serverId, &resp, RESP_MSG_SIZE, 0) == -1)
            errMsg("msgsnd: Failed to reply to PID %d", (long)req.pid);
    }

    /* If msgrcv() or fork() fails, remove server MQ and exit */
    if (msgctl(serverId, IPC_RMID, NULL) == -1)
        errExit("msgctl");
    exit(EXIT_SUCCESS);
}