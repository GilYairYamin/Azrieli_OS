#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

void carch_int(int sig) {
    printf("I don't want to quit\n");
}

int main() {
    signal(SIGINT, carch_int);
    while (true);
}