/*
 * 練習題：行程群組 (Process Group) 控制
 *
 * 題目要求：
 * 1. 建立一個父行程 (Manager) 和 3 個子行程 (Workers)。
 * 2. 父行程必須攔截 SIGINT (Ctrl+C)。
 * 3. 所有的「子行程」必須被歸到「同一個新的行程群組」。
 *    (提示：讓第一個子行程成為 Group Leader)。
 * 4. 3 個子行程都要進入無限迴圈 (例如 sleep(1)) 模擬持續工作。
 * 5. 當父行程收到 SIGINT (Ctrl+C) 時，父行程的 handler 必須：
 * a. 印出「收到 Ctrl+C！正在清理所有子行程...」
 * b. 使用 killpg() 發送 SIGKILL，一次性終止「整個子行程群組」。
 * c. 等待（回收）所有子行程。
 * d. 印出「清理完畢」，然後父行程自己終止。
 *
 * 涵蓋範圍：fork(), setpgid(), killpg(), signal() (sigaction), pause(), waitpid()
 */

// 啟用 POSIX 功能
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork, pause, getpid, setpgid
#include <signal.h>     // sigaction, killpg
#include <sys/wait.h>   // waitpid
#include <errno.h>      // errno
#include <string.h>     // strlen

#define NUM_WORKERS 3

// 需要全域變數來讓 handler 存取
static pid_t child_group_id = 0;
static volatile sig_atomic_t cleanup_done = 0;

void manager_sigint_handler(int signo) {
    const char *msg1 = "\n[Manager Handler] 收到 Ctrl+C！正在清理所有子行程...\n";
    write(STDOUT_FILENO, msg1, strlen(msg1));
    
    if (child_group_id > 0) {
        // 5.b: 一次性終止「整個」子行程群組
        if (killpg(child_group_id, SIGKILL) == -1) {
            perror("killpg");
        }
    }
    
    // 5.c: 等待並回收所有子行程
    int status;
    int reaped_count = 0;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        reaped_count++;
    }
    
    const char *msg2 = "[Manager Handler] 清理完畢。\n";
    write(STDOUT_FILENO, msg2, strlen(msg2));
    
    // 5.d: 終止父行程
    _exit(0);
}

int main() {
    struct sigaction sa;
    pid_t pid;
    
    printf("[Manager] PID %d (PGID %d)\n", getpid(), getpgrp());

    // 2. 父行程攔截 SIGINT
    sa.sa_handler = manager_sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // 讓 pause() 之外的呼叫重啟
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    // 3. 建立子行程
    for (int i = 0; i < NUM_WORKERS; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            // 已經 fork 出來的子行程會變成孤兒，
            // 這裡為了簡化，直接退出
            exit(1);
        }

        /* --- 子行程邏輯 --- */
        if (pid == 0) {
            pid_t my_pid = getpid();
            
            if (i == 0) {
                // ** 關鍵：第一個子行程成為新的「群組領導者」 **
                // setpgid(0, 0) -> 讓自己的 PGID = 自己的 PID
                setpgid(0, 0);
            } else {
                // ** 關鍵：後續的子行程「加入」該群組 **
                // setpgid(0, child_group_id) -> 加入領導者的群組
                if (setpgid(0, child_group_id) == -1) {
                    perror("setpgid");
                    _exit(1);
                }
            }
            
            printf("[Worker %d] PID %d, PGID %d (已啟動)\n", i, my_pid, getpgrp());
            
            // 4. 模擬持續工作
            while (1) {
                sleep(1);
            }
            // 子行程永遠不會執行到這裡
        }
        
        /* --- 父行程邏輯 (在迴圈中) --- */
        if (i == 0) {
            // 3. 儲存第一個子行程的 PID，它就是我們 PGID
            child_group_id = pid;
        }
    }
    
    /* --- 父行程邏輯 (迴圈後) --- */
    printf("\n[Manager] %d 個 Worker 已啟動於 PGID: %d\n", NUM_WORKERS, child_group_id);
    printf("[Manager] 等待 Ctrl+C 來終止所有 Worker...\n\n");

    // 5. 等待 SIGINT
    // 父行程進入睡眠，等待訊號
    while(1) {
        pause(); 
    }
    
    // 程式永遠不會執行到這裡，因為 handler 會呼叫 _exit()
    return 0;
}
