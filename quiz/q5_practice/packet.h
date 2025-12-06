#ifndef PACKET_H
#define PACKET_H
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORTNUM 7000

typedef struct {
    int sender_id;
    int receiver_id;
    char msg[256];
}packet_t;

void router_main();
void node_main(int id);

#endif