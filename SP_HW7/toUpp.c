#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define bufsize 4048

int main(){
    // 變數宣告區
    int p2c[2];
    int c2p[2];
    char buf[bufsize];

    int numRead, c2pReadCnt;

    if (pipe(p2c) == -1){
        fprintf(stderr, "p2c pipe failed");
        exit(0);
    }

    if (pipe(c2p) == -1){
        fprintf(stderr, "c2p pipe failed");
        exit(0);
    }

    int pid = fork();



    if (pid <0){
        fprintf(stderr, "fork failed");
        exit(0);
    }else if (pid == 0){
        close(c2p[0]);
        close(p2c[1]);

        while ((numRead = read(p2c[0], buf, bufsize)) > 0){

            for (int i = 0; i < numRead ; i++){
                buf[i] = toupper(buf[i]);
            }

            if (write(c2p[1], buf, numRead) == -1){
                fprintf(stderr, "child write c2p failed");
                exit(0);
            }
            
        }
        close(p2c[0]);
        close(c2p[1]);        
        exit(0);

    }else{
        close(c2p[1]);
        close(p2c[0]);
        printf("starting type some words here: ");
        fflush(stdout);
        while ((numRead = read(STDIN_FILENO, buf, bufsize)) > 0){
            
            if (write(p2c[1], buf, numRead) == -1){
                fprintf(stderr, "parent write p2c failed");
                exit(0);
            }

            if ((c2pReadCnt = read(c2p[0], buf, bufsize)) == -1){
                fprintf(stderr, "parent read c2p failed");
                exit(0);
            }
            printf("Upper case: ");
            fflush(stdout);
            write(STDOUT_FILENO, buf, c2pReadCnt);
            write(STDOUT_FILENO, "\n", 1);
            printf("starting type some words here: ");
            fflush(stdout);
        }
        close(p2c[1]);
        close(c2p[0]);

        // 等待child
        wait(NULL);
    }

    return 0;
}