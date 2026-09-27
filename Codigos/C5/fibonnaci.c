#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int fib(int n) {
    int x, y;

    if (n < 2) return n;

    #pragma omp task shared(x)
    {
        x = fib(n-1);
    }

    #pragma omp task shared(y)
    {
        y = fib(n-2);
    }

    #pragma omp taskwait

    return x + y;
}

int main(int argc, char** argv) {
    int result;

    if (argc != 2) {
        printf("Need argument: <N>\n");
        exit(0);
    }

    int n = atoi(argv[1]);

    #pragma omp parallel
    {
        #pragma omp single
        {
            result = fib(n);
        }
    }

    printf("Fibonacci(%d) = %d\n", n, result);
    fflush(0);

    omp_pause_resource_all(omp_pause_hard);
    return 0;
}