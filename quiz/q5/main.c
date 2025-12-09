#include "packet.h"

int main(int argc, char *argv[]) {

    /*q1 logic*/
    // 用fork來建立三個node
    if (argc == 1){        
        for (int i = 1; i <= 3; i++) {
            pid_t pid = fork();
            
            if (pid == 0) {
                node_main(i, 0);
                exit(0);
            }
        }
        
        // 建立完node之後 執行router
        router_main(0, 0, 0);
    }else{
        
        /*q2 logic*/
        if (argc != 4 || strcmp(argv[1], "help") == 0){
            fprintf(stderr, "Usage: a.out N[children count] t[time interval] R[integers from each client]\n");
            exit(1);
        }

        int N = atoi(argv[1]);  // how many children
        int t = atoi(argv[2]);  // how much time interval
        int R = atoi(argv[3]);  // how many random numbers from client

        for (int i = 1; i <= N; i++) {
            pid_t pid = fork();
            
            if (pid == 0) {
                node_main(i, R);  // node_main receives the random num from router
                exit(0);
            }
            
        }
        router_main(N, t, R);  // let the router be the client and send random num to children
        router_main(N, t, R);
        
        sleep(3);

        for (int i = 1; i <= N; i++) {
            wait(NULL);
        }
    }
    return 0;
}