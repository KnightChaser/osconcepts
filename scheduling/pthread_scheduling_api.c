#include <stdio.h>
#include <pthread.h>
#define NUMBER_OF_THREADS 5

void *runner (void *param) {
	printf("Thread %ld\n", pthread_self());
	pthread_exit(0);
}

int main(int argc, char* argv[]) {
	int i, scope;
	pthread_t tid[NUMBER_OF_THREADS];
	pthread_attr_t attr;

	// Get the default attributes
	pthread_attr_init(&attr);

	// First, inquire on the current scope
	if (pthread_attr_getscope(&attr, &scope) != 0)
		printf("Unable to get scheduling scope\n");
	else {
		if (scope == PTHREAD_SCOPE_PROCESS)
			printf("PTHREAD_SCOPE_PROCESS\n");
		else if (scope == PTHREAD_SCOPE_SYSTEM)
			printf("PTHREAD_SCOPE_SYSTEM\n");
		else
			fprintf(stderr, "Illegal scope value.\n");
	}

	// Set the scheduling algorithm to PCS or SCS
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	// Create and join threads
	for (int index = 0; index < NUMBER_OF_THREADS; index++)
		pthread_create(&tid[index], &attr, runner, NULL);

	for (int index = 0; index < NUMBER_OF_THREADS; index++)
		pthread_join(tid[index], NULL);
}


