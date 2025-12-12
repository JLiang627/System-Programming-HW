/*
 * lookup8 : does no looking up locally, but instead asks
 * a server for the answer. Communication is by Internet TCP Sockets
 * The name of the server is passed as resource. PORT is defined in dict.h
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "dict.h"

int lookup(Dictrec * sought, const char * resource) {
	static int sockfd;
	static struct sockaddr_in server;
	struct hostent *host;
	static int first_time = 1;
	int n;

	if (first_time) {        /* connect to socket ; resource is server name */
		first_time = 0;
		
		if ((host = gethostbyname(resource)) == NULL) {
			DIE("gethostbyname");
		}
		memset(&server, '\0', sizeof(server));

		/* Set up destination address. */
		server.sin_family = AF_INET;
		server.sin_port = htons(PORT);

		memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
		/* Allocate socket.
		 * Fill in code. */

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)DIE("socket");
		if ((connect(sockfd, (struct sockaddr *)&server, sizeof(server))) == -1)DIE("connect");
		
		/* Connect to the server.
		 * Fill in code. */
	}

	/* write query on socket ; await reply
	 * Fill in code. */

	n = write(sockfd, sought, sizeof(Dictrec));
	if (n!=sizeof(Dictrec)) exit(1);
	
	n = read(sockfd, sought, sizeof(Dictrec));
	if (n!=sizeof(Dictrec)) exit(1);

	if (strcmp(sought->text,"XXXX") != 0) {
		return FOUND;
	}

	return NOTFOUND;
}
