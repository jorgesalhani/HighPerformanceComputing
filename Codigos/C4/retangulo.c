#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int *base, *altura;
int *perimetro, *area;
int *diagonal;

void ret_perimetro(int dim) {
    int i;
    #pragma omp for simd
    for (i = 0; i < dim; i++) {
        perimetro[i] = (base[i] * 2) + (altura[i] * 2);
    }
}

void ret_area(int dim) {
    int i;
    #pragma omp for simd
    for (i = 0; i < dim; i++) {
        area[i] = base[i] * altura[i];
    }
}

void ret_diagonal(int dim) {
    double num;
    int i;
    #pragma omp for simd
    for (i = 0; i < dim; i++) {
        num = (pow(base[i], 2) + pow(altura[i], 2));
        diagonal[i] = sqrt(num);
    }
}

int main() {
    int i, dim;

    omp_set_nested(1);

    fscanf(stdin, "%d\n", &dim);

    base = (int*)malloc(sizeof(int) * dim);
    altura = (int*)malloc(sizeof(int) * dim);

    perimetro = (int*)malloc(sizeof(int) * dim);
    area = (int*)malloc(sizeof(int) * dim);
    diagonal = (double*)malloc(sizeof(double) * dim);

    for (i = 0; i < dim; i++) fscanf(stdin, "%d ", &(base[i]));
    for (i = 0; i < dim; i++) fscanf(stdin, "%d ", &(altura[i]));

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                ret_perimetro(dim);
            }

            #pragma omp task
            {
                ret_area(dim);
            }

            #pragma omp task
            {
                ret_diagonal(dim);
            }
        }
    }

    for (i = 0; i < dim; i++) {
        printf("base[%d]=%d, alt[%d]=%d, per[%d]=%d, area[%d]=%d, diag[%d]=%.2f\n", i, base[i], i, altura[i], i, perimetro[i], i, area[i], i, diagonal[i]);
        fflush(0);
    }
    
    return 0;
}