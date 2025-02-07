#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX 999
#define BUFFER_SIZE 5

pthread_mutex_t mutex;
int buffer[BUFFER_SIZE];
sem_t empty;
sem_t full;
bool active = true;

void *producer(void *);
void *consumer(void *);
int randGenerator(int);

int main(int argc, char const *argv[])
{
    /* Seed the random number generator */
    srand(time(NULL));
    int sNo, pNo, cNo;
    if (argc != 4)
    {
        printf("Invalid number of arguments!\n");
        exit(0);
    }
    else
    {
        sNo = atoi(argv[1]);
        pNo = atoi(argv[2]);
        cNo = atoi(argv[3]);
        printf("Number of arguments = %d\n", argc - 1);
        printf("Main Process Sleep Time = %d\n", sNo);
        printf("Number of Producers = %d\n", pNo);
        printf("Number of Consumers = %d\n", cNo);
    }

    // initialise the semaphores empty and full
    // counters to count if the
    if (sem_init(&empty, 0, BUFFER_SIZE) != 0)
    {
        printf("Failed to initialise empty semaphore!\n");
        exit(0);
    }
    if (sem_init(&full, 0, 0) != 0)
    {
        printf("Failed to initialise full semaphore!\n");
        exit(0);
    }

    //initialise mutex
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("Failed to initialise mutex!\n");
        exit(0);
    }

    // declare array to track producer and consumers
    pthread_t pro[pNo], con[cNo];
    int pCount[pNo], cCount[cNo];

    //initialise producers
    for (int i = 0; i < pNo; i++)
    {
        pCount[i] = i + 1;
        pthread_create(&pro[i], NULL, &producer, &pCount[i]);
        printf("Producer-%d thread created successfully\n", i + 1);
    }

    //initialise consumers
    for (int i = 0; i < cNo; i++)
    {
        cCount[i] = i + 1;
        pthread_create(&con[i], NULL, &consumer, &cCount[i]);
        printf("Consumer-%d thread created successfully\n", i + 1);
    }

    //will sleep for sNo amount of time
    sleep(sNo);
    active = false;
    printf("Sleep time elapsed. Producers and Consumers terminated.\n");

    //waiting for threads to terminate
    for (int i = 0; i < pNo; ++i)
    {
        pthread_join(pro[i], NULL);
    }

    for (int i = 0; i < cNo; ++i)
    {
        pthread_join(con[i], NULL);
    }

    //leftovers
    int leftover = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (buffer[i] != 0)
            leftover++;
    }
    printf("Number of items in buffer = %d\n", leftover);

    /* Destroy the mutex & semaphores */
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    exit(0);
}

int randGenerator(int max)
{
    // + 1 to include 999 and exclude 0
    return (rand() % max) + 1;
}

// function for producer threads
void *producer(void *args)
{
    // retrieve thread number
    int thread = *(int *)args;

    while (active)
    {
        //generate a random number to put inside buffer
        int input = randGenerator(MAX);
        //decrements empty counter
        sem_wait(&empty);
        // lock until after adding to buffer
        pthread_mutex_lock(&mutex);
        // get the value of full -> full inidicates the index of the array to add
        int index;
        sem_getvalue(&full, &index);
        //append the random generated number to the array at index
        buffer[index] = input;
        if (active)
        {
            printf("Producer-%d inserted item %d\n", thread, input);
        }
        //release hold
        pthread_mutex_unlock(&mutex);
        //increment full counter
        sem_post(&full);
        //put producer to sleep
        sleep(randGenerator(3));
    }

    sem_post(&empty);
}

//function for the consumer threads
void *consumer(void *args)
{
    // retrieve thread number
    int thread = *(int *)args;

    while (active)
    {
        //decrement full counter
        sem_wait(&full);
        // lock until after adding to buffer
        pthread_mutex_lock(&mutex);
        // get the value of full -> full inidicates the index of the array to consume
        int index;
        sem_getvalue(&full, &index);
        int next = buffer[index];
        buffer[index] = 0;
        if (active)
        {
            printf("Consumer-%d removed item \t%d\n", thread, next);
        }
        //release hold
        pthread_mutex_unlock(&mutex);
        //increment empty counter
        sem_post(&empty);
        //put producer to sleep
        sleep(randGenerator(3));
    }

    sem_post(&full);
}
