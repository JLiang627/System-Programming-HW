#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

void PrintUsage(const char *prog_name);
void errExit(const char *msg);

int main(int argc, char *argv[]){
    int inputFd, outputFd, openFlags;
    ssize_t numRead;
    char *buf;
    off_t Dataoffset=0, Holeoffset=0, off_set=0;
    mode_t filePerms;

    buf = malloc(BUF_SIZE);
    if (buf == NULL) errExit("malloc");
    
    if (argc < 3 || strcmp(argv[1], "--help") == 0 )
        PrintUsage(argv[0]);
    
    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // rw-rw-r--

    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) errExit("open input");
    outputFd = open(argv[2], openFlags, filePerms);
    if (outputFd == -1) errExit("open output");
    
    off_t offset = 0;
    struct stat st;
    if (fstat(inputFd, &st) == -1) errExit("fstat");
    off_t fileSize = st.st_size;

    while (offset < fileSize) {
        off_t dataPos = lseek(inputFd, offset, SEEK_DATA);
        if (dataPos == -1) break;

        off_t holePos = lseek(inputFd, dataPos, SEEK_HOLE);
        if (holePos == -1) holePos = fileSize;

        off_t dataLen = holePos - dataPos;
        if (lseek(inputFd, dataPos, SEEK_SET) == -1) errExit("lseek input");

        while (dataLen > 0) {
            ssize_t toRead = (dataLen > BUF_SIZE) ? BUF_SIZE : dataLen;
            ssize_t numRead = read(inputFd, buf, toRead);
            if (numRead <= 0) errExit("read");
            ssize_t numWritten = write(outputFd, buf, numRead);
            if (numWritten != numRead) errExit("write");
            dataLen -= numRead;
        }

        off_t nextDataPos = lseek(inputFd, holePos, SEEK_DATA);
        if (nextDataPos == -1) nextDataPos = fileSize;

        off_t holeLen = nextDataPos - holePos;
        if (holeLen > 0) {
            if (lseek(outputFd, holeLen, SEEK_CUR) == -1) errExit("lseek hole");
        }
        offset = nextDataPos;
    }


    free(buf);
    if (close(inputFd) == -1 ) errExit("close input");
    if (close(outputFd) == -1 ) errExit("close output");
    exit(EXIT_SUCCESS);
}


void PrintUsage(const char *prog_name){
	fprintf(stderr, "Usage: %s [-a] output_file\n", prog_name);
	exit(EXIT_FAILURE);
}

void errExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
