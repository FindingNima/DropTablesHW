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
    int num_producers = atoi(argv[1]);
    int num_consumers = atoi(argv[2]);
    int num_buffers = atoi(argv[3]);
    int num_items = atoi(argv[4]);


    int pid = fork();
    if (pid == 0) {
        //child process - producer
        execv("./5Producer", argv);
    } else {
        //parent process
        int pid2 = fork();
        if (pid2 == 0) {
            //second child - consumer
            execv("./5Consumer", argv);
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