#include <stdio.h>
#include <pthread.h>

// how many numbers should be printed?
#define MAX 100000
pthread_mutex_t mutex;
pthread_cond_t condc, condp;
int buffer = 0;

void *consumer(void *p) {
	int i;
	for(i=1; i<=MAX; i++) {
		// lock the mutex
		pthread_mutex_lock(&mutex);
		// wait if the buffer is empty
		while(buffer == 0) 
			pthread_cond_wait(&condc, &mutex);
		buffer = buffer - 1;
		// print out that the consumer consumed!
		printf("consumer set buffer to %d\n", buffer);
		// signal the producer
		pthread_cond_signal(&condp);
		// release the mutex
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(0);
}

void *producer(void *p) {
	int i;
	for(i=1; i<=MAX; i++) {
		// lock the mutex
		pthread_mutex_lock(&mutex);
		// wait if there is something in the buffer
		while(buffer != 0)
			pthread_cond_wait(&condp, &mutex);
		buffer = buffer + 1;
		// print out that something happened!
		printf("producer set buffer to %d\n",buffer);
		// wake up the consumer
		pthread_cond_signal(&condc);
		// release the mutex
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(0);
}

int main(int argc, char** argv) {
	// the two threads, producer and consumer
	pthread_t prod, cons;
	// initialize the mutex
	pthread_mutex_init(&mutex, 0);
	// init the conditions to wake up
	pthread_cond_init(&condc, 0);
	pthread_cond_init(&condp, 0);
	// create the threads
	pthread_create(&prod, 0, producer, 0);
	pthread_create(&cons, 0, consumer, 0);
	// join them so if one terminates, the other does too
	pthread_join(prod, 0);
	pthread_join(cons, 0);
	// destroy the threads and the mutex
	pthread_cond_destroy(&condc);
	pthread_cond_destroy(&condp);
	pthread_mutex_destroy(&mutex);
	// exit gracefully
	return(0);
}
