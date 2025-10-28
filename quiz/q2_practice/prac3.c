// 練習題 3：信號控制與強制終止

// 主題： 使用 raise()、sigpause()、SIGSTOP、SIGKILL

// 題目說明：
// 請撰寫一個程式，練習「自我暫停與自我終止」：
// 主行程使用 sigset() 安裝一個處理函式來攔截 SIGINT（Ctrl+C），顯示提示訊息。
// 程式呼叫 raise(SIGSTOP)，使自己暫停（使用 ps 可以看到程式暫停狀態）。
// 之後由另一個終端使用 kill -SIGCONT <pid> 讓程式繼續執行。
// 程式恢復後使用 sigpause() 等待另一個信號（例如 SIGTERM），最後由外部使用 kill -9 (SIGKILL) 強制結束。
// 重點：體驗 raise()、SIGSTOP、SIGKILL、sigpause() 在不同情境下的效果。

// 練習題 3：信號控制與強制終止
#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // write, getpid, pause, raise

void sigint_handler(int signo){
    write(STDOUT_FILENO, "\nCaught SIGINT (Ctrl+C)! Ignoring.\n", 35);
}

int main(){
    struct sigaction sa;

    sa.sa_flags = SA_RESTART; // 讓被中斷的系統呼叫重啟
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask); // 必須初始化

    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction error");
        exit(1);
    }


    // 顯示 PID 並測試 SIGINT 
    printf("My PID is: %d\n", getpid());
    printf("\nStep 1: SIGINT Handler is active.\n");
    printf(">>> Try pressing Ctrl+C now. The handler should run.\n");
    
    // 使用 pause() 等待，直到第一個 Ctrl+C 送達
    // (Handler 執行完後，pause() 會被中斷並返回)
    pause(); 
    
    // --- 3. 呼叫 raise(SIGSTOP) ---
    printf("\nStep 2: Raising SIGSTOP to pause myself.\n");
    printf(">>> Use 'kill -SIGCONT %d' from another terminal to resume.\n", getpid());
    
    raise(SIGSTOP); // 程式會在這裡「暫停」，直到收到 SIGCONT

    // --- 4. 被 SIGCONT 恢復 ---
    printf("\nStep 3: Resumed by SIGCONT!\n");

    // --- 5. 使用 sigpause() (現代版) 等待終止 ---
    // 建立一個「空的」遮罩 
    // 表示願意等待「任何」訊號 (包含 SIGTERM 或 SIGKILL)。
    
    sigset_t suspend_mask;
    sigemptyset(&suspend_mask);

    printf("\nStep 4: Waiting for a termination signal...\n");
    printf(">>> Use 'kill %d' (sends SIGTERM)\n", getpid());
    printf(">>> Or 'kill -9 %d' (sends SIGKILL)\n", getpid());
    
    sigsuspend(&suspend_mask);

    // --- 6. 程式結束 ---
    // 如果收到 SIGINT (Ctrl+C)，handler 會執行，sigsuspend 會返回。
    
    printf("Woke up from sigsuspend (e.g., by another Ctrl+C). Exiting.\n");

    return 0;
}
