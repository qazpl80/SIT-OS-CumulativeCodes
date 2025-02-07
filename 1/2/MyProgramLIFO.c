#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define BUFFER_SIZE 5 // Define the size of the buffer

int buffer[BUFFER_SIZE]; // Shared buffer
int count = 0; // Count of items in the buffer
int top = -1; // Index for the top of the stack
int stop = 0; // Global flag to signal threads to stop

sem_t empty; // Semaphore to track empty slots in the buffer
sem_t full; // Semaphore to track full slots in the buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect buffer access

// Producer thread function
void *producer(void *param) {
    int id = *((int *)param); // Get the producer ID
    while (!stop) { // Loop until stop flag is set
        int item = rand() % 999 + 1; // Produce a random item
        if (sem_wait(&empty) == -1 && stop) break; // Wait if buffer is full, break if stop flag is set
        pthread_mutex_lock(&mutex); // Lock the buffer
        if (stop) { // Check stop flag again after acquiring the lock
            pthread_mutex_unlock(&mutex);
            sem_post(&empty);
            break;
        }
        buffer[++top] = item; // Insert item into buffer
        count++; // Increment item count
        printf("Producer%d inserted item %d\n", id, item); // Print message
        pthread_mutex_unlock(&mutex); // Unlock the buffer
        sem_post(&full); // Signal that buffer has a new item
        sleep(rand() % 3 + 1); // Sleep for a random time
    }
    return NULL; // Exit thread
}

// Consumer thread function
void *consumer(void *param) {
    int id = *((int *)param); // Get the consumer ID
    while (!stop) { // Loop until stop flag is set
        if (sem_wait(&full) == -1 && stop) break; // Wait if buffer is empty, break if stop flag is set
        pthread_mutex_lock(&mutex); // Lock the buffer
        if (stop) { // Check stop flag again after acquiring the lock
            pthread_mutex_unlock(&mutex);
            sem_post(&full);
            break;
        }
        int item = buffer[top--]; // Remove item from buffer
        count--; // Decrement item count
        printf("Consumer%d removed item %d\n", id, item); // Print message
        pthread_mutex_unlock(&mutex); // Unlock the buffer
        sem_post(&empty); // Signal that buffer has an empty slot
        sleep(rand() % 3 + 1); // Sleep for a random time
    }
    return NULL; // Exit thread
}

int main(int argc, char *argv[]) {
    if (argc != 4) { // Check if the number of arguments is correct
        fprintf(stderr, "Usage: %s <sleep_time> <num_producers> <num_consumers>\n", argv[0]); // Print usage message
        return -1; // Exit with error
    }

    int sleep_time = atoi(argv[1]); // Get sleep time from arguments
    int num_producers = atoi(argv[2]); // Get number of producers from arguments
    int num_consumers = atoi(argv[3]); // Get number of consumers from arguments

    if (sleep_time <= 0 || num_producers <= 0 || num_consumers <= 0) { // Validate inputs
        fprintf(stderr, "Invalid input. All parameters must be positive integers.\n"); // Print error message
        return -1; // Exit with error
    }

    pthread_t producers[num_producers]; // Array to hold producer threads
    pthread_t consumers[num_consumers]; // Array to hold consumer threads
    int producer_ids[num_producers]; // Array to hold producer IDs
    int consumer_ids[num_consumers]; // Array to hold consumer IDs

    sem_init(&empty, 0, BUFFER_SIZE); // Initialize empty semaphore
    sem_init(&full, 0, 0); // Initialize full semaphore

    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < num_producers; i++) { // Create producer threads
        producer_ids[i] = i + 1; // Assign ID to producer
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]); // Create producer thread
        printf("Producer%d thread created successfully\n", i + 1); // Print message
    }

    for (int i = 0; i < num_consumers; i++) { // Create consumer threads
        consumer_ids[i] = i + 1; // Assign ID to consumer
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]); // Create consumer thread
        printf("Consumer%d thread created successfully\n", i + 1); // Print message
    }

    sleep(sleep_time); // Main thread sleeps for the specified time

    stop = 1; // Signal threads to stop

    // Signal semaphores to unblock any waiting threads
    for (int i = 0; i < num_producers; i++) {
        sem_post(&empty);
    }
    for (int i = 0; i < num_consumers; i++) {
        sem_post(&full);
    }

    // Wait for all producer threads to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL); // Wait for producer thread to finish
    }

    // Wait for all consumer threads to finish
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL); // Wait for consumer thread to finish
    }

    printf("Sleep time elapsed. Producers and Consumers terminated\n"); // Print message
    printf("Number of remaining items on buffer = %d\n", count); // Print remaining items in buffer

    sem_destroy(&empty); // Destroy empty semaphore
    sem_destroy(&full); // Destroy full semaphore

    return 0; // Exit program
}