/* Source: Module 9, Slide 2 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DIE(x) perror(x),exit(1)

int main() {
    /* 建立 FIFO1，使用 mknod (舊式 System V 方法) */
    /* 0666 代表權限 rw-rw-rw- */
    if (mknod("FIFO1", S_IFIFO | 0666, 0) == -1)
        DIE("FIFO1");

    /* 建立 FIFO2，使用 mkfifo (POSIX 標準方法) */
    if (mkfifo("FIFO2", 0666) == -1)
        DIE("FIFO2");

    printf("FIFOs created successfully.\n");
    return 0;
}