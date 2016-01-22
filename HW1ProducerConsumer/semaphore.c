#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define BUFFER_SIZE 10
#define NUM_ITEMS 100

sem_t criticalAccessor;
sem_t empty;
sem_t full;

pthread_t producer;
pthread_t consumer;

int startIndexInBuffer;

int buffer[NUM_ITEMS] = {};

/**
	can only be accessed 
	when binary semaphore critical accessor allows
**/
void insertItem(int thingToInsert){
	buffer[startIndexInBuffer] = thingToInsert;
	startIndexInBuffer++;
}

/**
	can only be accessed 
	when binary semaphore critical accessor allows
**/
int removeItem(){
	int item = buffer[startIndexInBuffer-1];
	startIndexInBuffer--;
	return item;
}


void* producerFunc(void* unneededArg){
	int numItemsProduced = 0;
	while (numItemsProduced != NUM_ITEMS){
		int itemProduced = numItemsProduced;
		numItemsProduced++;
		sem_wait(&empty);
		sem_wait(&criticalAccessor);
		insertItem(itemProduced);
		printf("Produced item %2d\n", itemProduced);
		sem_post(&criticalAccessor);
		sem_post(&full);
	}
}

void* consumerFunc(void* unneededArg){
	int numItemsConsumed = 0;
	while (numItemsConsumed != NUM_ITEMS){
		numItemsConsumed++;
		sem_wait(&full);
		sem_wait(&criticalAccessor);
		int item = removeItem();
		printf("Removed item %2d\n", item);
		sem_post(&criticalAccessor);
		sem_post(&empty);
	}
}

int main(){
	startIndexInBuffer = 0;
	sem_init(&criticalAccessor, 0, 1);
	sem_init(&empty,0,BUFFER_SIZE);
	sem_init(&full,0,0);
	pthread_create(&producer,NULL,producerFunc,NULL);
	pthread_create(&consumer,NULL,consumerFunc,NULL);
	pthread_join(consumer,NULL);
	pthread_join(producer,NULL);

	sem_destroy(criticalAccessor);
	sem_destroy(empty);
	sem_destroy(full);
}