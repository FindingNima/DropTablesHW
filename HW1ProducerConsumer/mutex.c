#include <stdio.h>
#include <pthread.h>

#define MAX 10000
pthread_mutex_t mutex;
pthread_cond_t condc, condp;
int buffer = 0;

void* producer(void* pointer);
void* consumer(void* pointer);

int main(){
    pthread_t consumer_thread,producer_thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condc, NULL);
    pthread_cond_init(&condp, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    pthread_cond_destroy(&condc);
    pthread_cond_destroy(&condp);
    pthread_mutex_destroy(&mutex);

}


void* producer(void* pointer){
    int i;
    for (i = 1; i <= MAX; i++) {
        // gain exclusive access to buffer variable
        pthread_mutex_lock(&mutex);
        while (buffer != 0) {
            pthread_cond_wait(&condp, &mutex);
        }

        //put item in buffer
        buffer = i;
        printf("Producer placed %d into buffer\n", buffer);


        pthread_cond_signal(&condc);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void* consumer(void* pointer){
    int i;
    for (i = 1; i <= MAX; i++) {
        // gain exclusive access to buffer variable
        pthread_mutex_lock(&mutex);
        while (buffer == 0) {
            pthread_cond_wait(&condc, &mutex);
        }

        printf("Consumer took %d from the buffer\n", buffer);
        //take item out of buffer
        buffer = 0;


        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}
