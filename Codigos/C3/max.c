#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define T 4

int main (int argc, char **argv) {
    int *vetor, maior = -1, tam, num_threads;
    double wtime;

    omp_lock_t my_lock;
    omp_init_lock(&my_lock);

    if (argc != 2) {
        printf("Arg needed: <totla_elements>\n");
        exit(0);
    }

    tam = atoi(argv[1]);
    printf("Total in vector: %d\n", tam);
    fflush(0);

    vetor = (int*)malloc(sizeof(int) * tam);

    wtime = omp_get_wtime();

    #pragma omp parallel num_threads(T) shared(maior)
    {
        int i, my_id, my_range, my_first_i, my_last_i;
        int localmaior;

        my_id = omp_get_thread_num();
        num_threads = omp_get_num_threads();

        my_range = tam / num_threads;
        my_first_i = my_range * my_id;

        if (my_id < num_threads - 1) {
            my_last_i = my_first_i + my_range;
        } else {
            my_last_i = tam;
        }

        for (i = my_first_i; i < my_last_i;i++) vetor[i] = 1;

        #pragma omp barrier

        #pragma omp single
        {
            vetor[tam/2] = tam;
        }

        localmaior = -1;

        for (i = my_first_i; i < my_last_i; i++) {
            if (vetor[i] > localmaior) localmaior = vetor[i];
        }

        omp_set_lock(&my_lock);
        if (localmaior > maior) maior = localmaior;

        omp_unset_lock(&my_lock);
    }

    wtime = omp_get_wtime() - wtime;

    printf("OMP Barrier: Tam=%d, Num_Threads=%d, maior=%d, Elapsed wall clock time = %f  \n", tam, num_threads, maior, wtime);
    free(vetor);

    omp_pause_resource_all(omp_pause_hard);

    return 0;
}