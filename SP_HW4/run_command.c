/*
 * run_command.c :    do the fork, exec stuff, call other functions
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h> 
#include "shell.h"
#include <string.h> 

void run_command(char **myArgv) {
    pid_t pid;
    int stat;
    int is_bg;
    is_bg = is_background(myArgv); /* Check if command is to be run in background. */
    
    for (int i = 0; myArgv[i] != NULL; i++) {
        printf("[%d] : %s\n", i, myArgv[i]);
    }
    
    /* Create a new child process. */
    pid = fork();
    switch (pid) {

        /* Error. */
        case -1 :
            perror("fork");
            exit(errno);

        /* Parent. */
        default :
            /* Wait for child to terminate. */
            if (is_bg){
                return;
            }
            
            if (waitpid(pid, &stat, 0) == -1) {
                perror("waitpid");
                exit(errno);
            }

            /* Display exit status with detailed checks. */
            if (WIFSIGNALED(stat)) {
                printf("Child terminated by signal %d.\n", WTERMSIG(stat));
            } else if (WIFEXITED(stat)) {
                printf("Child exited with status %d.\n", WEXITSTATUS(stat));
            } else if (WIFSTOPPED(stat)) {
                printf("Child stopped by signal %d.\n", WSTOPSIG(stat));
            }

            return;

        /* Child. */
        case 0 :
                /* Print myArgv in child process. */
            // for (int i = 0; myArgv[i] != NULL; i++) {
            //     printf("[%d] : %s\n", i, myArgv[i]);
            // }
            if (strcmp(myArgv[0], "sleep") == 0) {
                // 使用 execvp 呼叫 sleep 指令
                if (strcmp(myArgv[2], "&") == 0) 
                    myArgv[2] = NULL; // 把 & 從參數移除
                if (execvp(myArgv[0], myArgv) == -1) {
                    perror("execvp");
                    exit(errno);
                }
            }

            /* Run command in child process. */
            if (strcmp(myArgv[0], "ls") == 0){
                execlp("ls", "ls", "-l", (char *)NULL);
                perror("execlp");
                exit(errno);
            }

            if (strcmp(myArgv[0], "ps") == 0) {
                execlp("ps", "ps", "aux", (char *)NULL);
                perror("execlp");
                exit(errno);
            }
            /* Handle error return from exec */
    }
}
