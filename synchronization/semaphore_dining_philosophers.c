#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5

sem_t forks[NUM_PHILOSOPHERS];

void* philosopherThread(void* arg) {
    int id = *(int *)arg;
    int left  = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    for (int i = 0; i < 5; i++) {
        printf("Philosopher %d is thinking...\n", id);
        sleep(rand() % 1);

        sem_wait(&forks[left]);
        sem_wait(&forks[right]);

        printf("Philosopher %d is eating...\n", id);
        sleep(rand() % 1);

        sem_post(&forks[right]);
        sem_post(&forks[left]);

        printf("Philosopher %d finished eating...\n", id);
        sleep(rand() % 1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopherIds[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_init(&forks[i], 0, 1);
        philosopherIds[i] = i;
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopherThread, &philosopherIds[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_destroy(&forks[i]);
    }

    return 0;
}
