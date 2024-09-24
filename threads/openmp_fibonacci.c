// With using OpenMP's recursive parallelism, we can implement the Fibonacci sequence calculation
// in a similar way to the matrix multiplication example. The following code snippet shows how to

// gcc -o openmp_fibonacci.out ./openmp_fibonacci.c -fopenmp

#include <omp.h>
#include <stdio.h>

long fibonacci(int n) {
    long x, y;
    if (n < 2) {
        return n;
    } else {
        #pragma omp task shared(x)      // Creates a parallel task for the recursive call
        x = fibonacci(n - 1);
        #pragma omp task shared(y)      // Creates a parallel task for the recursive call
        y = fibonacci(n - 2);
        #pragma omp taskwait            // Wait for the tasks to finish
        return x + y;
    }
}

int main(int argc, char* argv[]) {
    int n = 30;
    long result;

    #pragma omp parallel
    {
        #pragma omp single              // Only one thread executes the following block
        {
            result = fibonacci(n);
        }
    }

    printf("Fibonacci(%d) = %ld\n", n, result);
}