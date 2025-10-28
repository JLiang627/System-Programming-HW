/*
 * 課程：網路系統程式設計 (Network System Programming) Quiz 2
 * 題目：Producer-Consumer (Process Version)
 *
 * * Bug 修正紀錄：
 * 1. (已修正) parent_sigchld_handler 必須使用 while(waitpid(...) > 0)，
 * 否則 waitpid 回傳 -1 時會導致無限迴圈。
 * 2. (已修正) producer_main 在等待 child 結束前，必須用 setitimer 關閉計時器，
 * 否則 SIGALRM 會持續中斷 wait() 導致無限迴圈。
 * 3. (本次修正) 修正 `producer_sigalrm_handler` 和 `parent_sigchld_handler` 
 * 內容錯置的問題。
 */

#define _GNU_SOURCE // for setpgid
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork, pause, getpid, setpgid, lseek, write, fsync
#include <signal.h>     // sigaction, killpg, sigemptyset, sig_atomic_t
#include <sys/wait.h>   // waitpid, wait
#include <errno.h>      // errno
#include <string.h>     // snprintf, strcmp
#include <sys/stat.h>   // S_IRUSR...
#include <fcntl.h>      // open, O_CREAT...
#include <sys/time.h>   // setitimer, struct itimerval
#include <time.h>       // time, srand, rand

#define WRITE_LIMIT 10  // 總共寫入次數 (如 user code)
#define BUF_SIZE 16

// --- Global variables for Signal Handlers ---

/* * 使用 volatile sig_atomic_t 是唯一安全的方式
 * 讓 signal handler 與 main loop 溝通。
 */
// Producer 用的旗標
static volatile sig_atomic_t g_sigalrm_received = 0;
// Consumer 用的旗標
static volatile sig_atomic_t g_sigusr1_received = 0;


// --- Signal Handlers ---

// Producer (Parent) 的 SIGALRM handler
// 它的唯一工作就是設定旗標
void producer_sigalrm_handler(int signo) {
    g_sigalrm_received = 1;
}

// Consumer (Child) 的 SIGUSR1 handler
void consumer_sigusr1_handler(int signo) {
    g_sigusr1_received = 1;
}

// Parent (Producer) 的 SIGCHLD handler (用來回收殭屍)
// 它的工作是呼叫 waitpid() 安全地回收 Child
void parent_sigchld_handler(int signo) {
    int old_errno = errno; // 保存 errno
    
    // 只有在 waitpid 回傳值 > 0 (表示成功回收) 時才繼續迴圈
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // do nothing
    }
    
    errno = old_errno; // 恢復 errno
}

// --- Consumer (Child) ---

void consumer_main(const char *filename) {
    struct sigaction sa_usr1;
    int child_sum = 0;
    pid_t my_pid = getpid();

    // 1. 設定 SIGUSR1 handler
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sa_usr1.sa_handler = consumer_sigusr1_handler;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("child sigaction SIGUSR1");
        _exit(1);
    }

    // 2. Consumer 主迴圈
    while (1) {
        // 3. 使用 pause() 等待 signal
        pause();

        // 4. Signal 觸發後，旗標被設定，我們在主迴圈中安全地處理 I/O
        if (g_sigusr1_received) {
            g_sigusr1_received = 0; // 重設旗標

            int fd = open(filename, O_RDONLY);
            if (fd == -1) {
                // 檔案可能還沒被 Producer 建立，或權限問題
                perror("consumer open");
                continue; 
            }

            char buf[BUF_SIZE];
            ssize_t bytes_read = read(fd, buf, BUF_SIZE - 1);
            close(fd);

            if (bytes_read > 0) {
                buf[bytes_read] = '\0'; // 確保字串結尾

                // 5. 檢查終止符號
                if (buf[0] == '#') {
                    // 收到 '#', 印出總和並結束
                    printf("Consumer %d: %d\n", my_pid, child_sum);
                    _exit(0); // 正常結束
                } else {
                    // 6. 累加總和
                    child_sum += atoi(buf);
                }
            }
        }
    }
}

// --- Producer (Parent) ---

void producer_main(int num_child, pid_t child_pgid, float interval, const char *filename) {
    int outputfd;
    struct sigaction sa_alrm, sa_chld;
    struct itimerval timer;
    int write_count = 0;

    // 1. 開啟檔案
    outputfd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (outputfd == -1) {
        perror("producer open");
        // 通知所有 child 結束，因為 producer 失敗了
        killpg(child_pgid, SIGTERM);
        exit(1);
    }

    // 2. 設定 SIGALRM handler (for timer)
    sigemptyset(&sa_alrm.sa_mask);
    sa_alrm.sa_flags = 0;
    sa_alrm.sa_handler = producer_sigalrm_handler; // <--- 使用正確的 handler
    if (sigaction(SIGALRM, &sa_alrm, NULL) == -1) {
        perror("producer sigaction SIGALRM");
        exit(1);
    }

    // 3. 設定 SIGCHLD handler (for reaping zombies)
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART; // 避免 pause() 被 EINTR 中斷
    sa_chld.sa_handler = parent_sigchld_handler; // <--- 使用正確的 handler
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("producer sigaction SIGCHLD");
        exit(1);
    }

    // 4. 設定「週期性」計時器
    long sec = (long)interval;
    long usec = (long)((interval - sec) * 1000000.0);
    
    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = usec;
    timer.it_interval.tv_sec = sec;     // 關鍵：設定 interval 才會重複
    timer.it_interval.tv_usec = usec;

    srand(time(NULL));

    // 5. 啟動計時器
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }

    // 6. Producer 主迴圈
    while (write_count <= WRITE_LIMIT) {
        
        // 7. 使用 pause() 等待 signal (等待 SIGALRM)
        pause();

        // 8. Signal 觸發後，旗標被設定，我們在主迴圈中安全地處理 I/O
        if (g_sigalrm_received) {
            g_sigalrm_received = 0; // 重設旗標
            write_count++;

            char buf[BUF_SIZE];

            // 9. 準備寫入內容
            if (write_count > WRITE_LIMIT) {
                // 寫入終止符號 '#'
                snprintf(buf, BUF_SIZE, "#");
            } else {
                // 寫入 1-9 隨機數
                int random_number = (rand() % 9) + 1;
                snprintf(buf, BUF_SIZE, "%d", random_number);
            }

            // 10. 覆寫檔案：移到開頭
            if (lseek(outputfd, 0, SEEK_SET) == -1) {
                perror("producer lseek");
                continue;
            }
            
            if (write(outputfd, buf, strlen(buf)) == -1) {
                perror("producer write");
                continue;
            }

            // 確保資料「立刻」寫入磁碟，避免 race condition
            // (即 consumer 讀到舊資料)
            fsync(outputfd);

            // 11. 通知「所有」Consumers
            if (killpg(child_pgid, SIGUSR1) == -1) {
                // (如果 child_pgid < 0 會失敗，但在這裡應該都 > 0)
                // perror("killpg");
            }
        }
    }
    
    // 12. 停止計時器 (Bug 2 修正)
    struct itimerval stop_timer;
    stop_timer.it_value.tv_sec = 0;
    stop_timer.it_value.tv_usec = 0;
    stop_timer.it_interval.tv_sec = 0;
    stop_timer.it_interval.tv_usec = 0;
    
    // 使用這個全零的結構來「停用」計時器
    if (setitimer(ITIMER_REAL, &stop_timer, NULL) == -1) {
        perror("disable setitimer");
    }

    close(outputfd);
    printf("Producer: Wrote 10 times and sent termination '#'. Waiting for consumers to exit...\n");

    // 13. 等待所有 Child Process 結束
    // (現在 SIGALRM 不會再觸發，這個迴圈可以正常運作了)
    while (wait(NULL) > 0 || errno == EINTR) {
        // do nothing
    }

    printf("Producer: All consumers exited. Exiting.\n");
}


// --- Main Function ---

int main(int argc, char *argv[]) {
    const char *output_filename = NULL;
    int num_child = 0;
    float interval_time = 0.0;
    pid_t child_pgid = -1; // 儲存子行程群組 ID

    // 1. 參數解析
    if (argc != 4) {
        fprintf(stderr, "Usage: %s num_consumer interval filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_child = atoi(argv[1]);
    interval_time = atof(argv[2]);
    output_filename = argv[3];

    if (num_child <= 0 || interval_time <= 0) {
        fprintf(stderr, "Error: num_consumer and interval must be positive numbers.\n");
        return EXIT_FAILURE;
    }
    
    // 2. Fork N 個 Consumers
    for (int i = 0; i < num_child; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            // 終止已經建立的 child (如果有的話)
            if (child_pgid > 0) killpg(child_pgid, SIGTERM);
            exit(1);

        } else if (pid == 0) {
            // --- Child (Consumer) ---
            // 關鍵：Parent 會設定我們的 PGID，我們直接執行 main logic
            consumer_main(output_filename);
            _exit(0); // 確保 child 執行完 consumer_main 後一定會結束

        } else {
            // --- Parent (Producer) ---
            // 3. 設定 Process Group
            if (i == 0) {
                // 第一個 child 成為 group leader (由 parent 設定)
                child_pgid = pid;
                if (setpgid(pid, child_pgid) == -1) {
                     perror("parent setpgid leader");
                }
            } else {
                // 其他 child 加入 group
                if (setpgid(pid, child_pgid) == -1) {
                    perror("parent setpgid join");
                }
            }
        }
    }

    // 4. 所有 Child 都 fork 完成，Parent 進入 producer logic
    producer_main(num_child, child_pgid, interval_time, output_filename);

    return EXIT_SUCCESS;
}
