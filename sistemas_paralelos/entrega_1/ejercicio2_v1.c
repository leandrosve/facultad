#include<stdio.h>
#include<stdlib.h>
#define ORDENXFILAS 0
#define ORDENXCOLUMNAS 1

//Dimension por defecto de las matrices
int N=1000;

int enablePrint = 0;

//Para calcular tiempo
double dwalltime(){
    double sec;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}

void printMatrix(double *matrix, int N, int byRow,const char *title) {
    if (enablePrint <= 0) return;
    printf("%s\n", title);  
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int index = byRow ? (i * N + j) : (j * N + i);
            printf("%f ", matrix[index]);
        }
        printf("\n");
    }
}

int main(int argc,char*argv[]){
    int NxN = N * N;

    double *A,*B,*C,*R,*AxB,*CxBT,*BT;

    //Aloca memoria para las matrices
    A=(double*)malloc(sizeof(double)*N*N);
    B=(double*)malloc(sizeof(double)*N*N);
    C=(double*)malloc(sizeof(double)*N*N);
    R=(double*)malloc(sizeof(double)*N*N);

    AxB=(double*)malloc(sizeof(double)*N*N);
    BT=(double*)malloc(sizeof(double)*N*N);
    CxBT=(double*)malloc(sizeof(double)*N*N);


    int i,j,k;
    //Inicializa las matrices A y B en 1, el resultado sera una matriz con todos sus valores en N
    for(i=0;i<NxN;i++){
        A[i]= 1;
        B[i]= 1;
        C[i]= 1;
    }

    printMatrix(A, N, 1, "Matriz A");
    printMatrix(B, N, 0, "Matriz B");

    // Comienza el calculo

    double timetick = dwalltime();

    double maxA = -1; 
    double maxB = -1;
    double minA = 99999;
    double minB = 99999;
    double promA = 0;
    double promB = 0;

    for(i=0;i<NxN;i++){
        double aux = A[i];
        if (aux > maxA) {
            maxA = aux;
        }
        if (aux < minA) {
            minA = aux;
        } 
        promA += aux;
    }
    promA = promA / NxN;

    for(i=0;i<NxN;i++){
        double aux = B[i];
        if (aux > maxB) {
            maxB = aux;
        }
        if (aux < minB){
            minB = aux;
        } 
        promB+=aux;
    }

    promB = promB / NxN;

    double escalar = maxA * maxB - minA * minB;

    
    printf("===================================\n");
    printf("promA: %f \n", promA);
    printf("promB: %f \n", promB);
    printf("minA: %f \n", minA);
    printf("minB: %f \n", minB);
    printf("escalar: %f \n", escalar);
    printf("===================================\n");
    
    // A x B (x el escalar)
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            int acc = 0;
            for(k=0;k<N;k++){
                acc += A[i*N+k] * B[j*N+k];
            }
            AxB[i * N + j] = acc * escalar; 
        }
    }

    printMatrix(AxB, N, 1, "A x B x Escalar");


    // ARMAR B TRANSPUESTA ANTES DE USARLA

    for(i=0; i<N;i++){
        for(j=0;j<N;j++){
            BT[j*N + i] = B[i * N + j];
        }
    }

    // C * B transversa
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            int acc = 0;
            for(k=0;k<N;k++){
                acc += C[i*N+k] * BT[j*N+k];
            }
            CxBT[i * N + j] = acc; 
        }
    }

    // matriz A multiplicado por identidad da A ==> para probar
    
    printMatrix(CxBT, N, 1, "C x B Transversa");

    // Multiplicacion final
    for(i=0;i<N;i++){
        for(j=0;j<N;j++){
            int acc = 0;
            for(k=0;k<N;k++){
                acc += AxB[i*N+k] * CxBT[k*N+j];
            }
            R[i * N + j] = acc; 
        }
    }

    double elapsed = dwalltime() - timetick;
    printMatrix(R, N, 1, "RESULTADO");

    printf("Tiempo en segundos %f\n", elapsed );

}