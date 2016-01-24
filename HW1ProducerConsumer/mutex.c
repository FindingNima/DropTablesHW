#include <pthread.h>
#include <stdio.h>

pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;

#define BUFFER_SIZE 10
#define NITEMS 100

int buffer[BUFFER_SIZE] = {};

int bufindex;

void insertitem(int item){
	buffer[bufindex] = item;
	bufindex++;
}

int removeitem(){
	int item = buffer[bufindex-1];
	buffer[bufindex-1] = 0;
	bufindex--;
	return item;
}

void printBuffer(){
	int i = 0;
	while (i < BUFFER_SIZE){
		printf("%2d ", buffer[i]);
		i++;
	}
	printf("\n");
}

void* producer(void* args){
	int produced = 0;
	while (produced != NITEMS){
		int item = produced;
		produced++;
		pthread_mutex_lock(&the_mutex);
		while(bufindex != 0) pthread_cond_wait(&condp, &the_mutex);
		insertitem(item);
		printf("Produced item %2d\n", item);
		//printBuffer();
		pthread_cond_signal(&condc);
		pthread_mutex_unlock(&the_mutex);
	}

	pthread_exit(0);
}

void* consumer(void* args){
	int consumed = 0;
	while (consumed != NITEMS){
		consumed++;
		pthread_mutex_lock(&the_mutex);
		while(bufindex == 0) pthread_cond_wait(&condc, &the_mutex);
		int item = removeitem();
		printf("Removed item %2d\n", item);
		printBuffer();
		pthread_cond_signal(&condp);
		pthread_mutex_unlock(&the_mutex);
	}
	pthread_exit(0);
}

int main (){
	bufindex = 0;
	pthread_t pro, con;
	pthread_t pro2, con2;
	pthread_mutex_init(&the_mutex, 0);
	pthread_cond_init(&condp, 0);
	pthread_cond_init(&condc, 0);
	pthread_create(&con, 0, consumer, 0);
	pthread_create(&con2, 0, consumer, 0);
	pthread_create(&pro, 0, producer, 0);
	pthread_create(&pro2, 0, producer, 0);
	pthread_join(pro,0);
	pthread_join(pro2,0);
	pthread_join(con,0);
	pthread_join(con2,0);
	pthread_cond_destroy(&condc);
	pthread_cond_destroy(&condp);
	pthread_mutex_destroy(&the_mutex);

}