#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "mpi.h"
#include <omp.h>

#define COORDINADOR 0

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

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

    int provided;
    int RANGO;
    int CANT_PROCESOS;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &CANT_PROCESOS);
    MPI_Comm_rank(MPI_COMM_WORLD, &RANGO);
    MPI_Status status;

    int N, BLOCK_SIZE, CANT_THREADS;

    // Controlar los argumentos del programa

    if ((argc != 4) || ((N = atoi(argv[1])) <= 0) || ((BLOCK_SIZE = atoi(argv[2])) <= 0) || ((CANT_THREADS = atoi(argv[3])) <= 0))
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

    if (((N / CANT_PROCESOS) % CANT_THREADS) != 0)
    {
        printf("\nLa cantidad de hilos debe ser consistente con el tamaño de los strips\n");
        exit(1);
    }

    if (RANGO == COORDINADOR)
    {
        printf("==============================================================\n");
        printf("N = %d  CANT_PROCESOS = %d CANT_HILOS = %d  BLOCK_SIZE = %d\n\n", N, CANT_PROCESOS, CANT_THREADS, BLOCK_SIZE);
    }

    if (N / (CANT_PROCESOS * CANT_THREADS) < BLOCK_SIZE) {
        BLOCK_SIZE =  N / (CANT_PROCESOS * CANT_THREADS);

        if (RANGO == COORDINADOR) {
            printf("El tamaño de bloque se cambio a %d para evitar inconsistencias\n", BLOCK_SIZE);
        }
    }

    // OpenMP
    omp_set_num_threads(CANT_THREADS);

    // Matrices enteras y strips
    double *A, *B, *BT, *C, *R;
    double *stripA, *stripAxB, *stripC, *stripCxBT, *stripR;

    // Minimos y maximos globales
    double maxA, maxB, minA, minB, promA, promB, escalar;

    // Minimos y maximos locales
    double minALocal, maxALocal, minBLocal, maxBLocal, totalALocal, totalBLocal;

    int NxN = N * N;
    int ROWS_PER_PROCESS = N / CANT_PROCESOS;
    int ELEMENTS_PER_PROCESS = ROWS_PER_PROCESS * N;

    // Iteradores
    int i, j, k, ixn, jxn;

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

    // Para medir tiempos de comunicacion
    double commTimes[6], maxCommTimes[6], minCommTimes[6], commTime, totalTime;

    // Variable auxiliar para acceder a los elementos de las matrices
    double aux;

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

    // Hacemos un barrier para garantizar que los procesos llegaron hasta este punto y estan listos para comenzar
    MPI_Barrier(MPI_COMM_WORLD);

    commTimes[0] = MPI_Wtime();

    // Distribuir las filas de la matriz A
    MPI_Scatter(A, ELEMENTS_PER_PROCESS, MPI_DOUBLE, stripA, ELEMENTS_PER_PROCESS, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    // Enviar B  atodos los procesos
    MPI_Bcast(B, NxN, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    // Distribuir las filas de la matriz C
    MPI_Scatter(C, ROWS_PER_PROCESS * N, MPI_DOUBLE, stripC, ROWS_PER_PROCESS * N, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    commTimes[1] = MPI_Wtime();

    // ======= Parte 1: Calculo del escalar ========
    minALocal = stripA[0];
    maxALocal = minALocal;
    minBLocal = B[inferior];
    maxBLocal = minBLocal;
    totalBLocal = 0;

    // Inicia bloque paralelo
    #pragma omp parallel shared(BLOCK_SIZE, N, NxN, stripA, stripC, stripR, stripCxBT, BT, B, ELEMENTS_PER_PROCESS)
    {

        // printf("Proceso: %d - Hilo: %d\n", RANGO, omp_get_thread_num());

        #pragma omp for nowait private(i, aux) reduction(min : minALocal) reduction(max : maxALocal) reduction(+ : totalALocal) schedule(static)
        for (i = 0; i < ELEMENTS_PER_PROCESS; i++)
        {
            aux = stripA[i];

            if (aux < minALocal)
                minALocal = aux;
            if (aux > maxALocal)
                maxALocal = aux;

            totalALocal += aux;
        }

        #pragma omp for private(i, aux) reduction(min : minBLocal) reduction(max : maxBLocal) reduction(+ : totalBLocal) schedule(static)
        for (i = inferior; i <= superior; i++)
        {
            aux = B[i];

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

        // Solo uno debe hacer esto
        #pragma omp single
        {
            // Agrupamos los maximos, minimos y totales en arreglos, para disminuir la cantidad de comunicaciones
            // Aprovechando que Allreduce soporta arreglos
            double localMin[2] = {minALocal, minBLocal};
            double localMax[2] = {maxALocal, maxBLocal};
            double localTotal[2] = {totalALocal, totalBLocal};

            double globalMin[2];
            double globalMax[2];
            double globalTotal[2];

            commTimes[2] = MPI_Wtime();

            MPI_Allreduce(localMin, globalMin, 2, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
            MPI_Allreduce(localMax, globalMax, 2, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
            MPI_Allreduce(localTotal, globalTotal, 2, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

            commTimes[3] = MPI_Wtime();

            minA = globalMin[0];
            minB = globalMin[1];
            maxA = globalMax[0];
            maxB = globalMax[1];
            promA = globalTotal[0] / NxN;
            promB = globalTotal[1] / NxN;

            escalar = (maxA * maxB - minA * minB) / (promA * promB);
        }

        // ======= Parte 2: Armar B transpuesta ========

        #pragma omp for nowait private(i, j) schedule(static)
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                BT[j * N + i] = B[i * N + j];
            }
        }

        #pragma omp for private(i, j, k, ixn, jxn) schedule(static)
        for (i = 0; i < ROWS_PER_PROCESS; i += BLOCK_SIZE)
        {
            ixn = i * N;

            for (j = 0; j < N; j += BLOCK_SIZE)
            {
                jxn = j * N;
                for (k = 0; k < N; k += BLOCK_SIZE)
                {
                    blkmul(&stripA[ixn + k], &B[jxn + k], &stripAxB[ixn + j], N, BLOCK_SIZE);
                }
            }
        }

        // ======= Parte 4: Multiplicar AxB por el escalar ========
        #pragma omp for private(i) schedule(static)
        for (i = 0; i < ROWS_PER_PROCESS * N; i++)
        {
            stripAxB[i] = stripAxB[i] * escalar;
        }

        // ======= Parte 5: Multiplicar C x BT =======

        #pragma omp for private(i, j, k, ixn, jxn) schedule(static)
        for (i = 0; i < ROWS_PER_PROCESS; i += BLOCK_SIZE)
        {
            ixn = i * N;
            for (j = 0; j < N; j += BLOCK_SIZE)
            {
                jxn = j * N;
                for (k = 0; k < N; k += BLOCK_SIZE)
                {
                    blkmul(&stripC[ixn + k], &BT[jxn + k], &stripCxBT[ixn + j], N, BLOCK_SIZE);
                }
            }
        }

        // ======= Parte 6: Multiplicar AxB(x escalar) x CxBT =======
        #pragma omp for private(i) schedule(static)
        for (i = 0; i <= ELEMENTS_PER_PROCESS; i++)
        {
            stripR[i] = stripAxB[i] + stripCxBT[i];
        }
    }

    commTimes[4] = MPI_Wtime();

    MPI_Gather(stripR, ROWS_PER_PROCESS * N, MPI_DOUBLE, R, ROWS_PER_PROCESS * N, MPI_DOUBLE, COORDINADOR, MPI_COMM_WORLD);

    commTimes[5] = MPI_Wtime();

    MPI_Reduce(commTimes, minCommTimes, 6, MPI_DOUBLE, MPI_MIN, COORDINADOR, MPI_COMM_WORLD);
    MPI_Reduce(commTimes, maxCommTimes, 6, MPI_DOUBLE, MPI_MAX, COORDINADOR, MPI_COMM_WORLD);

    if (RANGO == COORDINADOR)
    {

        totalTime = maxCommTimes[5] - minCommTimes[0];
        commTime = (maxCommTimes[1] - minCommTimes[0]) + (maxCommTimes[3] - minCommTimes[2]) + (maxCommTimes[5] - minCommTimes[4]);

        printf("Tiempo total: %lf\nTiempo comunicacion: %lf\n", totalTime, commTime);

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