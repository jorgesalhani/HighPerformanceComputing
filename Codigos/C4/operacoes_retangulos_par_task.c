//Sample solution for parallel programming challenge.
//Copyright (C) 2019 Gabriel Martins da Silva
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//Last Revision, November 2019 by Gabriel Martins da Silva.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// variaveis globais
int *base, *altura;
int *perimetro, *area;
double *diagonal;

void ret_perimetro(int dim)
{
    int i;
	
	#pragma omp parallel for simd
	for (i = 0; i < dim; i++)
    {
            //printf("i = %d thread %d\n",i,omp_get_thread_num());
            perimetro[i] = (base[i] * 2) + (altura[i] * 2);
    }
}

void ret_area(int dim)
{
    int i;

	#pragma omp parallel for simd
    for (i = 0; i < dim; i++)
    {
        //printf("i = %d thread %d\n",i,omp_get_thread_num());
        area[i] = base[i] * altura[i];
    }
}

void ret_diagonal(int dim)
{
    double num;
    int i;

	#pragma omp parallel omp for simd
    for (i = 0; i < dim; i++)
    {
        //printf("i = %d thread %d\n",i,omp_get_thread_num());
        num = (pow(base[i], 2) + pow(altura[i], 2));
        diagonal[i] = sqrt(num);
    }
}

int main()
{
    int i, dim;

    omp_set_nested(1);

    fscanf(stdin, "%d\n", &dim); // Lê a dimensão dos vetores

    // Aloca os vetores
    base = (int *)malloc(dim * sizeof(int));
    altura = (int *)malloc(dim * sizeof(int));

    perimetro = (int *)malloc(dim * sizeof(int));
    area = (int *)malloc(dim * sizeof(int));
    diagonal = (double *)malloc(dim * sizeof(double));

    for (i = 0; i < dim; i++)
        fscanf(stdin, "%d ", &(base[i])); // Lê o vetor base
    for (i = 0; i < dim; i++)
        fscanf(stdin, "%d ", &(altura[i])); // Lê o vetor altura

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

    for (i = 0; i < dim; i++)
    {
        printf("base[%d]=%d, alt[%d]=%d, per[%d]=%d, area[%d]=%d, diag[%d]=%.2f\n", i, base[i], i, altura[i], i, perimetro[i], i, area[i], i, diagonal[i]);
        fflush(0);
    }

    return 0;
}