#include "packet.h"
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

void node_main(int id){
    struct sockaddr_in server;
    struct hostent *host;
    packet_t pack;
    int sd;

    sleep(1);  // 為確保router準備好listen

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("node: socket");
        exit(1);
    }


    // get server 位址
    if ((host = gethostbyname("localhost")) == NULL) {
        perror("node: gethostbyname");
        exit(1);
    }

    memset((char *) &server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTNUM);
    memcpy((char *)&server.sin_addr, host->h_addr, host->h_length);


    // 建立跟server的連線
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("node: connect");
        exit(1);
    }

    // 直接write參數id
    write(sd, &id, sizeof(id));

    // Receive Loop
    while (1) {
        if (read(sd, &pack, sizeof(pack)) <= 0) {
            break; // Router 關閉或連線中斷
        }
        // 題目要求的輸出格式
        printf("Node %d receives \"%s\"\n", id, pack.msg);
        fflush(stdout);
    }

    close(sd);
    exit(0); 
}