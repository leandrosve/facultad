#include <stdio.h>
#include <stdlib.h>

// Dimension por defecto de las matrices
int N = 100;
int BLOCK_SIZE = 10;


/*****************************************************************/

/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs)
{
  int i, j, k;    /* Guess what... again... */

  for (i = 0; i < bs; i++)
  {
    for (j = 0; j < bs; j++)
    {
      for  (k = 0; k < bs; k++)
      {
        cblk[i*n + j] += ablk[i*n + k] * bblk[j*n + k];
      }
    }
  }
}
/* Multiply square matrices, blocked version */
void matmulblks(double *a, double *b, double *c, int n, int bs)
{
  int i, j, k;    /* Guess what... */

  for (i = 0; i < n; i += bs)
  {
    for (j = 0; j < n; j += bs)
    {
      for  (k = 0; k < n; k += bs)
      {
        blkmul(&a[i*n + k], &b[j*n + k], &c[i*n + j], n, bs);
      }
    }
  }
}




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

    printf("\nAlgoritmo v2 - Ejecución con N = %d y BLOCK_SIZE = %d\n", N, BLOCK_SIZE);

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

    // Comienza el calculo
    double timetick = dwalltime();

    double maxA = A[0];
    double maxB = B[0];
    double minA = A[0];
    double minB = B[0];
    double promA = 0;
    double promB = 0;

    // Auxiliares para calcular los indices
    int posA, posB, posAxB, posC, posBT, posCxBT;

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
    
    matmulblks(A, B, C, N, BLOCK_SIZE);
    

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

    // C x B Transpuesta (Multiplicacion por Bloques)

    matmulblks(A, BT, C, N, BLOCK_SIZE);

    // AxB(x escalar) + CxBT
    // Ambas matrices estan almacenadas por fila
    for (i = 0; i < NxN; i++)
    {
        R[i] = AxB[i] + CxBT[i];
    }

    double elapsed = dwalltime() - timetick;

    printf("\nTiempo en segundos %f\n", elapsed);
}