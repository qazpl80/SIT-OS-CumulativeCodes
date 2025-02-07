#include <stdio.h>
#include <pthread.h>

int x = 0;   // global variable

void *fun(void *n)
{
    int i;
    //for ( i = 0; i < 1000; i++ )        // for a shorter loop, "race" is mostly not observed
    for ( i = 0; i < 1000000; i++ )       // for a larger loop, "race" can be observed  
        x++;
}

int main()
{
    pthread_t t1, t2;
    printf("Start >> x is: %d\n", x);

    pthread_create(&t1, NULL, fun, NULL);
    pthread_create(&t2, NULL, fun, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("End   >> x is: %d\n\n\n", x);
    return 0;
}