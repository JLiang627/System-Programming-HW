/* Source: Module 9, Slides 3-4 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* 為了 strlen */

#define DIE(x) perror(x),exit(1)

int main() {
    int pd, n;
    char msg[] = "Hi, Folks!";

    printf("Talker's here.\n");

    /* 打開 pipe 進行寫入 (Write Only) */
    /* 注意：這裡檔案名稱是 "mypipe"，必須先存在 */
    pd = open("./mypipe", O_WRONLY);

    if (pd == -1) {
        perror("open");
        exit(1);
    }

    /* 寫入訊息 */
    n = write(pd, msg, strlen(msg) + 1); 
    if (n == -1) {
        perror("write");
        exit(1);
    }

    close(pd);
    return 0;
}