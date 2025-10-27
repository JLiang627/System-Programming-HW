/*
 * 練習題:IPC 管線 (Pipe) 與 setitimer 逾時監控
 *
 * 題目要求：
 * 1. 建立一個父行程和一個子行程，並使用 pipe() 進行單向通訊（子 -> 父）。
 * 2. 子行程：模擬一個耗時 5 秒的任務，完成後透過 pipe 寫入一條 "WORK_DONE" 訊息。
 * 3. 父行程：
    * a. 必須使用 setitimer() 設定一個 3 秒的「逾時鬧鐘」(SIGALRM)。
    * b. 阻塞式地 read() 管線的讀取端。
    * c. 如果在 3 秒內讀到 "WORK_DONE"，印出「任務成功」。
    * d. 如果 3 秒逾時 (SIGALRM 觸發)，handler 必須：
    * i.  印出「任務逾時！」。
    * ii. 使用 kill() 強制終止子行程。
    * iii. 終止父行程。
 * 4. 父行程必須正確回收（waitpid）子行程。
 *
 * 涵蓋範圍：fork(), pipe(), setitimer(), signal() (sigaction), kill(), read() (EINTR)
 */

// 啟用 POSIX 功能
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork, pipe, read, alarm, sleep
#include <signal.h>     // sigaction, kill
#include <sys/wait.h>   // waitpid
#include <sys/time.h>   // setitimer, struct itimerval
#include <string.h>     // strcmp
#include <errno.h>      // errno

// 需要一個全域變數來讓 handler 存取 child_pid
static pid_t child_pid = 0;

void timeout_handler(int signo) {
    // (安全作法：在 handler 中使用 write 而非 printf)
    const char *msg = "[Handler] 任務逾時！正在終止子行程...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    
    if (child_pid > 0) {
        // 4.d.ii: 強制終止子行程
        kill(child_pid, SIGKILL);
    }
    
    // 4.d.iii: 終止父行程
    _exit(1); // _exit 是 signal-safe 的
}

void setup_handler(int sig, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    // 要讓 read() 被中斷，所以不設定 SA_RESTART
    sa.sa_flags = 0; 
    
    if (sigaction(sig, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

int main() {
    int pipe_fd[2]; // pipe_fd[0] = 讀取端, pipe_fd[1] = 寫入端
    char buffer[100];
    ssize_t bytes_read;
    
    // 1. 建立管線
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // 3.a: 安裝 SIGALRM handler
    setup_handler(SIGALRM, timeout_handler);

    // 1. 建立子行程
    child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        exit(1);
    }

    /* --- 子行程邏輯 --- */
    if (child_pid == 0) {
        // 關閉不需要的「讀取端」
        close(pipe_fd[0]);
        
        // 2. 模擬 5 秒的耗時任務
        printf("[Child] PID %d: 開始 5 秒的任務...\n", getpid());
        sleep(5);
        
        // 2. 任務完成，寫入管線
        printf("[Child] 任務完成，發送訊息。\n");
        write(pipe_fd[1], "WORK_DONE", 10); // 包含 \0
        
        // 關閉「寫入端」並退出
        close(pipe_fd[1]);
        exit(0);
    }
    
    /* --- 父行程邏輯 --- */
    // 關閉不需要的「寫入端」
    close(pipe_fd[1]);
    
    printf("[Parent] PID %d: 監控子行程 (PID %d)\n", getpid(), child_pid);
    
    // 3.a: 設定 3 秒的「單次」逾時鬧鐘
    struct itimerval timer;
    timer.it_value.tv_sec = 3;  // 3 秒後觸發
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0; // 不重複
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }

    // 3.b: 阻塞式地等待子行程的訊息
    printf("[Parent] 等待 3 秒... (read() 阻塞中)\n");
    bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
    
    // --- 程式執行到這裡，有兩種可能 ---
    
    if (bytes_read > 0) {
        // **可能 1: 任務成功 (read() 正常返回)**
        
        // 讀取成功，第一件事：取消鬧鐘！
        timer.it_value.tv_sec = 0;
        setitimer(ITIMER_REAL, &timer, NULL); // alarm(0) 的 setitimer 版本

        if (strcmp(buffer, "WORK_DONE") == 0) {
            printf("[Parent] 任務成功！子行程在時限內完成。\n");
        } else {
            printf("[Parent] 收到未知訊息：%s\n", buffer);
        }
        
    } else if (bytes_read == 0) {
        // 子行程關閉了 pipe (但沒寫東西)
        printf("[Parent] 子行程關閉了管線。\n");
        
    } else if (bytes_read == -1) {
        // **可能 2: 任務失敗 (read() 被中斷)**
        
        if (errno == EINTR) {
            // EINTR = Interrupted System Call
            // 這代表 SIGALRM handler 成功中斷了 read()
            // handler 已經執行完畢 (並 kill 了子行程)
            printf("[Parent] read() 被 SIGALRM 中斷。任務逾時。\n");
        } else {
            // 其他 read 錯誤
            perror("read");
        }
    }
    
    // 4. 無論成功或失敗，都要回收子行程
    waitpid(child_pid, NULL, 0);
    printf("[Parent] 已回收子行程，程式結束。\n");
    close(pipe_fd[0]);
    return 0;
}
