#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>

bool isPrime(int num) {
    for (int i = 2; i <= num / 2; i++) {
        if (num % i == 0)
            return false;
    }
    return true;
}

int generateRandom(int otherPID) {
    srand(time(NULL) * getpid());

    int num = rand();
    while (!isPrime(num))
        num = rand();

    printf("ID: %d, Prime: %d\n", getpid(), num);
    kill(otherPID, SIGKILL);
}

int main() {
    int fatherID = getpid();
    int sonID = fork();

    if (sonID > 0) {
       // father proccess
        printf("Father process: %d\nSon process: %d\n", fatherID, sonID);
        generateRandom(sonID);
    }
    else if (sonID == 0) {
         // son proccess.
        generateRandom(fatherID);
    }
    else {
        perror("- fork failed");
        exit(1);
    }
}