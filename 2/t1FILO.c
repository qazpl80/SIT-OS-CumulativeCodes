#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>     // for sleep()

typedef int buffer_item;

#define BUFFER_SIZE 5
#define TRUE 1


//  The circular buffer
buffer_item buffer[BUFFER_SIZE];

//  Global variable for the buffer counter
int buffer_counter;

// int global variable for inserting and deleting
int removeItem;
int addItem;

//  Semaphore for number of empty slots in the buffer
sem_t empty_count;

//  Semaphore for number of full slots in the buffer
sem_t full_count;

//  Mutual exclusion lock
//  pthread_mutex_t mutex
sem_t mutex;

//  Inserts an item into the buffer
//  Returns 0 if successful, otherwise -1
int insert_item(buffer_item item)
{
    if(buffer_counter < BUFFER_SIZE)
    {
        buffer[addItem] = item;
        addItem++;
        if(addItem == BUFFER_SIZE){
            addItem = 0;
        }
        buffer_counter++;
        return 0;
    }
    printf("Buffer Full...\n");
    return -1;
}

//  Removes an item from the buffer
//  Returns 0 if successful, otherwise -1
int remove_item(buffer_item *item)
{
    if(buffer_counter > 0)
    {
        *item = buffer[removeItem];
        removeItem++;
        if(removeItem == BUFFER_SIZE){
            removeItem = 0;
        }
        buffer_counter--;
        return 0;
    }
    printf("Buffer Empty...\n");
    return -1;
}

//  Producer thread
void* producer(void *arg)
{
    buffer_item item;

    while (TRUE)
    {
        //  Sleep for a random amount of time between 1 and 3 seconds
        sleep(rand() % 3 + 1);

        //  Create the item to be inserted
        item = rand() % 1000;

        //  Decrement the empty count semaphore
        sem_wait(&empty_count);

        //  Acquire mutex lock before critical section
        //  pthread_mutex_lock(&mutex)
        sem_wait(&mutex);

        //  Critical section, insert the item into buffer
        if (insert_item(item) == 0)
        {
            printf("Producer%d inserted item %d\n", *(int *)arg, item);
        }
        else
        {
            printf("Producer%d error\n", *(int *)arg);
        }

        //  Critical section finished, release the mutex lock
        //  pthread_mutex_unlock(&mutex);
        sem_post(&mutex);

        //  Increment the full semaphore, signalling that consumer process may start
        sem_post(&full_count);
    }
}

//  Consumer thread
void* consumer(void *arg)
{
    buffer_item item;

    while(TRUE)
    {
        //  Sleep for a random amount of time between 1 and 3 seconds
        sleep((rand() % 3) + 1);

        //  Decrement the full semaphore
        sem_wait(&full_count);

        //  Acquire mutex lock before critical section
        //  pthread_mutex_lock(&mutex);
        sem_wait(&mutex);

        //  Critical section, insert the item into buffer
        if(remove_item(&item) == 0)
        {
            printf("Consumer%d removed item %d\n", *(int *)arg, item);
        }
        else
        {
            printf("Consumer%d error\n", *(int *)arg);
        }

        // Critical section finished, release the mutex lock
        //  pthread_mutex_unlock(&mutex);
        sem_post(&mutex);

        //  Increment the empty semaphore, signalling that producer process may start
        sem_post(&empty_count);
    }
}

//  Initialise the semaphores and mutex lock
void init_locks()
{
    //  Initialize mutex lock
    //  pthread_mutex_init(&mutex, NULL);
    sem_init(&mutex, 0, 1);

    //  Initialize semaphores
    sem_init(&full_count, 0, 0);
    sem_init(&empty_count,0, BUFFER_SIZE);
}

int main(int argc, char* argv[]) {
    // Verify the number of arguments passed through the command linke
    if (argc != 4) {
        printf("Required inputs: sleep time, the number of producers and the number of consumers\n");
        return -1;
    }

    // Get parameters
    int sleep_duration = atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);

    printf("Number of arguments = %d\n", argc - 1);
    printf("Main Process sleep time = %d\n", sleep_duration);
    printf("Number of producers(s) = %d\n", num_producers);
    printf("Number of consumers(s) = %d\n", num_consumers);

    if (sleep_duration < 1) {
        printf("Sleep duration shall be greater than 0");
        return 0;
    }

    else if (num_producers < 1) {
        printf("Number of producer(s) shall be greater than 0");
        return 0;
    }

    else if (num_consumers < 1) {
        printf("Number of consumer(s) shall be greater than 0");
        return 0;
    }

    // Initialise buffer
    buffer_counter = 0;
    removeItem = 0;
    addItem = 0;

    // Initialise semaphores and mutex lock
    init_locks();

    // Create producer and consumer threads
    int i = 0;
    int ret_value = -1;
    pthread_t thread_id;

    for (i = 0; i < num_producers; i++){
        int *num = malloc(sizeof(*num));
        *num = i + 1;
        ret_value = pthread_create(&thread_id, NULL, producer, (void *) num);
        if (ret_value == 0)
            printf("producer %d thread created successfully\n", i + 1);
        else
            printf("Error: %d while creating Producer %d thread\n", ret_value, i + 1);

    }

    for (i = 0; i < num_consumers; i++){
        int *num = malloc(sizeof(*num));
        *num = i + 1;
        ret_value = pthread_create(&thread_id, NULL, consumer, (void *) num);
        if (ret_value == 0)
            printf("Consumer %d thread created successfully\n", i + 1);
        else
            printf("Error: %d while creating Consumer %d thread\n", ret_value, i + 1);
    }

    // Sleep before terminating producer and consumers 
    sleep(sleep_duration);

    // Sleep time elapse, terminate the semaphores 
    // pthread_mutex_destroy(&mutex);

    sem_destroy(&mutex);
    sem_destroy(&full_count);
    sem_destroy(&empty_count);

    printf("Sleep time elapsed. Producers and Consumers terminated\n");
    printf("Number of remaining items on buffer = %d\n", buffer_counter);

    exit(0);
    return 0;
}