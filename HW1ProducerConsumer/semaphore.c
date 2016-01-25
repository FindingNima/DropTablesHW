#define TOTAL_ITEMS 100
#define BUFFER_SIZE 10
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t critical_accessor;
sem_t empty;
sem_t full;

int buffer[BUFFER_SIZE] = {};

void* producer(void *);
void* consumer(void *);
void print_buffer();
void insert_item_into_buffer(int);
int take_item_from_buffer();



int main() {
    //only 1 thread can be in critical section at a time
    sem_init(&critical_accessor, 0, 1);

    //number of empty positions in buffer is initially all positions
    sem_init(&empty, 0, BUFFER_SIZE);

    //number of full positions in buffer is initially none
    sem_init(&full, 0, 0);

    pthread_t producer_thread;
    pthread_t consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(&critical_accessor);
    sem_destroy(&empty);
    sem_destroy(&full);

    pthread_exit(NULL);

}


void* producer(void* unnecessary_argument){
    int num_items_produced = 0;
    while (num_items_produced < TOTAL_ITEMS) {
        int current_item_number = num_items_produced+1;

        //wait until there is an empty slot in buffer
        sem_wait(&empty);

        //wait until buffer is not being accessed anymore
        sem_wait(&critical_accessor);

        insert_item_into_buffer(current_item_number);

        printf("Producer inserted %d into the buffer!\n", current_item_number);
        printf("Buffer currently looks like: \n");
        print_buffer();

        //finished with critical region
        sem_post(&critical_accessor);

        //unblock anything waiting for more input
        sem_post(&full);

        num_items_produced++;
    }
    return NULL;
}

void* consumer(void* unnecessary_argument){
    int num_items_consumed = 0;
    while (num_items_consumed < TOTAL_ITEMS) {
        //wait until there is an empty slot in buffer
        sem_wait(&full);

        //wait until buffer is not being accessed anymore
        sem_wait(&critical_accessor);

        int item = take_item_from_buffer();

        printf("Consumer took %d from the buffer!\n", item);
        printf("Buffer currently looks like: \n");
        print_buffer();

        //finished with critical region
        sem_post(&critical_accessor);

        //unblock anything waiting for more input
        sem_post(&empty);

        num_items_consumed++;
    }
    return NULL;
}

/**
 * inserts the number passed to the function into the
 * first free location found in the buffer
 *
 * the number passed in must be nonzero!!!
 */
void insert_item_into_buffer(int item) {
    int i;
    for (i = 0; i < BUFFER_SIZE; ++i) {
        if (buffer[i] == 0) {
            buffer[i] = item;
            return;
        }
    }
    /*
     * if we didn't return from within loop, then
     * we tried inserting when buffer was full
     */
    exit(-1);
}

/**
 * takes first nonzero item from buffer
 */
int take_item_from_buffer() {
    int i;
    for (i = 0; i < BUFFER_SIZE; ++i) {
        if (buffer[i] != 0) {
            int item_to_return = buffer[i];
            buffer[i] = 0;
            return item_to_return;
        }
    }
    /*
    * if we didn't return from within loop, then
    * we tried removing when buffer was empty
    */
    exit(-2);
}


/**
 * prints the contents of the buffer array
 */
void print_buffer() {
    int i;
    for (i = 0; i < BUFFER_SIZE; ++i) {
        printf("%d, ", buffer[i]);
    }
    printf("\n\n");
}

