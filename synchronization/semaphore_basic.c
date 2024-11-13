#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

sem_t semaphore;
int counter = 0;

void* increment(void* arg) {
    for (int i = 0; i < 5; i++) {
        sem_wait(&semaphore);
        counter++;
        printf("Thread %ld: %d\n", (long)arg, counter);
        sem_post(&semaphore);
        sleep(1);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t t1, t2;
    sem_init(&semaphore, 0, 1); // Initialize semaphore with value 1 (binary semaphore)
    
    pthread_create(&t1, NULL, increment, (void*)1);
    pthread_create(&t2, NULL, increment, (void*)2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&semaphore);
    printf("Final counter value: %d\n", counter);

    return 0;
}
