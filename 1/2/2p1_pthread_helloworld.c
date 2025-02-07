#include <pthread.h>
#include <stdio.h>
#include <string.h>

/* function to be run as a thread always must have the same signature:
   it has one void* parameter and returns void */
void* threadfunction(void *arg)
{
    printf("Inside Thread\n"); 
    printf("Hello, World!\n"); /*printf() is specified as thread-safe as of C11*/
    return NULL;
}

int main(void)
{
    /*create a new thread with default attributes and NULL passed as the argument to the start routine*/    
    pthread_t thread;
    int retcode = pthread_create(&thread, NULL, threadfunction, NULL);

    if (!retcode) /*check whether the thread creation was successful*/
    {
        int retval= pthread_join(thread, NULL); /*wait until the created thread terminates*/
        printf("return valuval e = %d\n\n\n", retval);
        return 0;
    }
    fprintf(stderr, "%s\n", strerror(retcode));
    return 1;
}