#include <stdio.h>
#include <omp.h>

int main(int argc, char* argv[]) {
    #pragma omp parallel 
    {
        int threadId = omp_get_thread_num();
        printf("Hello from thread %d\n", threadId);
    }
    return 0;
}