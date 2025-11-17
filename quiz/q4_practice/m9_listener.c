/* Source: Module 9, Slides 5-6 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 80 

// 執行前在terminal輸入mkfifo mypipe

int main() {
    int pd, n = 0;
    char inmsg[BUFSIZE];
    char introMessage[] = "Listener heard:";

    /* 打開 pipe 進行讀取 (Read Only) */
    pd = open("./mypipe", O_RDONLY); 

    if (pd == -1) {
        perror("open");
        exit(1);
    }

    write(STDOUT_FILENO, introMessage, strlen(introMessage)); 

    /* 持續讀取直到 pipe 關閉 */
    while ((n = read(pd, inmsg, BUFSIZE)) > 0) { 
        write(STDOUT_FILENO, inmsg, n); 
    }

    if (n == -1) {
        perror("read");
        exit(1);
    }

    write(STDOUT_FILENO, "\n", 1);
    close(pd);
    return 0;
}