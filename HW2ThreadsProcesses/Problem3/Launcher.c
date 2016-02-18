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


//This needs to happen at compile time since we're using system semaphores:
//Programs using the POSIX semaphores API must be compiled with cc -lrt to link against the real-time library, librt.

/**
 *
 *
 * creates the memory mapped file,
 * launches the producer and consumer process
 * waits for both to finish
 * mapped memory should have:
 *
 * int buffers[num_buffers]
 * each index corresponds to an integer representing number of items within the buffer
 *
 * 3 named semaphores
 * first semaphore is total_empty
 * second semaphore is total_full
 * third semaphore is mutex guard against concurrent modification of buffer array
 */

void main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments...Exiting\n");
        return;
    }
    int num_producers = atoi(argv[1]);
    int num_consumers = atoi(argv[2]);
    int num_buffers = atoi(argv[3]);
    int num_items = atoi(argv[4]);


    int *mapped_memory;    //Starting Address of Shared Memory
    int mapped_memory_size = num_buffers * sizeof(int);    //bytes allocate rounded up to integer multip of page size

    //ALLOCATE MAPPED MEMORY SEGMENT
    //Prepate a file large enough to hold the int buffer
    const char *filename = "/tmp/mapped_memory_bufferfile.txt";

    int file_descriptor = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file_descriptor < 0) {
        printf("ERROR: Couldn't create a buffer file! Exiting...");
        exit(1);
    }
    lseek(file_descriptor, mapped_memory_size + 1, SEEK_SET);    //make sure file is large enough
    write(file_descriptor, "", 1);
    lseek(file_descriptor, 0, SEEK_SET);    //move write pointer back to beginning


    //ATTACH SHARED MEMORY SEGMENT
    mapped_memory = (int*)mmap(NULL, mapped_memory_size, PROT_WRITE, MAP_SHARED, file_descriptor, 0);


    close(file_descriptor);

    int *all_buffers = mapped_memory;
    int i;
    for (i = 0; i < num_buffers; ++i) {
        all_buffers[i] = 0;
    }
    printf(" shared memory attached at address %p\n", mapped_memory);



    //initialize full semaphore to 0
    sem_t *total_full = sem_open(FULL_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 0);
    //initialize empty semaphore to total number of spots in all buffers
    sem_t *total_empty = sem_open(EMPTY_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, num_buffers * BUFFER_SIZE);
    //initialize buffer semaphore with 1 to use as a lock on shared memory buffers
    sem_t *buffer_lock = sem_open(BUFFER_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);


    int pid = fork();
    if (pid == 0) {
        //child process - producer
        execv("./3Producer", argv);
    } else {
        //parent process
        int pid2 = fork();
        if (pid2 == 0) {
            //second child - consumer
            execv("./3Consumer", argv);
        } else {
            //parent process
            int producer_status;
            waitpid(pid, &producer_status, 0);
            int consumer_status;
            waitpid(pid2, &consumer_status, 0);

            //DEALLOCATE THE MAPED MEMORY ****(Don't Forget This Step!!!!)****
            munmap(mapped_memory, mapped_memory_size);
            //DEALLOCATE SHARED SEMAPHORES
            sem_close(total_empty);
            sem_close(total_full);
            sem_close(buffer_lock);

            return;
        }
    }
}