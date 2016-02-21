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
#include <sys/un.h>
#include <string.h>
#include <errno.h>


#define BUFFER_SIZE 1024

//array of all buffers
int *all_buffers;
int num_producers;
int num_consumers;
int num_buffers;
int num_items;


int inpipe;
int outpipe;


int num_consumer_iterations;

pthread_mutex_t should_continue_consuming_lock;

sem_t total_full; //total number of full slots in all buffers combined
sem_t buffer_lock; //guards access to the buffer which is in shared memory
sem_t send_message_lock;





bool shouldContinueConsuming() {
    if (num_consumer_iterations <= num_items) {
        num_consumer_iterations++;
        return true;
    } else {
        return false;
    }
}

void send_message_to_producer(int index){
//    printf("Consumer sending message! Index: %d\n", index);

    //Connect to the server - Call Server

    int write_result = write(outpipe, &index, sizeof(int));
    if (write_result < 0) {
        printf("Consumer write_result: %d, error string: %s\n", write_result, strerror(errno));
    }
}

void *consumer(void *t_number) {
    int thread_number = *((int *) t_number);
    printf("Consumer Thread #%d started!\n", thread_number);
    while (true) {
        pthread_mutex_lock(&should_continue_consuming_lock);
        bool shouldContinue = shouldContinueConsuming();
        pthread_mutex_unlock(&should_continue_consuming_lock);
        if (shouldContinue == false) {
            break;
        }

        while (sem_trywait(&total_full) != 0){
//            printf("Consumer Thread #%d is Yielding!\n", thread_number);
//            sem_wait(&total_full);
            pthread_yield();
        }
        int index_of_buffer_that_was_decremented = 0;

        sem_wait(&buffer_lock);
        int i;
        for (i = 0; i < num_buffers; i++) {
            if (all_buffers[i]> 0) {
                all_buffers[i]--;
                index_of_buffer_that_was_decremented = i;
                break;
            }
        }
        sem_post(&buffer_lock);

        sem_wait(&send_message_lock);
        send_message_to_producer(index_of_buffer_that_was_decremented);
        sem_post(&send_message_lock);

    }
    printf("Consumer Thread #%d is finished\n", thread_number);
}

void handle_received_message(){
    int received_messages = 0;
    while (received_messages <= num_items) {
        int index_to_increment;
        if (read(inpipe, &index_to_increment, sizeof(int)) == 0) {
            break;
        }
        sem_wait(&buffer_lock);
        all_buffers[index_to_increment]++;
//        printf("Consumer received index to increment: %d\n", index_to_increment);
        sem_post(&total_full);
        sem_post(&buffer_lock);
        received_messages++;
    }
}


void *pipereader(void *unneeded_arg) {

    handle_received_message();
    close(inpipe);
}


void main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Wrong number of arguments...Exiting\n");
        return;
    }
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    num_buffers = atoi(argv[3]);
    num_items = atoi(argv[4]);

    //pipe
    inpipe =  atoi(argv[5]);
    outpipe = atoi(argv[6]);


    printf("Num producers: %d, Num Conusumers: %d,"
                   " Num Buffers: %d, Num Items: %d\n",
           num_producers, num_consumers, num_buffers, num_items);


    num_consumer_iterations = 0;

    all_buffers = (int *)malloc(num_buffers * sizeof(int));
    int i;
    for (i = 0; i < num_buffers; ++i) {
        all_buffers[i] = 0;
    }


    sem_init(&total_full,0,0);
    sem_init(&buffer_lock,0,1);
    sem_init(&send_message_lock,0,1);

    pthread_mutex_init(&should_continue_consuming_lock, NULL);

    pthread_t reader;
    pthread_create(&reader, NULL, &pipereader, NULL);
    sleep(1);

    //array of pthreads
    pthread_t *consumer_threads = (pthread_t *) malloc(num_consumers * sizeof(pthread_t));
    int *consumer_counters = (int *) malloc(num_consumers * sizeof(int));

    int counter;

    for (counter = 0; counter < num_consumers; ++counter) {
        consumer_counters[counter] = counter;
    }

    for (counter = 0; counter < num_consumers; ++counter) {
        printf("Creating consumer thread %d\n", counter);
        pthread_create(&consumer_threads[counter], NULL, consumer, (void *) &consumer_counters[counter]);
    }

    for (counter = 0; counter < num_consumers; ++counter) {
        pthread_join(consumer_threads[counter], NULL);
    }

    //Close connection -  HANG UP
    close(outpipe);

    pthread_join(reader, NULL);
}