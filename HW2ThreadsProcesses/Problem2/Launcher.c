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


#define BUFFER_SIZE 1024
#define FULL_SEMAPHORE_NAME "/TeamDropTablesFullSemaphore"
#define EMPTY_SEMAPHORE_NAME "/TeamDropTablesEmptySemaphore"
#define BUFFER_SEMAPHORE_NAME "/TeamDropTablesBufferSemaphore"
#define KEY_INT 1337


//This needs to happen at compile time since we're using system semaphores:
//Programs using the POSIX semaphores API must be compiled with cc -lrt to link against the real-time library, librt.

/**
 *
 *
 * creates the shared memory pool,
 * launches the producer and consumer process
 * waits for both to finish
 * shared memory should have:
 *
 * int buffers[num_buffers]
 * each index corresponds to an integer representing number of items within the buffer
 *
 * 3 named semaphores
 * first semaphore is total_empty
 * second semaphore is total_full
 * third semaphore is mutex guard against concurrent modification of buffer array
 *
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


    key_t memory_key = ftok(".", KEY_INT); //key for accessing shared memory


    int segment_id;    //ID to Shared Memory Segment
    int *shared_memory;    //Starting Address of Shared Memory
    struct shmid_ds shmbuffer;
    int segment_size;
    int shared_segment_size = num_buffers * sizeof(int);    //bytes allocate rounded up to integer multip of page size

    //ALLOCATE SHARED MEMORY SEGMENT
    segment_id = shmget(memory_key, shared_segment_size, IPC_CREAT | IPC_EXCL | 0666);

    //ATTACH SHARED MEMORY SEGMENT
    shared_memory = (int *) shmat(segment_id, 0, 0);
    int *all_buffers = shared_memory;
    int i;
    for (i = 0; i < num_buffers; ++i) {
        all_buffers[i] = 0;
    }
    printf(" shared memory attached at address %p\n", shared_memory);

    //DETERMINE SHARED MEMORY SEGMENT SIZE
    shmctl(segment_id, IPC_STAT, &shmbuffer);
    segment_size = shmbuffer.shm_segsz;
    printf("Shared Memory Segment Size = %d\n", segment_size);

    //DETACH THE SHARED MEMORY SEGMENT
    shmdt(shared_memory);

    //initialize full semaphore to 0
    sem_t *total_full = sem_open(FULL_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 0);
    //initialize empty semaphore to total number of spots in all buffers
    sem_t *total_empty = sem_open(EMPTY_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, num_buffers * BUFFER_SIZE);
    //initialize buffer semaphore with 1 to use as a lock on shared memory buffers
    sem_t *buffer_lock = sem_open(BUFFER_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);


    if (segment_id < 0) {
        printf("shmget error from launcher process!\n");
        exit(1);
    }

    int pid = fork();
    if (pid == 0) {
        //child process - producer
        execv("./2Producer", argv);
    } else {
        //parent process
        int pid2 = fork();
        if (pid2 == 0) {
            //second child - consumer
            execv("./2Consumer", argv);
        } else {
            //parent process
            int producer_status;
            waitpid(pid, &producer_status, 0);
            int consumer_status;
            waitpid(pid2, &consumer_status, 0);

            //DEALLOCATE THE SHARED MEMORY SEGMENT   ****(Don't Forget This Step!!!!)****
            shmctl(segment_id, IPC_RMID, 0);
            //DEALLOCATE SHARED SEMAPHORES
            sem_close(total_empty);
            sem_close(total_full);
            sem_close(buffer_lock);
            return;
        }
    }
}