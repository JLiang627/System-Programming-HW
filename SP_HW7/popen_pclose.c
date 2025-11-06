#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

static pid_t *pid_record;
static int max_fds;

// 實作mypopen
FILE *mypopen(const char *command, const char *type) {
	// 參數宣告區
	int fd[2];
	pid_t child_pid;

    // 初始化 record
    if (pid_record == NULL) {
        max_fds = sysconf(_SC_OPEN_MAX); // 取得系統 fd 上限
        pid_record = calloc(max_fds, sizeof(pid_t));
        
        if (pid_record == NULL) {
            errno = ENOMEM; // 記憶體不足
            return NULL; 
        }
    }

    if (strcmp(type, "w") != 0 && strcmp(type, "r") != 0) {
	    errno = EINVAL;
	    return NULL;
	}

    if (pipe(fd) == -1){
    	fprintf(stderr, "pipe failed");
    	return NULL;
    }

    int pread_flag=1;  //預設parent會是read
    if (strcmp(type, "r") != 0) pread_flag = 0;  //如果不是的話再設false

    // 步驟 4：fork()
    child_pid = fork();
    switch (child_pid){
    case -1:
    	fprintf(stderr, "fork failed");
    	close(fd[0]);
    	close(fd[1]);
    	return NULL;
    	break;
    case 0:
    	if (pread_flag){
    		close(fd[0]);
			if (dup2(fd[1], STDOUT_FILENO) == -1) {
                perror("child dup2 failed");
                _exit(127);
            }	    	
            close(fd[1]);
    	}else{
			close(fd[1]); 
            if (dup2(fd[0], STDIN_FILENO) == -1) {
                perror("child dup2 failed");
                _exit(127);
            }
            close(fd[0]);
    	}
    	

    	for (int i = 0; i < max_fds; i++) {
			if (pid_record[i] > 0) {
		     	close(i);
			}
		}

		if ((execl("/bin/sh", "sh", "-c", command, (char *) NULL)) == -1){
			perror("execl failed");
			_exit(127);
		}
    	break;
    default:
        if (pread_flag) {
            close(fd[1]); 
            // 將child PID 存入 record
            pid_record[fd[0]] = child_pid; 
            
            FILE *stream = fdopen(fd[0], "r");
            if (stream == NULL) {
                close(fd[0]);
                waitpid(child_pid, NULL, 0);
            }
            return stream;

        } else {
            close(fd[0]); 
            // 將child PID 存入 record
            pid_record[fd[1]] = child_pid;

            FILE *stream = fdopen(fd[1], "w");
            if (stream == NULL) {
                close(fd[1]);
                waitpid(child_pid, NULL, 0);
            }
            return stream;
        }
    }
    return NULL; 
}

// 實作mypclose
int mypclose(FILE *stream) {
	int fd = fileno(stream);
	if (fd < 0 || fd >= max_fds || pid_record[fd] <= 0) {
        errno = EBADF; 
        return -1;     
    }
    pid_t child_pid = pid_record[fd];
    fclose(stream);

    int status;
    pid_t wait_ret;

    do {
        wait_ret = waitpid(child_pid, &status, 0); 
    } while (wait_ret == -1 && errno == EINTR);

    // 檢查 waitpid 是否真的出錯
    if (wait_ret == -1) {
        errno = ECHILD;
        return -1;
    }

    pid_record[fd] = 0;
    return status; 
}