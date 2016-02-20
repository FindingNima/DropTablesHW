#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>


#define BUFFER_SIZE 1024


typedef struct Buffer {
    //number of items currently within the buffer
    int num_items;
    //originally I intended to put more fields here, but I ended up not needing any
} Buffer;


//array of all buffers
Buffer *all_buffers;
int num_producers;
int num_consumers;
int num_buffers;
int num_items;


int num_producer_iterations;
int num_consumer_iterations;
int actual_num_produced;

pthread_mutex_t should_continue_producing_lock;
pthread_mutex_t should_continue_consuming_lock;
pthread_mutex_t buffer_printer_lock;
pthread_mutex_t buffer_lock;

int total_empty; //total number of empty slots in all buffers combined
int total_full; //total number of full slots in all buffers combined
pthread_cond_t no_longer_empty;
pthread_cond_t no_longer_full;
pthread_mutex_t buffer_size_lock; //guards both total_empty and total_full variables




bool shouldContinueProducing() {
    if (num_producer_iterations <= num_items) {
        num_producer_iterations++;
        return true;
    } else {
        return false;
    }
}


bool shouldContinueConsuming() {
    if (num_consumer_iterations <= num_items) {
        num_consumer_iterations++;
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
        pthread_mutex_lock(&buffer_lock);
        for (i = 0; i < num_buffers; i++) {
            printf("Shared buffer %d has %d number of items\n", i + 1, all_buffers[i].num_items);
        }
        pthread_mutex_unlock(&buffer_lock);
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

        //equivalent to sem_wait
        pthread_mutex_lock(&buffer_size_lock);
        while (total_empty == 0) {
            pthread_cond_wait(&no_longer_full, &buffer_size_lock);
        }
        total_empty--;
        pthread_mutex_unlock(&buffer_size_lock);

        int i;
        pthread_mutex_lock(&buffer_lock);
        for (i = 0; i < num_buffers; i++) {
            if (all_buffers[i].num_items < BUFFER_SIZE) {
                all_buffers[i].num_items++;
                break;
            }
        }
        pthread_mutex_unlock(&buffer_lock);


        //equivalent to sem_post
        pthread_mutex_lock(&buffer_size_lock);
        total_full++;
        pthread_cond_signal(&no_longer_empty);
        pthread_mutex_unlock(&buffer_size_lock);


        pthread_mutex_lock(&buffer_printer_lock);
        bufferPrinter(thread_number);
        pthread_mutex_unlock(&buffer_printer_lock);
    }
    printf("Producer Thread #%d is finished\n", thread_number);
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


        pthread_mutex_lock(&buffer_size_lock);
        while (total_full == 0) {
            printf("Consumer Thread #%d is Yielding!\n", thread_number);
            pthread_cond_wait(&no_longer_empty, &buffer_size_lock);
        }
        total_full--;
        pthread_mutex_unlock(&buffer_size_lock);


        int i;
        pthread_mutex_lock(&buffer_lock);
        for (i = 0; i < num_buffers; i++) {
            if (all_buffers[i].num_items > 0) {
                all_buffers[i].num_items--;
                break;
            }
        }
        pthread_mutex_unlock(&buffer_lock);


        pthread_mutex_lock(&buffer_size_lock);
        total_empty++;
        pthread_cond_signal(&no_longer_full);
        pthread_mutex_unlock(&buffer_size_lock);

    }
    printf("Consumer Thread #%d is finished\n", thread_number);
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
    num_consumer_iterations = 0;
    actual_num_produced = 0;

    all_buffers = (Buffer *) malloc(num_buffers * sizeof(Buffer));
    int counter;
    for (counter = 0; counter < num_buffers; counter++) {
        all_buffers[counter].num_items = 0;
    }

    total_empty = num_buffers * BUFFER_SIZE;
    total_full = 0;

    pthread_mutex_init(&buffer_size_lock, NULL);

    pthread_mutex_init(&should_continue_producing_lock, NULL);
    pthread_mutex_init(&should_continue_producing_lock, NULL);
    pthread_mutex_init(&buffer_printer_lock, NULL);
    pthread_mutex_init(&buffer_lock, NULL);


    printf("Num producers: %d, Num Conusumers: %d,"
                   " Num Buffers: %d, Num Items: %d\n",
           num_producers, num_consumers, num_buffers, num_items);


    //array of pthreads
    pthread_t *producer_threads = (pthread_t *) malloc(num_producers * sizeof(pthread_t));
    pthread_t *consumer_threads = (pthread_t *) malloc(num_consumers * sizeof(pthread_t));
    int *producer_counters = (int *) malloc(num_producers * sizeof(int));
    int *consumer_counters = (int *) malloc(num_consumers * sizeof(int));

    for (counter = 0; counter < num_producers; ++counter) {
        producer_counters[counter] = counter;
    }

    for (counter = 0; counter < num_consumers; ++counter) {
        consumer_counters[counter] = counter;
    }


    for (counter = 0; counter < num_producers; ++counter) {
        printf("Creating producer thread %d\n", counter);
        pthread_create(&producer_threads[counter], NULL, producer, (void *) &producer_counters[counter]);
    }

    for (counter = 0; counter < num_consumers; ++counter) {
        printf("Creating consumer thread %d\n", counter);
        pthread_create(&consumer_threads[counter], NULL, consumer, (void *) &consumer_counters[counter]);
    }


    for (counter = 0; counter < num_producers; ++counter) {
        pthread_join(producer_threads[counter], NULL);
    }
    for (counter = 0; counter < num_consumers; ++counter) {
        pthread_join(consumer_threads[counter], NULL);
    }

}