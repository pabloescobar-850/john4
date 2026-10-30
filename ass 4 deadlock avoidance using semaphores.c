//4. Thread synchronization and Deadlock handling (CO3, CO4)
//Deadlock Avoidance Using Semaphores:
//Implement the deadlock-free solution to Dining Philosophers problem to illustrate the //problem of
//deadlock and/or starvation that can occur when many synchronized threads are competing for
//limited resources.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5

sem_t fork_s[N];
sem_t room;

// philosopher
void *philosopher(void *arg)
{
    int id = *(int *)arg;

    printf("Philosopher is thinking \n");

    sem_wait(&room);
    sem_wait(&fork_s[id]);
    sem_wait(&fork_s[(id + 1) % N]);

    printf("Philosopher is eating \n");

    sem_post(&fork_s[(id + 1) % N]);
    sem_post(&fork_s[id]);
    sem_post(&room);

    return NULL;
}

// main
int main()
{
    // initialize
    pthread_t t[N];
    int id[N], i;

    sem_init(&room, 0, N - 1);

    for (i = 0; i < N; i++)
    {
        sem_init(&fork_s[i], 0, 1);
    }

    // create threads
    for (i = 0; i < N; i++)
    {
        id[i] = i;
        pthread_create(&t[i], NULL, philosopher, &id[i]);
    }

    // join threads
    for (i = 0; i < N; i++)
    {
        pthread_join(t[i], NULL);
    }

    // destroy semaphores
    sem_destroy(&room);

    for (i = 0; i < N; i++)
    {
        sem_destroy(&fork_s[i]);
    }

    return 0;
}


//  how to run is here 
//  gcc name.c -o name -pthread
// ./name
