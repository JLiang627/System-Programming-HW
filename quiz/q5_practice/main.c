#include "packet.h"

int main() {
    // 用fork來建立三個node
    for (int i = 1; i <= 3; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            node_main(i);
            exit(0);
        }
    }
    
    // 建立完node之後 執行router
    router_main();

    return 0;
}