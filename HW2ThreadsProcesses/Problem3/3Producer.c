#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


#define BUFFER_SIZE 1024

//array of all buffers
int *all_buffers;
int num_producers;
int num_consumers;
int num_buffers;
int num_items;


int num_producer_iterations;
int actual_num_produced;

pthread_mutex_t should_continue_producing_lock;
pthread_mutex_t buffer_printer_lock;

//shared semaphores
sem_t *total_empty; //total number of empty slots in all buffers combined
sem_t *total_full; //total number of full slots in all buffers combined
sem_t *buffer_lock; //guards access to the buffer which is in shared memory


bool shouldContinueProducing() {
    if (num_producer_iterations <= num_items) {
        num_producer_iterations++;
        return true;
    } else {
        return false;
    }
}


void bufferPrinter(int thread_number) {
//    printf("bufferprinter called by %d\n", thread_number);
    if (actual_num_produced % 1000 == 0 && actual_num_produced != 0) {
        printf("%d items created\n", actual_num_produced);
        int i;
        sem_wait(buffer_lock);
        for (i = 0; i < num_buffers; i++) {
            printf("Shared buffer %d has %d number of items\n", i + 1, all_buffers[i]);
        }
        sem_post(buffer_lock);
    }
    actual_num_produced++;
    return;
}


void *producer(void *t_number) {
    int thread_number = *((int *) t_number);
    printf("Producer Thread #%d started!\n", thread_number);
    while (true) {
        pthread_mutex_lock(&should_continue_producing_lock);
        bool shouldContinue = shouldContinueProducing();
        pthread_mutex_unlock(&should_continue_producing_lock);
        if (shouldContinue == false) {
            break;
        }

        sem_wait(total_empty);

        sem_wait(buffer_lock);
        int i;
        for (i = 0; i < num_buffers; i++) {
            if (all_buffers[i] < BUFFER_SIZE) {
                all_buffers[i]++;
                break;
            }
        }
        sem_post(buffer_lock);

        sem_post(total_full);

        pthread_mutex_lock(&buffer_printer_lock);
        bufferPrinter(thread_number);
        pthread_mutex_unlock(&buffer_printer_lock);
    }
    printf("Producer Thread #%d is finished\n", thread_number);
}


void main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments...Exiting\n");
        return;
    }
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    num_buffers = atoi(argv[3]);
    num_items = atoi(argv[4]);

    num_producer_iterations = 0;
    actual_num_produced = 0;

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

    mapped_memory = (int*)mmap(NULL, mapped_memory_size, PROT_WRITE, MAP_SHARED, file_descriptor, 0);

    close(file_descriptor);




    all_buffers = mapped_memory;

    //initialize full semaphore to 0
    total_full = sem_open(FULL_SEMAPHORE_NAME, 0);

    //initialize empty semaphore to total number of spots in all buffers
    total_empty = sem_open(EMPTY_SEMAPHORE_NAME, 0);

    //initialize buffer semaphore with 1 to use as a lock on shared memory buffers
    buffer_lock = sem_open(BUFFER_SEMAPHORE_NAME, 0);


    pthread_mutex_init(&should_continue_producing_lock, NULL);
    pthread_mutex_init(&buffer_printer_lock, NULL);


    printf("Num producers: %d, Num Conusumers: %d,"
                   " Num Buffers: %d, Num Items: %d\n",
           num_producers, num_consumers, num_buffers, num_items);


    //array of pthreads
    pthread_t *producer_threads = (pthread_t *) malloc(num_producers * sizeof(pthread_t));
    int *producer_counters = (int *) malloc(num_producers * sizeof(int));

    int counter;
    for (counter = 0; counter < num_producers; ++counter) {
        producer_counters[counter] = counter;
    }

    for (counter = 0; counter < num_producers; ++counter) {
        printf("Creating producer thread %d\n", counter);
        pthread_create(&producer_threads[counter], NULL, producer, (void *) &producer_counters[counter]);
    }

    for (counter = 0; counter < num_producers; ++counter) {
        pthread_join(producer_threads[counter], NULL);
    }
}