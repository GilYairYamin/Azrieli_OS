#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#define THREAD_AMOUNT 2

#define PRINT_START 0
#define PRINT_END 4
#define PRINT_INTERVAL 3

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
bool bool_array[THREAD_AMOUNT] = { 0 };

int min(int x, int y) {
    return (x < y) ? x : y;
}

void resetBoolArray() {
    for (int i = 0; i < THREAD_AMOUNT; i++) {
        if (bool_array[i] == false)
            return;
    }
    for (int i = 0; i < THREAD_AMOUNT; i++) {
        bool_array[i] = false;
    }
}

void* runThread(void* arg) {
    int thread_num = *((int*)arg);

    int i = PRINT_START, temp = 0;
    while (i <= PRINT_END) {
        temp = min(i + PRINT_INTERVAL, PRINT_END + 1);
        pthread_mutex_lock(&mtx);

        for (int j = i; j < temp; j++) {
            printf("thread %lu: i = %d\n", pthread_self(), j);
            sleep(1);
        }
        bool_array[thread_num] = true;

        resetBoolArray();
        pthread_mutex_unlock(&mtx);

        i = temp;
        while (bool_array[thread_num]) {
            usleep(25);
        }
    }
    return NULL;
}

int main() {
    int status = 0;
    int thread_nums[THREAD_AMOUNT] = { 0 };
    pthread_t threads[THREAD_AMOUNT] = { 0 };

    for (int i = 0; i < THREAD_AMOUNT; i++) {
        thread_nums[i] = i;
        status = pthread_create(&(threads[i]), NULL, runThread, (void*)(&thread_nums[i]));
        if (status != 0) {
            fputs("pthread create failed in main", stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREAD_AMOUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&mtx);
    return EXIT_SUCCESS;
}