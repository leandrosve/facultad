#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi.h"

#define COORDINADOR 0

/* Multiplicacion de matrices por bloque*/
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs)
{
    int i, j, k; /* Guess what... again... */

    for (i = 0; i < bs; i++)
    {
        for (j = 0; j < bs; j++)
        {
            for (k = 0; k < bs; k++)
            {
                cblk[i * n + j] += ablk[i * n + k] * bblk[j * n + k];
            }
        }
    }
}

int main(int argc, char *argv[])
{

    
    int RANGO;
    int CANT_PROCESOS;
    
    double startTime, endTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &CANT_PROCESOS);
    MPI_Comm_rank(MPI_COMM_WORLD, &RANGO);
    MPI_Status status;

    int N, NxN, ROWS_PER_PROCESS, ELEMENTS_PER_PROCESS, BLOCK_SIZE;

     // Controlar los argumentos del programa

    if ((argc != 3) || ((N = atoi(argv[1])) <= 0) || ((BLOCK_SIZE = atoi(argv[2])) <= 0))
    {
        printf("\nUsar: %s N tamaño_bloque\n", argv[0]);
        exit(1);
    }

    if (((N % BLOCK_SIZE) != 0))
    {
        printf("\nError: N debe ser multiplo de BLOCK_SIZE \n");
        exit(1);
    }

    if (N % CANT_PROCESOS != 0)
    {
        printf("\nLa dimension de la matriz debe ser divisible por la cantidad de procesos\n");
        exit(1);
    }

    if ((N / BLOCK_SIZE % CANT_PROCESOS) != 0)
    {
        printf("\nLa cantidad de procesos debe ser consistente con la cantidad total de bloques\n");
        exit(1);
    }
        

    // Matrices enteras y strips
    double *A, *B, *BT, *C, *R;
    double *stripA, *stripAxB, *stripC, *stripCxBT, *stripR;

    // Minimos y maximos globales
    double maxA, maxB, minA, minB, promA, promB, escalar;

    // Minimos y maximos locales
    double minALocal, maxALocal, minBLocal, maxBLocal, totalALocal, totalBLocal;

    NxN = N * N;
    ROWS_PER_PROCESS = N / CANT_PROCESOS;
    ELEMENTS_PER_PROCESS = ROWS_PER_PROCESS * N;

    int i, j, k;


    // Para iterar sobre los strips
    int inferior = RANGO * ELEMENTS_PER_PROCESS;
    int superior = inferior + ELEMENTS_PER_PROCESS - 1;

    // Alocacion de memoria

    // Para B y BT, todos los procesos necesitan la totalidad de la matriz
    B = (double *)malloc(sizeof(double) * NxN);
    BT = (double *)malloc(sizeof(double) * NxN);

    stripA = (double *)malloc(sizeof(double) * ELEMENTS_PER_PROCESS);
    stripAxB = (double *)malloc(sizeof(double) * ELEMENTS_PER_PROCESS);
    stripC = (double *)malloc(sizeof(double) * ELEMENTS_PER_PROCESS);
    stripCxBT = (double *)malloc(sizeof(double) * ELEMENTS_PER_PROCESS);
    stripR = (double *)malloc(sizeof(double) * ELEMENTS_PER_PROCESS);

    // Si soy el coordinador
    if (RANGO == COORDINADOR)
    {
        A = (double *)malloc(sizeof(double) * NxN);
        C = (double *)malloc(sizeof(double) * NxN);
        R = (double *)malloc(sizeof(double) * NxN);

        // Inicializacion de las matrices
        int i, j, k;
        for (i = 0; i < NxN; i++)
        {
            A[i] = (i % 2) + 1;
            B[i] = (i % 4) + 1;
            C[i] = 1;
        }
    }

    // Inicializar los strips con ceros para que la multiplicacion de correctamente
    for (i = 0; i < ROWS_PER_PROCESS * N; i++)
    {
        stripAxB[i] = 0;
        stripCxBT[i] = 0;
    }

    // Hacemos un barrier para garantizar que los procesos se iniciaron correctamente y estan listos para comenzar
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (RANGO == COORDINADOR) {
        startTime = MPI_Wtime();
    }


    // Distribuir las filas de la matriz A
    MPI_Scatter(A, ELEMENTS_PER_PROCESS, MPI_DOUBLE, stripA, ELEMENTS_PER_PROCESS, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    // Enviar B  atodos los procesos
    MPI_Bcast(B, NxN, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    // Distribuir las filas de la matriz C
    MPI_Scatter(C, ROWS_PER_PROCESS * N, MPI_DOUBLE, stripC, ROWS_PER_PROCESS * N, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);


    // ======= Parte 1: Calculo del escalar ========
    minALocal = stripA[0];
    maxALocal = minALocal;
    minBLocal = B[inferior];
    maxBLocal = minBLocal;
    totalALocal = 0;
    totalBLocal = 0;

    for (i = 0; i < ELEMENTS_PER_PROCESS; i++)
    {
        double aux = stripA[i];
        if (aux > maxALocal)
        {
            maxALocal = aux;
        }
        if (aux < minALocal)
        {
            minALocal = aux;
        }
        totalALocal += aux;
    }

    for (i = inferior; i <= superior; i++)
    {
        double aux = B[i];

        if (aux > maxBLocal)
        {
            maxBLocal = aux;
        }
        if (aux < minBLocal)
        {
            minBLocal = aux;
        }
        totalBLocal += aux;
    }

    // printf("Proceso %d - maxALocal: %f - minALocal: %f - totalALocal: %f \n", rango, maxALocal, minALocal, totalALocal);


    // Agrupamos los maximos, minimos y totales en arreglos, para disminuir la cantidad de comunicaciones
    double localMin[2] = {minALocal, minBLocal};
    double localMax[2] = {maxALocal, maxBLocal};
    double localTotal[2] = {totalALocal, totalBLocal};

    double globalMin[2];
    double globalMax[2];
    double globalTotal[2];

    MPI_Allreduce(localMin, globalMin, 2, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(localMax, globalMax, 2, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(localTotal, globalTotal, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    minA = globalMin[0];
    minB = globalMin[1];
    maxA = globalMax[0];
    maxB = globalMax[1];
    promA = globalTotal[0] / NxN;
    promB = globalTotal[1] / NxN;

    escalar = (maxA * maxB - minA * minB) / (promA * promB);

    /*  printf("Proceso %d - maxA: %f - minA: %f - promA: %f \n", RANGO, maxA, minA, promA);
        printf("Proceso %d - maxB: %f - minB: %f - promB: %f \n", RANGO, maxB, minB, promB);
        printf("Proceso %d - escalar: %f \n", RANGO, escalar);*/


    // ======= Parte 2: Armar B transpuesta ========

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            BT[j * N + i] = B[i * N + j];
        }
    }

    // ======= Parte 3: Multiplicar A x B =======

    for (i = 0; i < ROWS_PER_PROCESS; i += BLOCK_SIZE)
    {
        int ixn = i * N;

        for (j = 0; j < N; j += BLOCK_SIZE)
        {
            int jxn = j * N;
            for (k = 0; k < N; k += BLOCK_SIZE)
            {
                blkmul(&stripA[ixn + k], &B[jxn + k], &stripAxB[ixn + j], N, BLOCK_SIZE);
            }
        }
    }

       // ======= Parte 4: Multiplicar AxB por el escalar ========

    for (i = 0; i < ROWS_PER_PROCESS * N; i++)
    {
        stripAxB[i] = stripAxB[i] * escalar;
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            BT[j * N + i] = B[i * N + j];
        }
    }

    // ======= Parte 5: Multiplicar C x BT =======
    for (i = 0; i < ROWS_PER_PROCESS; i += BLOCK_SIZE)
    {
        int ixn = i * N;
        for (j = 0; j < N; j += BLOCK_SIZE)
        {
            int jxn = j * N;
            for (k = 0; k < N; k += BLOCK_SIZE)
            {
                blkmul(&stripC[ixn + k], &BT[jxn + k], &stripCxBT[ixn + j], N, BLOCK_SIZE);
            }
        }
    }

    // ======= Parte 6: Multiplicar AxB(x escalar) x CxBT =======
    for (i = 0; i <= ROWS_PER_PROCESS * N; i++)
    {
        stripR[i] = stripAxB[i] + stripCxBT[i];
    }

    MPI_Gather(stripR, ROWS_PER_PROCESS * N, MPI_DOUBLE, R, ROWS_PER_PROCESS * N, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    if (RANGO == COORDINADOR) {
    
        endTime = MPI_Wtime();

        double resolution = MPI_Wtick();
        printf("Tiempo transcurrido: %f\n", (endTime - startTime));
        printf("Tiempo total: %f\n", resolution);

    }
    if (RANGO == COORDINADOR)
    {
        double promedio = 0;
        for (i = 0; i < NxN; i++)
        {
            promedio += R[i];
        }
        promedio = promedio / NxN;

        printf("\nPromedio %f\n", promedio);
    }

    MPI_Finalize();
}
