/*
 * usock_server : listen on a Unix socket ; fork ;
 *                child does lookup ; replies down same socket
 * argv[1] is the name of the local datafile
 * PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include "dict.h"
#include <signal.h>

int main(int argc, char **argv) {
    struct sockaddr_un server;
    int sd,cd,n;
    Dictrec tryit;

    if (argc != 3) {
      fprintf(stderr,"Usage : %s <dictionary source>"
          "<Socket name>\n",argv[0]);
      exit(errno);
    }
	signal(SIGCHLD, SIG_IGN);
    /* Setup socket.
     * Fill in code. */
	if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        DIE("server: socket");
    }

    /* Initialize address.
     * Fill in code. */
	memset((char*) &server, '\0', sizeof(server));  // 先清空socketname
	strcpy(server.sun_path, argv[2]);    
	server.sun_family = AF_UNIX;

    /* Name and activate the socket.
     * Fill in code. */
	unlink(argv[2]);
	if (bind(sd, (struct sockaddr*) &server, sizeof(server)) == -1){ 
        perror("server: bind");
        exit(1);
    }

    if (listen(sd, 3) == -1){
        perror("server: listen");
        exit(1);
    }
    /* main loop : accept connection; fork a child to have dialogue */
    for (;;) {
		/* Wait for a connection.
		 * Fill in code. */
		if ((cd = accept(sd, NULL, NULL)) == -1) {
			perror("accept");
			continue; /* 如果 accept 失敗，繼續下一次迴圈 */
    	}
		/* Handle new client in a subprocess. */
		switch (fork()) {
			case -1 : 
				DIE("fork");
			case 0 :
				close (sd);	/* Rendezvous socket is for parent only. */
				/* Get next request.
				 * Fill in code. */
				while (( n = read(cd, &tryit, sizeof(Dictrec)))>0) {
					/* Lookup request. */
					switch(lookup(&tryit,argv[1]) ) {
						/* Write response back to client. */
						case FOUND: 
							/* Fill in code. */
							write(cd, &tryit, sizeof(Dictrec));
							break;
						case NOTFOUND: 
							/* Fill in code. */
							write(cd, &tryit, sizeof(Dictrec));
							break;
						case UNAVAIL:
							DIE(argv[1]);
					} /* end lookup switch */
					
				} /* end of client dialog */

				/* Terminate child process.  It is done. */
				exit(0);

			/* Parent continues here. */
			default :
				close(cd);
				break;
		} /* end fork switch */
    } /* end forever loop */
} /* end main */
