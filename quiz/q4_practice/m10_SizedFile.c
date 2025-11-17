/* Source: Module 10, Slides 6-7 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int fd;
    int pagesize;
    char *addr;

    /* 取得系統 Page Size */
    pagesize = sysconf(_SC_PAGESIZE);
    printf("Page size is %d bytes.\n", pagesize);

    /* 開啟檔案，若不存在則建立，若存在則清空 */
    fd = open("mapfile", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    /* 擴充檔案大小為 6 個 Page */
    if (ftruncate(fd, (off_t)(6 * pagesize)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    /* 顯示目前檔案資訊 */
    system("ls -l mapfile");

    /* 映射檔案 */
    addr = mmap(NULL, 6 * pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    /* 不再需要 fd */
    close(fd);

    /* 像操作記憶體陣列一樣寫入檔案 */
    strcpy(addr, "Test string.\n");

    /* 驗證寫入結果 */
    system("head -1 mapfile");

    return 0;
}