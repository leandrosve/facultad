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
        printf("\nUsar: %s n\n  dimension cant_hilos\n", argv[0]);
        exit(1);
    }

    printf("\nEjecución con n = %d \n", N);

    int NxN = N * N;

    double *A, *B,*AxB;

    // Alocamos memoria para las matrices
    A = (double *)malloc(sizeof(double) * N * N);
    B = (double *)malloc(sizeof(double) * N * N);
    AxB = (double *)malloc(sizeof(double) * N * N);

    int i, j, k;
    // Inicializa la matrices A en 7 y B como matriz identidad
    for (i = 0; i < NxN; i++)
    {
        A[i] = 7;
        B[i] = 0;
    }

    for (i = 0; i < N; i++)
    {
        B[i * N + i] = 1;
    }
    // Comienza el calculo
    double timetick = dwalltime();

    int ixn, jxn;
    // A x B
    for (i = 0; i < N; i++)
    {
        ixn = i *N;
        for (j = 0; j < N; j++)
        {
            jxn = j * N;
            double acc = 0;
            for (k = 0; k < N; k++)
            {
                acc += A[ixn + k] * B[jxn + k];
            }
            AxB[ixn + j] = acc;
        }
    }

    double elapsed = dwalltime() - timetick;

    printf("\nTiempo en segundos %f\n", elapsed);

    //Verifica el resultado
    for(i=0;i<NxN;i++){
        if(AxB[i]!=7) {
            printf("Suma de vectores resultado erroneo, i: %d  valor: %f\n", i, AxB[i]);
            exit(0);
        }
    }

    printf("Suma de vectores resultado correcto\n");
}