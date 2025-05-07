#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define ORDENXFILAS 0
#define ORDENXCOLUMNAS 1

void *thread_process(void *ptr);

pthread_mutex_t minA_lock;
pthread_mutex_t minB_lock;
pthread_mutex_t maxA_lock;
pthread_mutex_t maxB_lock;
pthread_mutex_t promA_lock;
pthread_mutex_t promB_lock;

pthread_mutex_t count_axb_lock;
pthread_mutex_t count_bt_lock;

pthread_mutex_t mutexCalc;


pthread_barrier_t barrier;

sem_t bt_sem;
sem_t axb_sem;

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
double count_axb = 0;

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

    if (((N % BLOCK_SIZE) != 0))
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

    // Cosas de pthread
    int ids[T];
    pthread_attr_t attr;
    pthread_t threads[T];

    int *status;

    pthread_mutex_init(&minA_lock, NULL);
    pthread_mutex_init(&minB_lock, NULL);
    pthread_mutex_init(&maxA_lock, NULL);
    pthread_mutex_init(&maxB_lock, NULL);
    pthread_mutex_init(&promA_lock, NULL);
    pthread_mutex_init(&promB_lock, NULL);
    pthread_mutex_init(&count_bt_lock, NULL);
    pthread_mutex_init(&count_axb_lock, NULL);
    pthread_barrier_init(&barrier, NULL, T);

    pthread_mutex_init(&mutexCalc, NULL);

    sem_init(&bt_sem, 0, 0);
    sem_init(&axb_sem, 0, 0);

    pthread_attr_init(&attr);

    // Comienza el calculo
    double timetick = dwalltime();

    /* Crea los hilos */
    for (i = 0; i < T; i++)
    {
        ids[i] = i;
        pthread_create(&threads[i], &attr, thread_process, &ids[i]);
    }

    /* Espera a que los hilos terminen */
    for (i = 0; i < T; i++)
    {
        pthread_join(threads[i], (void *)&status);
    }

    double elapsed = dwalltime() - timetick;

    printf("\nTiempo en segundos %f\n", elapsed);

    double promedio = 0;
    for (i = 0; i < NxN; i++)
    {
        promedio += R[i];
    }
    promedio = promedio / NxN;

    printf("\nPromedio %f\n", promedio);
}

void *thread_process(void *ptr)
{
    int *p, id, i, j, k; 
    p = (int *)ptr;
    id = *p;

    int inferior = id * PORTION_SIZE;
    int superior = inferior + PORTION_SIZE - 1;



      // ======= Parte 2: Armar B transpuesta =======
    // Lo hacemos antes asi no tenemos que usarla enseguida
    int start2 = id * ROWS_PER_THEAD;
    int end2 = start2 + ROWS_PER_THEAD - 1;

    for (i = start2; i <= end2; i++)
    {
        int ixn = i * N;
        for (j = 0; j < N; j++)
        {
            BT[j * N + i] = B[ixn + j];
        }
    }

    // Aviso que termine b transpuesta, si soy el ultimo le sumo T al semaforo
    pthread_mutex_lock(&count_bt_lock);
    count_bt++;
    if (count_bt == T)
    {
        for (i = 0; i < T; i++)
        {
            sem_post(&bt_sem);
        }
    }
    pthread_mutex_unlock(&count_bt_lock);

    // ======= Parte 1: Calculo del escalar =======

   // printf("\nHilo: %d: [%d, %d]\n", id, inferior, superior);
    double maxALocal = A[inferior];
    double maxBLocal = B[inferior];
    double minALocal = A[inferior];
    double minBLocal = B[inferior];

    double promALocal = 0, promBLocal = 0;

    for (i = inferior; i <= superior; i++)
    {
        double aux = A[i];
        if (aux > maxALocal)
        {
            maxALocal = aux;
        }
        if (aux < minALocal)
        {
            minALocal = aux;
        }
        promALocal += aux;
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
        promBLocal += aux;
    }

    // Escribir en las variables globales
    pthread_mutex_lock(&mutexCalc);
    if (maxALocal > maxA)
        maxA = maxALocal;
    if (maxBLocal > maxB)
        maxB = maxBLocal;

    if (minALocal < minA)
        minA = minALocal;
    
    if (minBLocal < minB)
        minB = minBLocal;
    
    promA += promALocal;

    promB += promBLocal;
    count_escalar += 1;
    
    // Si soy el ultimo en llegar hasta aca, calculo el promedio y luego el escalar
    if (count_escalar == T)
    {
        promB = promB / NxN;
        promA = promA / NxN;

        escalar = (maxA * maxB - minA * minB) / (promA * promB);

        // printf("Resultados:\n");
        // printf("minA: %f  maxA: %f  promA: %f \n", minA, maxA, promA);
        // printf("minB: %f  maxB: %f  promB: %f \n", minB, maxB, promB);
        // printf("Escalar %f \n", escalar);
    }
    pthread_mutex_unlock(&mutexCalc);

    // Por ahora no hay que esperar

    // ======= Parte 3: Multiplicar A x B =======


    int start = id * BLOCKS_PER_THREAD * BLOCK_SIZE;
    int end = start + BLOCKS_PER_THREAD * BLOCK_SIZE;

    for (i = start; i < end; i += BLOCK_SIZE)
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

    // Aviso que termine AxB, si soy el ultimo le sumo T al semaforo
    pthread_mutex_lock(&count_axb_lock);
    count_axb++;
    if (count_axb == T)
    {
        for (i = 0; i < T; i++)
        {
            sem_post(&axb_sem);
        }
    }
    pthread_mutex_unlock(&count_axb_lock);


    // Garantizar que B Transpuesta esta calculada
    sem_wait(&bt_sem);

    // ======= Parte 4: Multiplicar C x BT =======
    for (i = start; i < end; i += BLOCK_SIZE)
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

    // ======= Parte 5: Multiplicar AxB por el escalar =======

    // Garantizar que AxB (y el escalar) esta calculado
    sem_wait(&axb_sem);

    for (i = inferior; i <= superior; i++)
    {
        AxB[i] = AxB[i] * escalar;
    }

    // ======= Parte 5: Multiplicar AxB(x escalar) x CxBT =======

    // Garantizar que todos llegaron hasta aqui
    pthread_barrier_wait(&barrier);

    for (i = inferior; i <= superior; i++)
    {
        R[i] = AxB[i] + CxBT[i];
    }

    pthread_exit((void *)ptr);
}