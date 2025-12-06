#include "packet.h"
#include <stdio.h>

void router_main(){
    // 變數宣告
    int sd;
    int ns;
    struct sockaddr_in socketname, client;
    socklen_t clientlen = sizeof(client);
    
    // index 0不用 只用1, 2, 3
    int client_arr[4]= {0};
    int cnt_connection = 0;

    // 建立socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("router: socket");
        exit(1);
    }

    int opt = 1;
    // 允許立即重用 Port 
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("router: setsockopt");
        exit(1);
    }

    // bind socket
    memset((char*) &socketname, '\0', sizeof(socketname));  // 先清空socketname
    socketname.sin_family = AF_INET;
    socketname.sin_port = htons(PORTNUM);
    socketname.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr*) &socketname, sizeof(socketname)) == -1){  // 把前面準備好的socketname bind 到sd
        perror("router: bind");
        exit(1);
    }

    if (listen(sd, 3) == -1){
        perror("router: listen");
        exit(1);
    }


    // 建立3個node的連線
    while (cnt_connection < 3){
        if ((ns = accept(sd, (struct sockaddr *) &client, &clientlen)) == -1) {
            perror("router: accept");
            exit(1);
        }

        int id;
        read(ns, &id, sizeof(id));
        
        if (id >= 1 && id <=3){
            client_arr[id] = ns;
            cnt_connection++;
        }else {
            close(ns);
        }
    }

    // input number
    int sender, receiver;
    packet_t pack;
    while (1){
        printf("Please input the sender and the receiver: ");
        fflush(stdout);

        if (scanf("%d %d", &sender, &receiver) != 2) break;

        // 建立前面輸入的id
        pack.receiver_id = receiver;
        pack.sender_id = sender;

        if (receiver == 0){
            sprintf(pack.msg, "%d sends to all", sender);
        }else {
            sprintf(pack.msg, "%d sends to %d", sender, receiver);
        }

        if (receiver == 0){
            for (int i = 0; i <= 3; i++){
                if (client_arr[i]!=0) write(client_arr[i], &pack, sizeof(pack));
            }
        }else{
            if (client_arr[receiver]!=0) write(client_arr[receiver], &pack, sizeof(pack));
        }
        usleep(50000);  // 增加一點延遲 讓NODE有時間印出
    }
    for (int i = 1; i<=3; i++)
        if (client_arr[i] != 0) close(client_arr[i]);
    close(sd);
}