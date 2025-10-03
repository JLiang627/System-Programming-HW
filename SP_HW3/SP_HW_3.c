#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 4048

void usageErr(const char *msg) {
    fprintf(stderr, "Usage error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    char buf[BUF_SIZE];
    int fd;
    //預設會輸入tail -n num filename, 抓出num value
    int num_line = atoi(argv[2]);
    off_t file_size, pos, cur_pos;
    int newline_count = 0;
    ssize_t num_read;
    size_t read_size;
    int found = 0;

    if (argc < 4 || strcmp(argv[1], "--help") == 0) {
        usageErr("tail -n num filename");
    }

    fd = open(argv[3], O_RDONLY);
    if (fd == -1) usageErr("open");

    file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0) usageErr("lseek");

    cur_pos = file_size;

    while (cur_pos > 0 && !found) {
        read_size = (cur_pos >= BUF_SIZE) ? BUF_SIZE : cur_pos;
        cur_pos -= read_size;

        if (lseek(fd, cur_pos, SEEK_SET) == -1) usageErr("lseek");
        num_read = read(fd, buf, read_size);
        if (num_read == -1) usageErr("read");

        // 從後往前掃描 buf
        for (ssize_t i = num_read - 1; i >= 0; i--) {
            if (buf[i] == '\n') {
                newline_count++;
                if (newline_count == num_line + 1) { 
                    // 起始位置為該換行的下一字元，所以加一
                    cur_pos += i + 1;
                    found = 1;
                    break;
                }
            }
        }
    }

    // 若找不到夠多的行，就從檔案起點開始
    if (!found) cur_pos = 0;

    if (lseek(fd, cur_pos, SEEK_SET) == -1) usageErr("lseek");

    ssize_t bytes;
    while ((bytes = read(fd, buf, BUF_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, bytes) != bytes) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    return 0;
}
