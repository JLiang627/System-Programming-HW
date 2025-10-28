//P產生in吃放到file 每隔一段時間寫資料進去蓋掉數字 signal其他comsumer去讀取資料，必須要被signal才可以去讀資料
//comsumer有很多個,如果寫進#就代表程式要結束 接著把每個int累加 int都為個位數，每一個consumer把累加結果印在螢幕
//$./a.out n t file,    三個參數t是delta t, 代表間隔時間, 理論上大家都要一樣 但如果cons越來越多 時間越來越小
//可能會造成來不及讀 可能有些cons會loss掉結果
//可以用pipe 或 全域變數 讓child知道檔案名稱
//child 設定成群組 signal比較快
//下次考試一樣題目 但改成thread

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork, pause, getpid, setpgid
#include <signal.h>     // sigaction, killpg
#include <sys/wait.h>   // waitpid
#include <errno.h>      // errno
#include <string.h>     // strlen
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h> 
#include <time.h>

#define write_file_time 10 //寫進檔案次數
#define BUF_SIZE 4


int child_group_id;

void parent_handler(int signo){
    int old_errno = errno; // 儲存 errno，避免 waitpid 汙染
    int status;
    pid_t pid;

    // 使用 WNOHANG 迴圈，回收所有可能已終止的子行程
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    }
    
    // 恢復 errno
    errno = old_errno;
}



int main(int argc, char *argv[]){
    const char *output_filename = NULL;  //檔案名稱
    int num_child;  //child 數量
    float interval_time;  //間隔時間
    struct sigaction sa;
    mode_t filePerms;
    int openFlags, outputfd, child_sum = 0;
    int pipe_fd[2];

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // rw-rw-r--

    if (argc == 4) {
        num_child = atoi(argv[1]);
        output_filename = argv[2];
        interval_time = atof(argv[3]);
        outputfd = open(argv[3], openFlags, filePerms);
        if (outputfd == -1) perror("open");

    } else if (argc != 4) {
        fprintf(stderr, "Usage:  %s n t <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (pipe(pipe_fd) == -1){
        perror("pipe");
        exit(1);
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // 使用預設行為
    sa.sa_handler = parent_handler; // 指定處理函式

    if (sigaction(SIGCHLD ,&sa ,NULL ) == -1){
        perror("sigaction");
        exit(1);
    }
    
    
    // 設定 t 秒的「單次」逾時鬧鐘
    struct itimerval timer;
    timer.it_value.tv_sec = interval_time;  // t 秒後觸發
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0; // 不重複
    timer.it_interval.tv_usec = 0;

    
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }

    for (int i = 0; i<num_child; i++){
        int pid = fork();
        if (pid < 0){  //錯誤處理
            perror("fork");
            exit(1);
        }else if (pid == 0){  //child
            pid_t my_pid = getpid();
            close(pipe_fd[1]);

            if (i == 0){
                setpgid(0, 0);
            }else {
                if (setpgid(0, child_group_id) == -1) {
                    perror("setpgid");
                    _exit(1);
                }
            }
            char buf[BUF_SIZE];
            if (dup2(pipe_fd[0], STDIN_FILENO) == -1){
                perror("dup");
                exit(1);
            }
            close(pipe_fd[0]);
            char buff[BUF_SIZE];
            while (read(pipe_fd[0], buff, BUF_SIZE) != 0 && strcmp(buff, "send") == 0 ){
                *buff = NULL;
                break;
            }
            int bytes_read = read(STDIN_FILENO, buf, BUF_SIZE);
            child_sum += atoi(buf);

            if (buf[0] == '#'){
                printf("Consumer %d: %d\n", my_pid, child_sum);
                close(pipe_fd[0]);
            }
        }else{  //parent
            close(pipe_fd[0]);  
            if (i == 0){
                child_group_id = pid;
            }
            int tmp = 0;
            sleep(3); //等child都fork完成

            while (tmp < write_file_time){  //預設限制10次
                tmp++;
                srand(time(NULL)); // Seed the random number generator with current time
                int random_number = (rand() % 9) + 1;
                char buf[BUF_SIZE];
                buf[0] = random_number;
                write(outputfd, buf, BUF_SIZE);
                if (tmp + 1 == write_file_time){
                    write(outputfd, "#", 1);
                    write(pipe_fd[1], "send", 4);
                    if (child_group_id > 0) {
                        // 終止「整個」子行程群組
                        if (killpg(child_group_id, SIGKILL) == -1) {
                            perror("killpg");
                        }
                    }
                }


            }
            close(pipe_fd[0]); 
            while(1){
                pause();
            }
        }
    }
    
    return 0;
}


