#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

void sigchld_handler(int sig){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int signum) {
    write(1, "\n", 1);
    return;
}

void setup_signals() {
    struct sigaction sa_chld, sa_int;

    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
}