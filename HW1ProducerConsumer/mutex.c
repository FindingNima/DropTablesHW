<<<<<<< HEAD
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
=======
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
>>>>>>> 6081b1336acc3fc8250d874a83225945f4a7e005
