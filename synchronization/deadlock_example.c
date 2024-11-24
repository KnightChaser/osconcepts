#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void *thread1Function(void *arg) {
    printf("Thread 1 locks mutex\n");
    pthread_mutex_lock(&mutex1);
    sleep(1);
    printf("Thread 1 attempts to lock mutex 2");
    pthread_mutex_lock(&mutex2);

    printf("Thread 1 acquired both mutexes\n");

    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void *thread2Function(void *arg) {
    printf("Thread 2 locks mutex\n");
    pthread_mutex_lock(&mutex2);
    sleep(1);
    printf("Thread 2 attempts to lock mutex 1");
    pthread_mutex_lock(&mutex1);

    printf("Thread 2 acquired both mutexes\n");

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread1, thread2;

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    pthread_create(&thread1, NULL, thread1Function, NULL);
    pthread_create(&thread2, NULL, thread2Function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    printf("Main thread exits\n");

    return 0;
}
