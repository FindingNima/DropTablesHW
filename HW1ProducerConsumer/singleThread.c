#include <stdio.h>

#define NUMBER_ITEMS 100
#define BUFFER_SIZE 10

/*
	producer takes in the buffer array
	will set the whole array to "1"
	means it fills the buffer
*/
void producer(int* buffer){
	int i;
	for (i = 0; i < BUFFER_SIZE; ++i)
	{
		*buffer = 1;
		buffer++;
	}
}

/*
	consumer takes in the buffer array
	will set the whole array to "0"
	means it takes everything from the buffer
*/
void consumer(int* buffer){
	int i;
	for (i = 0; i < BUFFER_SIZE; ++i)
	{
		*buffer = 0;
		buffer++;
	}
}

void printBuffer(int *buffer){
	int i;
	for (i = 0; i < BUFFER_SIZE; ++i)
	{
		printf("%d ", *buffer);
		buffer++;
	}
	printf("\n");
}

int main(){
	int buffer[BUFFER_SIZE] = {};
	int numItemsProduced = 0;

	while (numItemsProduced < NUMBER_ITEMS){
		producer(buffer);
		printf("producer has filled the buffer with 1s\n");
		printBuffer(buffer);
		consumer(buffer);
		printf("consumer has consumed all the 1s\n");
		printBuffer(buffer);
		numItemsProduced += BUFFER_SIZE;
	}

	printf("All %d items were consumed\n", NUMBER_ITEMS);
}
