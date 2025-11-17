/* Source: Module 10, Slides 14-16 */
#define _XOPEN_SOURCE 500 /* 為了 ftruncate 和 POSIX 功能 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define DIE(x) do { perror(x); exit(1); } while(0)
#define MEMSIZ 256

int main(int argc, char **argv) {
    int shmid, delay;
    /* POSIX Shared Memory 名稱通常以 / 開頭 */
    char *path = "/SHM_DEMO";
    char *shm;

    /* 如果有 -u 參數，則刪除共享記憶體 */
    if (argc == 2 && strcmp(argv[1], "-u") == 0) {
        if (shm_unlink(path) == -1)
            DIE("Failed to unlink shared memory");
        printf("Shared memory unlinked.\n");
        exit(0);
    }

    /* 嘗試開啟已存在的共享記憶體 */
    if ((shmid = shm_open(path, O_RDWR, 0)) == -1) {
        printf("Creating new shared memory segment.\n");
        /* 不存在則建立 */
        if ((shmid = shm_open(path, O_RDWR | O_CREAT, 0666)) == -1)
            DIE("Failed to create shm");
        
        /* 設定大小 */
        if (ftruncate(shmid, MEMSIZ) == -1)
            DIE("Setting the size failed");
    }

    /* 映射到記憶體 */
    if ((shm = mmap(NULL, MEMSIZ, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0)) == MAP_FAILED)
        DIE("mmap failed");

    /* 設定隨機延遲 */
    srand(getpid());

    /* 無窮迴圈：讀取內容 -> 寫入自己的 PID -> 睡覺 */
    for (;;) {
        printf("Shared memory contains: %s\n", shm);
        
        /* 寫入資料 */
        sprintf(shm, "Process %d was here\n", getpid());
        
        delay = rand() % 5;
        sleep(delay);
    }

    return 0;
}