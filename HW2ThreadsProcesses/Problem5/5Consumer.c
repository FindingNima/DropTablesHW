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
#include <sys/socket.h>
#include <string.h>
#include <errno.h>


#define BUFFER_SIZE 1024
#define PRODUCER_SOCKET_NAME "TeamDropTablesProducerSocket"
#define CONSUMER_SOCKET_NAME "TeamDropTablesConsumerSocket"

//array of all buffers
int *all_buffers;
int num_producers;
int num_consumers;
int num_buffers;
int num_items;

//producer socket related info
int producer_socket_fd;     //like a file descriptor used as handle to socket
struct sockaddr_un name;


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


    int write_result = write(producer_socket_fd, &index, sizeof(int));
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

void handle_received_message(int socket_fd){
    int received_messages = 0;
    while (received_messages <= num_items) {
        int index_to_increment;
        if (read(socket_fd, &index_to_increment, sizeof(int)) == 0) {
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


void *socket_listener(void *unneeded_arg) {
    int consumer_socket_fd;     //like a file descriptor used as handle to socket
    struct sockaddr_un name;

    /* start with a clean address structure */
    memset(&name, 0, sizeof(struct sockaddr_un));

    printf("Consumer waiting for a connection(1)...");
    //Create Socket - establish phone
    consumer_socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);    //Using Local namespace,SOCK_STREAM-Connection style
    if (consumer_socket_fd < 0) {
        printf("Consumer SOCKET ERROR. COULD NOT CREATE");
    }


    //Indicate this is the server and get unique address -- Receiver in the phone system and get phone #
    name.sun_family = AF_LOCAL;
    strcpy(name.sun_path, CONSUMER_SOCKET_NAME);
    int result = bind(consumer_socket_fd, (struct sockaddr *) &name, SUN_LEN (&name));
    if (result < 0) {
        printf("Consumer BINDING FAILED");
    }

    //Listen for Connection - Turn on Ringer and wait for call
    result = listen(consumer_socket_fd, (num_buffers * BUFFER_SIZE) / 2 + 1);
    if (result < 0) {
        printf("Consumer LISTEN FAILED");
    }


    struct sockaddr_un client_name;
    socklen_t client_name_len;
    int client_socket_fd;

    //Accept a connection - pick up phone
    printf("Consumer waiting for a connection...");
    client_socket_fd = accept(consumer_socket_fd, (struct sockaddr *) &client_name, &client_name_len);
    if (client_socket_fd < 0) {
        printf("In Consumer, Client_socket_fd: %d, error string: %s\n", client_socket_fd, strerror(errno));
    }

    handle_received_message(client_socket_fd);

    //Close connection -  HANG UP
    close(client_socket_fd);

    close(consumer_socket_fd);
    unlink(CONSUMER_SOCKET_NAME);
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

    pthread_t socket_thread;
    pthread_create(&socket_thread, NULL, socket_listener, NULL);
    sleep(1);

    memset(&name, 0, sizeof(struct sockaddr_un));
    //Create Socket - establish phone
    producer_socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);    //Using Local namespace,SOCK_STREAM-Connection style

    // Store server name in socket address -- Receiver in the phone system and get phone #
    name.sun_family = AF_LOCAL;
    strcpy(name.sun_path, PRODUCER_SOCKET_NAME);

    int connect_result = connect(producer_socket_fd, (struct sockaddr *) &name, SUN_LEN (&name));
    if (connect_result < 0) {
        printf("Consumer connect_result: %d, error string: %s\n", connect_result, strerror(errno));
    }

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
    close(producer_socket_fd);

    pthread_join(socket_thread, NULL);
}