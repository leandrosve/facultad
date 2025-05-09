#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

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

/* Multiply square matrices, blocked version */
void matmulblks(double *a, double *b, double *c, int n, int bs)
{
  int i, j, k; /* Guess what... */

  for (i = 0; i < n; i += bs)
  {
    for (j = 0; j < n; j += bs)
    {
      for (k = 0; k < n; k += bs)
      {
        blkmul(&a[i * n + k], &b[j * n + k], &c[i * n + j], n, bs);
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

// Dimension por defecto de las matrices
int N = 256;
int NxN;

// Especifico para la multiplicacion por bloque
int BLOCK_SIZE;
int BLOCKS_PER_THREAD;

// Como se dividen las matrices entre los threads
int PORTION_SIZE, ROWS_PER_THEAD;

// Matrices
double *A, *B, *C, *R, *AxB, *CxBT, *BT;

// Maximos, minimos, promedios
double maxA, maxB, minA, minB;
double promA = 0.0, promB = 0.0;
double escalar = 0.0;

double count_escalar = 0;
double count_bt = 0;
double count_cxbt = 0;
double count_axb = 0;

int i, j, k;

// Cantidad de hilos
int T;

int main(int argc, char *argv[])
{

  // Controlar los argumentos del programa

  if ((argc != 4) || ((N = atoi(argv[1])) <= 0) || ((BLOCK_SIZE = atoi(argv[2])) <= 0) || ((T = atoi(argv[3])) <= 0))
  {
    printf("\nUsar: %s N tamaño_bloque cant_hilos\n", argv[0]);
    exit(1);
  }

  if ((N % BLOCK_SIZE) != 0)
  {
    printf("\nError: N debe ser multiplo de BLOCK_SIZE \n");
    exit(1);
  }

  if (N % T != 0)
  {
    printf("\nLa dimension de la matriz debe ser divisible por la cantidad de hilos\n");
    exit(1);
  }

  if ((N / BLOCK_SIZE % T) != 0)
  {
    printf("\nLa cantidad de hilos debe ser consistente con la cantidad total de bloques\n");
    exit(1);
  }

  // Memorizar tamaño del vector
  NxN = N * N;

  BLOCKS_PER_THREAD = N / BLOCK_SIZE / T;
  PORTION_SIZE = NxN / T;
  ROWS_PER_THEAD = N / T;

  printf("\nCantidad de bloques por hilo: %d\n", BLOCK_SIZE);
  printf("\nPorcion de matriz por hilo: %d\n", BLOCK_SIZE);

  // Alocacion de memoria para las matrices
  A = (double *)malloc(sizeof(double) * NxN);
  B = (double *)malloc(sizeof(double) * NxN);
  C = (double *)malloc(sizeof(double) * NxN);
  // Resultado Final
  R = (double *)malloc(sizeof(double) * NxN);
  // Matrices para resultados intermedios
  AxB = (double *)malloc(sizeof(double) * NxN);
  BT = (double *)malloc(sizeof(double) * NxN);
  CxBT = (double *)malloc(sizeof(double) * NxN);

  // Inicializacion de las matrices
  int i, j, k;
  for (i = 0; i < NxN; i++)
  {
    A[i] = (i % 2) + 1;
    B[i] = (i % 4) + 1;
    C[i] = 1;
    AxB[i] = 0;
    CxBT[i] = 0;
    R[i] = 0;
  }

  // Inicializacion de maximos, minimos y promedios
  minA = A[0];
  maxA = A[0];
  minB = B[0];
  maxB = B[0];
  promA = 0;
  promB = 0;

  // Cosas de openMP
  omp_set_num_threads(T);

  // Comienza el calculo
  double timetick = dwalltime();

  int tid;
  #pragma omp parallel
  {
    //tid = omp_get_thread_num();
    //printf("Hilo %d", tid);

    // ======= Parte 1: Calcular min, max, y promedios =======
    #pragma omp for nowait private(i) reduction(min : minA) reduction(max : maxA) reduction(+ : promA)
    for (i = 0; i < NxN; i++)
    {
      double aux = A[i];

      if (aux < minA)
        minA = aux;
      if (aux > maxA)
        maxA = aux;

      promA += aux;
    }

    #pragma omp for nowait private(i) reduction(min : minB) reduction(max : maxB) reduction(+ : promB)
    for (i = 0; i < NxN; i++)
    {
      double aux = B[i];

      if (aux < minB)
        minB = aux;
      if (aux > maxB)
        maxB = aux;

      promB += aux;
    }

    // ======= Parte 4: Armar B Transpuesta =======

    #pragma omp for nowait private(i)
    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        BT[j * N + i] = B[i * N + j];
      }
    }

    // ======= Parte 2: Multiplicar A por B =======

    #pragma omp for nowait private(i)
    for (i = 0; i < N; i += BLOCK_SIZE)
    {
      int ixn = i * N;
      for (j = 0; j < N; j += BLOCK_SIZE)
      {
        int jxn = j * N;
        for (k = 0; k < N; k += BLOCK_SIZE)
        {
          blkmul(&A[ixn + k], &B[jxn + k], &AxB[ixn + j], N, BLOCK_SIZE);
        }
      }
    }
  }

  // ======= Parte 1.5: Calcular promedios y escalar =======
  // ======= Aca si o si tienen que haber terminado todos la primera parte =======
  promA = promA / NxN;
  promB = promB / NxN;

  escalar = (maxA * maxB - minA * minB) / (promA * promB);

  // ======= Parte 3: Multiplicar AB por escalar =======
  #pragma omp parallel
  {
    #pragma omp for nowait private(i)
    for (i = 0; i < NxN; i++)
    {
      AxB[i] = AxB[i] * escalar;
    }

    // ======= Parte 5: Multiplicar C por B Transpuesta =======
    #pragma omp for nowait private(i)
    for (i = 0; i < N; i += BLOCK_SIZE)
    {
      int ixn = i * N;
      for (j = 0; j < N; j += BLOCK_SIZE)
      {
        int jxn = j * N;
        for (k = 0; k < N; k += BLOCK_SIZE)
        {
          blkmul(&C[ixn + k], &BT[jxn + k], &CxBT[ixn + j], N, BLOCK_SIZE);
        }
      }
    }

    #pragma omp barrier
    // ======= Parte 6: Multiplicar AB(x escalar) por CxBT =======
    #pragma omp for private(i)
    for (i = 0; i < NxN; i++)
    {
      R[i] = AxB[i] + CxBT[i];
    }
  }

  double elapsed = dwalltime() - timetick;

  printf("\nTiempo en segundos %f\n", elapsed);

  printf("Resultados:\n");
  printf("minA: %f  maxA: %f  promA: %f \n", minA, maxA, promA);
  printf("minB: %f  maxB: %f  promB: %f \n", minB, maxB, promB);
  printf("Escalar %f \n", escalar);

  double promedio = 0;
  for (i = 0; i < NxN; i++)
  {
    promedio += R[i];
  }
  promedio = promedio / NxN;

  printf("\nPromedio %f\n", promedio);
}