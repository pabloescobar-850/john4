//4. Thread synchronization and Deadlock handling (CO3, CO4)
//Deadlock Avoidance Using Semaphores:
//Implement the deadlock-free solution to Dining Philosophers problem to illustrate the //problem of
//deadlock and/or starvation that can occur when many synchronized threads are competing for
//limited resources.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define N 5                  // Total philosophers
#define MEALS 2              // Each philosopher eats 2 times
#define ROOM_SEMAPHORE 1     // Method 1
#define ASYMMETRIC 2         // Method 2
#define BOTH_AVAILABLE 3     // Method 3

sem_t chopstick[N];          // Semaphore for each chopstick
sem_t room;                  // Limits number of philosophers
sem_t waiter;                // Controls access to chopsticks
int fork_available[N];       // Checks whether chopstick is free
int method;                  // Stores selected method

// Philosopher thread function
void *philosopher(void *arg)
{
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    // Each philosopher eats 2 meals
    for (int meal = 1; meal <= MEALS; meal++)
    {
        printf("Philosopher %d is thinking.\n", id + 1);
        printf("Philosopher %d is hungry.\n", id + 1);

        // METHOD 1: ROOM SEMAPHORE
        if (method == ROOM_SEMAPHORE)
        {
            // Allow maximum 4 philosophers to enter
            sem_wait(&room);

            // Pick up left chopstick
            sem_wait(&chopstick[left]);
            printf("Philosopher %d picked up left chopstick.\n",
                   id + 1);

            // Pick up right chopstick
            sem_wait(&chopstick[right]);
            printf("Philosopher %d picked up right chopstick.\n",
                   id + 1);
        }

        // METHOD 2: ASYMMETRIC SOLUTION
        else if (method == ASYMMETRIC)
        {
            if (id % 2 == 0)
            {
                // Philosophers 1, 3 and 5 pick left first
                sem_wait(&chopstick[left]);
                printf("Philosopher %d picked up left chopstick.\n",
                       id + 1);

                // Then pick right
                sem_wait(&chopstick[right]);
                printf("Philosopher %d picked up right chopstick.\n",
                       id + 1);
            }
            else
            {
                // Philosophers 2 and 4 pick right first
                sem_wait(&chopstick[right]);
                printf("Philosopher %d picked up right chopstick.\n",
                       id + 1);

                // Then pick left
                sem_wait(&chopstick[left]);
                printf("Philosopher %d picked up left chopstick.\n",
                       id + 1);
            }
        }

        // METHOD 3: BOTH CHOPSTICKS AVAILABLE
        else if (method == BOTH_AVAILABLE)
        {
            struct timespec delay = {0, 1000000};

            // Keep checking until both chopsticks are free
            while (1)
            {
                // Lock waiter to check safely
                sem_wait(&waiter);

                if (fork_available[left] &&
                    fork_available[right])
                {
                    // Reserve both chopsticks together
                    fork_available[left] = 0;
                    fork_available[right] = 0;

                    // Unlock waiter
                    sem_post(&waiter);
                    break;
                }

                // Unlock if both are not available
                sem_post(&waiter);

                // Wait briefly before checking again
                nanosleep(&delay, NULL);
            }

            printf("Philosopher %d picked up left chopstick.\n",
                   id + 1);
            printf("Philosopher %d picked up right chopstick.\n",
                   id + 1);
        }

        // Philosopher starts eating
        printf("Philosopher %d is eating (meal %d).\n",
               id + 1, meal);

        // Small delay to represent eating
        struct timespec eating_delay = {0, 100000000};
        nanosleep(&eating_delay, NULL);

        // Release chopsticks after eating
        if (method == BOTH_AVAILABLE)
        {
            // Lock waiter before updating availability
            sem_wait(&waiter);

            // Mark both chopsticks as available
            fork_available[left] = 1;
            fork_available[right] = 1;

            // Unlock waiter
            sem_post(&waiter);
        }
        else
        {
            // Release left and right chopsticks
            sem_post(&chopstick[left]);
            sem_post(&chopstick[right]);

            if (method == ROOM_SEMAPHORE)
            {
                // Allow another philosopher to enter
                sem_post(&room);
            }
        }

        printf("Philosopher %d finished eating.\n\n",
               id + 1);
    }

    return NULL;
}

int main(void)
{
    // Display menu repeatedly until user exits
    while (1)
    {
        pthread_t threads[N];   // Stores philosopher threads
        int ids[N];             // Stores philosopher IDs
        int choice;             // User's menu choice
        int created = 0;        // Counts created threads
        int init_failed = 0;    // Checks initialization errors

        // Display menu
        printf("\n==============================\n");
        printf("  DINING PHILOSOPHERS PROBLEM\n");
        printf("==============================\n");

        printf("1. Room Semaphore Solution\n");
        printf("2. Asymmetric Solution\n");
        printf("3. Both Chopsticks Available Solution\n");
        printf("0. Exit\n");

        printf("Enter your choice: ");

        // Check whether input is valid
        if (scanf("%d", &choice) != 1)
        {
            int ch;

            // Clear invalid input
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
                // Clear invalid input
            }

            printf("Invalid input! Enter a number.\n");
            continue;
        }

        // Exit program when user enters 0
        if (choice == 0)
        {
            printf("Exiting program. Goodbye!\n");
            break;
        }

        // Check whether choice is between 1 and 3
        if (choice < 1 || choice > 3)
        {
            printf("Invalid choice! Select 1, 2 or 3.\n");
            continue;
        }

        // Store selected method
        method = choice;

        // Initialize room semaphore for Method 1
        if (method == ROOM_SEMAPHORE)
        {
            // Allow only N-1 philosophers at a time
            if (sem_init(&room, 0, N - 1) == -1)
            {
                perror("Room semaphore initialization failed");
                continue;
            }
        }

        // Initialize waiter for Method 3
        if (method == BOTH_AVAILABLE)
        {
            if (sem_init(&waiter, 0, 1) == -1)
            {
                perror("Waiter semaphore initialization failed");
                continue;
            }

            // Initially all chopsticks are available
            for (int i = 0; i < N; i++)
            {
                fork_available[i] = 1;
            }
        }
        else
        {
            // Initialize each chopstick semaphore
            for (int i = 0; i < N; i++)
            {
                if (sem_init(&chopstick[i], 0, 1) == -1)
                {
                    perror("Chopstick initialization failed");

                    // Destroy previously initialized semaphores
                    for (int j = 0; j < i; j++)
                    {
                        sem_destroy(&chopstick[j]);
                    }

                    // Destroy room semaphore if Method 1
                    if (method == ROOM_SEMAPHORE)
                    {
                        sem_destroy(&room);
                    }

                    init_failed = 1;
                    break;
                }
            }
        }

        // Return to menu if initialization fails
        if (init_failed)
        {
            continue;
        }

        printf("\nStarting selected solution...\n\n");

        // Create 5 philosopher threads
        for (int i = 0; i < N; i++)
        {
            ids[i] = i;

            int result = pthread_create(
                &threads[i],
                NULL,
                philosopher,
                &ids[i]
            );

            // Check whether thread creation failed
            if (result != 0)
            {
                fprintf(stderr,
                        "Failed to create philosopher thread: %d\n",
                        result);
                break;
            }

            created++;
        }

        // Wait for all created threads to finish
        for (int i = 0; i < created; i++)
        {
            pthread_join(threads[i], NULL);
        }

        // Destroy waiter semaphore for Method 3
        if (method == BOTH_AVAILABLE)
        {
            sem_destroy(&waiter);
        }
        else
        {
            // Destroy all chopstick semaphores
            for (int i = 0; i < N; i++)
            {
                sem_destroy(&chopstick[i]);
            }
        }

        // Destroy room semaphore for Method 1
        if (method == ROOM_SEMAPHORE)
        {
            sem_destroy(&room);
        }

        printf("\nCurrent run completed.\n");
        printf("Returning to menu...\n");
    }

    return 0;
}