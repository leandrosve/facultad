/*
** Sending simple, point-to-point messages.
** Now including a delay and a timer.
*/

/*

2. Los códigos blocking.c y non-blocking.c siguen el patrón master-worker, donde los procesos
worker le envían un mensaje de texto al master empleando operaciones de comunicación
bloqueantes y no bloqueantes, respectivamente.
- Compile y ejecute ambos códigos usando P={4,8,16} (no importa que el número de
núcleos sea menor que la cantidad de procesos). ¿Cuál de los dos retorna antes el
control?
- En el caso de la versión no bloqueante, ¿qué sucede si se elimina la operación
MPI_Wait() (línea 52)? ¿Se imprimen correctamente los mensajes enviados? ¿Por qué?
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi.h" 

#define MASTER 0

int main(int argc, char* argv[])
{
  int rank;
  int size;
  int dest;              /* destination rank for message */
  int source;            /* source rank of a message */
  int tag = 0;           /* scope for adding extra information to a message */
  MPI_Status status;     /* struct used by MPI_Recv */
  char message[BUFSIZ];
  int delay = 2;         /* a delay (s) on the senders aimed at stalling the receive */
  double tic, toc;       /* for timing: interval end points (measured in seconds) */
  double resolution;     /* for timing: resolution of timing in seconds */

  MPI_Init( &argc, &argv );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );

  if (rank != MASTER) {
    sleep(rank*delay);
    sprintf(message, "Hola Mundo! Soy el proceso %d", rank);
    dest = MASTER;
    MPI_Send(message,strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  }
  else {
    tic = MPI_Wtime();
    for (source=1; source<size; source++) {
      toc = MPI_Wtime();
      printf("Tiempo transcurrido %f (s):\tproceso %d, llamando a MPI_Recv() [bloqueante] (fuente rank %d)\n",
	     (toc - tic), rank, source);
      MPI_Recv(message, BUFSIZ, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
      toc = MPI_Wtime();
      printf("Tiempo transcurrido %f (s):\tproceso %d, MPI_Recv() devolvio control con mensaje: %s\n",
	     (toc - tic), rank, message);
    }
    toc = MPI_Wtime();
    resolution = toc - tic;
    printf("\nTiempo total = %f (s)\n", resolution);
  }



  MPI_Finalize();
  return EXIT_SUCCESS;
}