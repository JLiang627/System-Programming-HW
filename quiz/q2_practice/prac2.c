// 練習題 2：父子行程與 SIGCHLD 處理

// 主題： 使用 fork()、sigaction()、SIGCHLD、sigemptyset()、sigaddset()

// 題目說明：
// 請撰寫一個程式，模擬父行程建立子行程的情境，並要求：
// 子行程執行結束後，父行程必須收到 SIGCHLD。
// 在父行程中，使用 sigaction() 安裝一個 SIGCHLD 處理函式，用以回收已結束的子行程（透過 waitpid()）。
// 為了避免在子行程結束前接收到其他訊號，使用 sigprocmask 建立 mask，封鎖除 SIGCHLD 外的所有訊號。
// 程式必須印出訊息顯示「子行程結束，父行程成功回收」。
// 重點：理解如何正確使用 sigchild handler + mask 組合。
#define _GNU_SOURCE
#include <signal.h>   // sigaction, sigset_t, sig... functions
#include <sys/types.h>  // pid_t
#include <unistd.h>   // fork(), sleep(), pause(), write(), STDOUT_FILENO, getpid()
#include <stdio.h>    // perror, printf
#include <stdlib.h>   // exit
#include <sys/wait.h>   // waitpid, WNOHANG

void handler_chld(int signo) {
    int status;
    pid_t pid;
    // WNOHANG = (Wait No Hang) 立即返回，不要掛起
    // -1 = 等待任何一個子行程
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        write(STDOUT_FILENO, "Handler: Reaped child.\n", 23);
    }
}


int main(){
    struct sigaction sa;
    sigset_t block_mask, old_mask;
    sa.sa_handler = handler_chld;
    
    sigemptyset(&sa.sa_mask); 
    // 讓被中斷的系統呼叫自動重啟
    sa.sa_flags = SA_RESTART; 

    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction SIGCHLD");
        exit(1);
    }

    // 先建立一個「全滿」的遮罩 (封鎖所有訊號)
    sigfillset(&block_mask);
    // 在遮罩上「挖一個洞」，允許 SIGCHLD 通過
    sigdelset(&block_mask, SIGCHLD);



    pid_t pid = fork();
    if (pid == 0){
        printf("child pid:%d\n", getpid());
        sleep(2);
        exit(0);
    }
    else if(pid>0){
        sigprocmask(SIG_BLOCK, &block_mask, &old_mask);
        printf("Parent: waiting for child...\n");
        pause();
        printf("parent: waking up from pausing...\n");
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
    }else{
        perror("fork error");
        exit(1);
    }

    return 0;
}

