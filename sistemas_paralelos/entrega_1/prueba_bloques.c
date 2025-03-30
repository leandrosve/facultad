#include <stdio.h>
#include <stdlib.h>

// Dimension por defecto de las matrices
int N = 100;
int BLOCK_SIZE = 10;
int imprimir = 1;

// Para calcular tiempo
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

void printMatrix(double *matrix, int N, int byRow, const char *title)
{
    if (imprimir <= 0)
        return;
    printf("%s\n", title);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            int index = byRow ? (i * N + j) : (j * N + i);
            printf("%f ", matrix[index]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{

    // Recibimos parametro con la dimension de la matriz
    if ((argc != 3) || ((N = atoi(argv[1])) <= 0) || ((BLOCK_SIZE = atoi(argv[2])) <= 0))
    {
        printf("\nUsar: %s N  BLOCK_SIZE\n", argv[0]);
        exit(1);
    }

    if (((N % BLOCK_SIZE) != 0))
    {
        printf("\nError: N debe ser multiplo de BLOCK_SIZE \n");
        exit(1);
    }

    int NxN = N * N;

    double *A, *B, *C ;

    // Alocamos memoria para las matrices
    A = (double *)malloc(sizeof(double) * N * N);
    B = (double *)malloc(sizeof(double) * N * N);
    C = (double *)malloc(sizeof(double) * N * N);

    int i, j, k, ii, jj, kk;

    // Inicializa las matrices A y B en 1
    for (i = 0; i < NxN; i++)
    {
        A[i] = 5;
        B[i] = 0;
        C[i] = 0;
    }

    for (i = 0; i < N; i++)
    {
        B[i * N + i] =  1;
    }

  
    printMatrix(A, N, 1, "Matriz A");
    printMatrix(B, N, 0, "Matriz B");

    // Comienza el calculo
    double timetick = dwalltime();

   
   
    // A x B (Multiplicacion por bloques)
    
    int posA, posB, posC;
    for (i = 0; i < N; i += BLOCK_SIZE)
    {
        for (j = 0; j < N; j += BLOCK_SIZE)
        {
            for (k = 0; k < N; k += BLOCK_SIZE)
            {

                for (ii = 0; ii < BLOCK_SIZE; ii++)
                {
                    for (jj = 0; jj < BLOCK_SIZE; jj++)
                    {
                        for (kk = 0; kk < BLOCK_SIZE; kk++)
                        {

                            posC = (i + ii) * N + (j + jj);
                            posA = (i + ii) * N + (k + kk);
                            posB = (k + kk) * N + (j + jj);
                            C[posC] += A[posA] * B[posB];
                        }
                    }
                }
            }
        }
    }

    
    printMatrix(C, N, 1, "A x B");

}