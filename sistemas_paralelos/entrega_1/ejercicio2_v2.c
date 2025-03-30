#include <stdio.h>
#include <stdlib.h>

// Dimension por defecto de las matrices
int N = 100;
int BLOCK_SIZE = 10;
int imprimir = 0;

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

    double *A, *B, *C, *R, *AxB, *CxBT, *BT;

    // Alocamos memoria para las matrices
    A = (double *)malloc(sizeof(double) * N * N);
    B = (double *)malloc(sizeof(double) * N * N);
    C = (double *)malloc(sizeof(double) * N * N);
    R = (double *)malloc(sizeof(double) * N * N);

    AxB = (double *)malloc(sizeof(double) * N * N);
    BT = (double *)malloc(sizeof(double) * N * N);
    CxBT = (double *)malloc(sizeof(double) * N * N);

    int i, j, k, ii, jj, kk;

    // Inicializa las matrices A y B en 1
    for (i = 0; i < NxN; i++)
    {
        A[i] = 1;
        B[i] = 1;
        C[i] = 1;
        AxB[i] = 0;
        CxBT[i] = 0;
        R[i] = 0;
    }

  
    printMatrix(A, N, 1, "Matriz A");
    printMatrix(B, N, 0, "Matriz B");

    // Comienza el calculo
    double timetick = dwalltime();

    double maxA = A[0]; 
    double maxB = B[0];
    double minA = A[0];
    double minB = B[0];
    double promA = 0;
    double promB = 0;

    // Calculamos maximos, minimos, y promedio de A y B
    for (i = 0; i < NxN; i++)
    {
        double aux = A[i];
        if (aux > maxA)
        {
            maxA = aux;
        }
        if (aux < minA)
        {
            minA = aux;
        }
        promA += aux;
    }
    promA = promA / NxN;

    for (i = 0; i < NxN; i++)
    {
        double aux = B[i];
        if (aux > maxB)
        {
            maxB = aux;
        }
        if (aux < minB)
        {
            minB = aux;
        }
        promB += aux;
    }

    promB = promB / NxN;

    double escalar = (maxA * maxB - minA * minB) / (promA * promB);

    if (imprimir > 0)
    {

        printf("===================================\n");
        printf("promA: %f \n", promA);
        printf("promB: %f \n", promB);
        printf("minA: %f \n", minA);
        printf("minB: %f \n", minB);
        printf("escalar: %f \n", escalar);
        printf("===================================\n");
    }

    // A x B (Multiplicacion por bloques)

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
                            AxB[i * N + j] += A[i * N + k] * B[j * N + k];
                        }
                    }
                }
            }
        }
    }

    // Multiplicamos AxB por el escalar calculado anteriormente
    for (i = 0; i < NxN; i++)
    {
        AxB[i] = AxB[i] * escalar;
    }

    printMatrix(AxB, N, 1, "A x B x Escalar");

    // Armamos B transpuesta

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            BT[j * N + i] = B[i * N + j];
        }
    }

    // C x B Transpuesta (Multiplicacion por Bloques)
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
                            CxBT[i * N + j] += C[i * N + k] * BT[j * N + k];
                        }
                    }
                }
            }
        }
    }

    printMatrix(CxBT, N, 1, "C x B Transversa");
   
    //AxB(x escalar) + CxBT
    //Ambas matrices estan almacenadas por fila
    for (i = 0; i < NxN; i++)
    {
        R[i] = AxB[i] + CxBT[i];
    }

    double elapsed = dwalltime() - timetick;
    printMatrix(R, N, 1, "RESULTADO");

    printf("\nTiempo en segundos %f\n", elapsed);
}