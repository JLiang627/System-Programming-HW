// 練習題 1：Alarm Mask 與 sigsuspend 的應用

// 主題： 使用 sigemptyset()、sigfillset()、sigprocmask()、alarm()、sigsuspend()

// 題目說明：
// 請撰寫一個程式，模擬「設定 5 秒鬧鐘後執行特定動作」的行為。
// 要求如下：
// 安裝一個 SIGALRM 處理函式（例如輸出訊息 "Time’s up!"）。
// 在設定鬧鐘前，使用 sigprocmask 將所有訊號暫時封鎖（mask all signals）。
// 呼叫 alarm(5) 後，使用 sigsuspend() 讓程式暫停，直到 SIGALRM 到達。
// 在 SIGALRM 處理完後，恢復原本的 mask 狀態。
// 重點：練習「如何正確設定 alarm mask」與避免競態條件（race condition）。
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h> // 為了 perror 和 printf

void handler(int signo){
    write(1, "Time's up!\n", 11); //unistd.h
}

int main(){
    struct sigaction sa;
    sigset_t new_mask, old_mask, suspend_mask;    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // 使用預設行為
    sa.sa_handler = handler; // 指定處理函式
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
   
    sigfillset(&new_mask);
    sigemptyset(&suspend_mask);

    printf("Blocking all signals and waiting for alarm...\n");
    if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) == -1) {
        perror("sigprocmask(SIG_BLOCK)");
        exit(1);
    }

    alarm(5);
    sigsuspend(&suspend_mask);
    //Woke up from sigsuspend
    printf("Restoring original mask.\n");
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) == -1) {
        perror("sigprocmask(SIG_SETMASK)");
        exit(1);
    }
    
    printf("Exiting normally.\n");
    return 0;
}