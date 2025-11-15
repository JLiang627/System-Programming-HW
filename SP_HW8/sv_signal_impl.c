//Implement the System V functions sigset(), sighold(), sigrelse(), sigignore(), andsigpause() using the POSIX signal API.
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "tlpi_hdr.h" // 假設您會使用 tlpi_hdr.h 來輔助


//用於通知main函式
static volatile sig_atomic_t sigCaught = 0;

typedef void (*sighandler_t)(int);

static void handler(int signo)
{



}

//函式宣告
sighandler_t sigset(int sig, sighandler_t handler);
int sighold(int sig);
int sigrelse(int sig);
int sigignore(int sig); 
int sigpause(int sig);



int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);






    return 0;
}

sighandler_t sigset(int sig, sighandler_t handler);
{

}


int sighold(int sig)
{

}

int sigrelse(int sig)
{

}

int sigignore(int sig)
{


}


int sigpause(int sig)
{


}