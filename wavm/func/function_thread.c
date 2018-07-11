#include <stdio.h>
#include <pthread.h>

void *print_message_function( void *ptr )
{
    char *message;
    message = (char *) ptr;
    printf("%s \n", message);
}

int main()
{
    pthread_t t1;
    const char *m1 = "Thread 1";
    int  r1;

    r1 = pthread_create( &t1, NULL, print_message_function, (void*) m1);
    if(r1)
    {
        printf("Error - pthread_create() return code: %d\n",r1);
    }

    printf("pthread_create() for thread 1 returns: %d\n", r1);

    pthread_join(t1, NULL);
}
