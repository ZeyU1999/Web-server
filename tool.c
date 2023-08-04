#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "tool.h"

extern int sigint_flag;

void handle_sigint(int sigint_flag) {
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
    exit(0);
}

void load_sigint() {
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}