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
    if (argc != 5) {
        printf("Wrong number of arguments...Exiting\n");
        return;
    }
    // int num_producers = atoi(argv[1]);
    // int num_consumers = atoi(argv[2]);
    // int num_buffers = atoi(argv[3]);
    // int num_items = atoi(argv[4]);

    int pro2con[2];
    int con2pro[2];

    pipe(pro2con);
    pipe(con2pro);

    char *read = malloc(10*sizeof(char));
    char *write = malloc(10*sizeof(char));

    int pid = fork();
    if (pid == 0) {
        //child process - producer
        sprintf(read, "%d", con2pro[0]);
        sprintf(write, "%d", pro2con[1]);
        char * const extendParm1[] = {argv[0], argv[1], argv[2], argv[3], 
            argv[4], read, write, NULL};
        close(con2pro[1]);
        close(pro2con[0]);
        int i;
        execv("./4Producer", extendParm1);
    } else {
        //parent process
        int pid2 = fork();
        if (pid2 == 0) {
            //second child - consumer
            sprintf(read, "%d", pro2con[0]);
            sprintf(write, "%d", con2pro[1]);
            char * const extendParm2[] = {argv[0], argv[1], argv[2], argv[3], 
                argv[4], read, write, NULL};
            close(pro2con[1]);
            close(con2pro[0]);
            execv("./4Consumer", extendParm2);
        } else {
            //parent process
            int producer_status;
            waitpid(pid, &producer_status, 0);
            int consumer_status;
            waitpid(pid2, &consumer_status, 0);
            return;
        }
    }
}