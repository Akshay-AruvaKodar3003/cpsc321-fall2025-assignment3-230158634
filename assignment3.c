/* 
Akshay Arulkrishnan
230158634
CPSC 321 - Assignment 3
31-10-2025
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Thread function to test pthread setup
void* worker(void* arg) {
    (void)arg; 
    printf("[worker] pthread started\n");
    sleep(1); 
    return NULL;
}

int main(void) {
    printf("Assignment 3 running\n");

    pthread_t t; // Just one thread 

    // Creating the thread 
    if (pthread_create(&t, NULL, worker, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    // Waiting for the thread to finish
    pthread_join(t, NULL);

    printf("[main] pthread join OK\n");
    return 0;
}