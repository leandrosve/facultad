#include <stdio.h>
#include <stdlib.h>

#define ORDENXFILAS 0
#define ORDENXCOLUMNAS 1

// Version sin impresiones
// Dimension por defecto de las matrices
int N = 1000;

// Para calcular tiempo
double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec + tv.tv_usec / 1000000.0;
    return sec;
}

int main(int argc, char *argv[])
{

    // Recibimos parametro con la dimension de la matriz
    if ((argc != 2) || ((N = atoi(argv[1])) <= 0))
    {
        printf("\nUsar: %s n\n  n: Dimension de la matriz (nxn X nxn)\n", argv[0]);
        exit(1);
    }

    printf("\nAlgoritmo v1 - Ejecución con n = %d \n", N);

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

    int i, j, k;

    // Inicializa las matrices A y B en 1
    for (i = 0; i < NxN; i++)
    {
        A[i] = 1;
        B[i] = 1;
        C[i] = 1;
    }

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

    // A x B
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            double acc = 0;
            for (k = 0; k < N; k++)
            {
                acc += A[i * N + k] * B[j * N + k];
            }
            AxB[i * N + j] = acc;
        }
    }

    // Multiplicamos AxB por el escalar calculado anteriormente
    for (i = 0; i < NxN; i++)
    {
        AxB[i] = AxB[i] * escalar;
    }

    // Armamos B transpuesta

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            BT[j * N + i] = B[i * N + j];
        }
    }

    // C * B transpuesta
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            double acc = 0;
            for (k = 0; k < N; k++)
            {
                acc += C[i * N + k] * BT[j * N + k];
            }
            CxBT[i * N + j] = acc;
        }
    }

    // AxB(x escalar) + CxBT
    // Ambas matrices estan almacenadas por fila
    for (i = 0; i < NxN; i++)
    {
        R[i] = AxB[i] + CxBT[i];
    }

    double elapsed = dwalltime() - timetick;

    printf("\nTiempo en segundos %f\n", elapsed);
}