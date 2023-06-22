#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define ARR_LEN 20

void sort(int arr[], int len) {
    int temp = 0;
    for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
            if (arr[j] < arr[j - 1]) {
                temp = arr[j];
                arr[j] = arr[j - 1];
                arr[j - 1] = temp;
            }
        }
    }
}

int main() {
    int arr[ARR_LEN];
    srand(time(NULL));
    for (int i = 0; i < ARR_LEN; i++) {
        arr[i] = rand() % 100;
    }

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("cannot open pipe");
        exit(EXIT_FAILURE);
    }

    int son = fork();
    if (son == 0) {
        close(pipe_fd[0]);
        if (dup2(pipe_fd[1], STDOUT_FILENO) != -1) {
            sort(arr, ARR_LEN);
            printf("%d\n", arr[0]);
            printf("%d\n", arr[ARR_LEN - 1]);
        }
        else {
            perror("dup2 failed\n");
        }
        close(pipe_fd[1]);
        exit(0);
    }

    close(pipe_fd[1]);
    if (dup2(pipe_fd[0], STDIN_FILENO) != -1) {
        int min = -1, max = -1;
        scanf("%d", &min);
        scanf("%d", &max);
        printf("I got %d %d\n", min, max);
    }
    else {
        perror("dup2 failed\n");
    }
    close(pipe_fd[0]);
}