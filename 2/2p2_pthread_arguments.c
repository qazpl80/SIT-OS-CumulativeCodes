#include <stdio.h>
#include <pthread.h>

void *thread_func(void *arg)                    //correct syntax
//void* thread_func(int* arg)                   //incorrect syntax - will result in compile error
{
    printf("I am thread #%d\n", *(int *)arg);   //correct syntax
    //printf("I am thread #%d\n", *arg);        //incorrect syntax - will result in compile error
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int i = 1;
    int j = 2;

    /* Create 2 threads t1 and t2 with default attributes which will execute
    function "thread_func()" in their own contexts with specified arguments. */
    pthread_create(&t1, NULL, &thread_func, &i);
    pthread_create(&t2, NULL, &thread_func, &j);

    // This makes the main thread wait on the termination of t1 and t2.
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("In main thread\n\n\n");
    return 0;
}