/* Source: Module 10, Slides 3-5 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 功能與cat相似

int main(int argc, char *argv[]) {
    int fd;
    char *addr; 
    struct stat statbuf;

    if (argc != 2) {
        fprintf(stderr, "Usage: mycat2 filename\n");
        exit(1);
    }

    /* 取得檔案大小資訊 */
    if (stat(argv[1], &statbuf) == -1) {
        perror("stat");
        exit(1);
    }

    /* 開啟檔案 */
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    /* 建立 Memory Mapping */
    /* PROT_READ: 唯讀, MAP_SHARED: 共享變更 */
    addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    /* 檔案描述符可以關閉了，因為映射已經建立 */
    close(fd);

    /* 直接從記憶體讀取並寫入到 stdout (1) */
    write(1, addr, statbuf.st_size);

    return 0;
}