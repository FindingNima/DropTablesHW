#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


#define BUFFER_SIZE 1024



void main(int argc, char *argv[]) {
    // int num_producers = atoi(argv[1]);
    // int num_consumers = atoi(argv[2]);
    // int num_buffers = atoi(argv[3]);
    // int num_items = atoi(argv[4]);

    pid_t pid;
    int pro2con[2];
    int con2pro[2];

    printf("pro2con[0] = %d\n", pro2con[0]);
    printf("pro2con[1] = %d\n", pro2con[1]);
    printf("con2pro[0] = %d\n", con2pro[0]);
    printf("con2pro[1] = %d\n", con2pro[1]);

    pipe(pro2con);
    pipe(con2pro);

    char *pro2con0 = malloc(10*sizeof(char));
    char *pro2con1 = malloc(10*sizeof(char));
    char *con2pro0 = malloc(10*sizeof(char));
    char *con2pro1 = malloc(10*sizeof(char));

    sprintf(pro2con0, "%d", pro2con[0]);
        sprintf(pro2con1, "%d", pro2con[1]);
    sprintf(con2pro0, "%d", con2pro[0]);
        sprintf(con2pro1, "%d", con2pro[1]);

    printf("After calling pipe()...\n");

    printf("pro2con[0] = %s\n", pro2con0);
    printf("pro2con[1] = %s\n", pro2con1);
    printf("con2pro[0] = %s\n", con2pro0);
    printf("con2pro[1] = %s\n", con2pro1);


    return;

}