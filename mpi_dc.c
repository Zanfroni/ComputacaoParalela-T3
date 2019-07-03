#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "mpi.h" 

//#define DEBUG 1            // comentar esta linha quando for medir tempo
#define ARRAY_SIZE 1000000      // trabalho final com o valores 10.000, 100.000, 1.000.000
#define DELTA 31250

int *interleaving(int vetor[], int tam);

void bs(int n, int * vetor);

int *interleaving(int vetor[], int tam)
{
	int *vetor_auxiliar;
	int i1, i2, i_aux;

	vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

	i1 = 0;
	i2 = tam / 2;

	for (i_aux = 0; i_aux < tam; i_aux++) {
		if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
		    || (i2 == tam))
			vetor_auxiliar[i_aux] = vetor[i1++];
		else
			vetor_auxiliar[i_aux] = vetor[i2++];
	}

	return vetor_auxiliar;
}

void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int main(int argc, char **argv) 
{ 
   MPI_Status status, statusPai; 

   MPI_Init(&argc,&argv);

   int my_rank, proc_n;

   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

   int vetor[ARRAY_SIZE];
   int *new_vector;
   int i,n;

   int left_child = 2 * my_rank + 1;
   int right_child = 2 * my_rank + 2;
   clock_t start, end;
   double cpu_time_used;

   if(my_rank != 0) {

      MPI_Recv(vetor, ARRAY_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &statusPai);
      MPI_Get_count(&statusPai, MPI_INT, &n);
		
   } else {

	  start = clock();
      for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
        vetor[i] = ARRAY_SIZE-i;
      n = ARRAY_SIZE;

   } 
   if(n <= DELTA) {
      bs(n, vetor);                     /* sort array */
      new_vector = vetor;
      
   } else {
      
      MPI_Send(&vetor[0], n/2, MPI_INT, left_child, 1, MPI_COMM_WORLD);
      MPI_Send(&vetor[n/2], n/2, MPI_INT, right_child, 1, MPI_COMM_WORLD);
      
      MPI_Recv(&vetor[0], n/2, MPI_INT, left_child, 1, MPI_COMM_WORLD, &status);
      MPI_Recv(&vetor[n/2], n/2, MPI_INT, right_child, 1, MPI_COMM_WORLD, &status);

      new_vector = interleaving(vetor, n);
      
   }

   if(my_rank != 0) {
      
      MPI_Send(new_vector, n, MPI_INT, statusPai.MPI_SOURCE, 1, MPI_COMM_WORLD);

   } else {
      
    #ifdef DEBUG
    printf("\nVetor: ");
    for (i=0 ; i<ARRAY_SIZE; i++)                              /* print sorted array */
        printf("[%03d] ", new_vector[i]);
    #endif
	end = clock();
	cpu_time_used = ((double) (end-start)) / CLOCKS_PER_SEC;
	printf("\nTime that took to sort was %.2f seconds\n",cpu_time_used);

   }
   MPI_Finalize();
   return 0; 
} 
