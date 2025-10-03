#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>

#define BUF_SIZE 1024

void PrintUsage(const char *prog_name);
void errExit(const char *msg);

int main(int argc, char *argv[]){
	int inputFd, outputFd, openFlags, opt;
	char *buf;
	ssize_t numRead;

	buf = malloc(BUF_SIZE);
	if (buf == NULL) errExit("malloc");

	if (argc < 2 || strcmp(argv[1], "--help") == 0 )
		PrintUsage(argv[0]);

	//default overwrite
	openFlags = O_CREAT | O_WRONLY | O_TRUNC;

	//detect the '-a' 
	while ((opt = getopt(argc, argv, "a")) != -1) {
    	switch (opt) {
        	case 'a':
				openFlags = O_CREAT | O_WRONLY | O_APPEND;
				break;
        	default:
           		break;
    	}
	}

	//build the file descriptor
	inputFd = STDIN_FILENO;
	
	outputFd = open(argv[optind],openFlags , 0644);
	if (outputFd == -1) errExit("open");

	//transfer data til we encounter end of input or an error

	while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0){
		if (write(outputFd, buf, numRead) != numRead)
			errExit("couldn't write whole buffer");
		if (write(STDOUT_FILENO, buf, numRead) != numRead)
			errExit("write to stdout");
	}

	free(buf);
	if (numRead == -1) errExit("read");
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
