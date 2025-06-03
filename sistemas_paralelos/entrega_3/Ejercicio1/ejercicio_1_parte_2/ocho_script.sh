#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=4


#SBATCH -o $RESULT_DIR/output.txt
#SBATCH -e $RESULT_DIR/errors.txt 


# Preparación de directorios base
BASE_DIR="resultados_ocho"
mkdir -p compilados

# Archivos fuente
SRC_BLOCKING="blocking-ring.c"
SRC_NON_BLOCKING="non-blocking-ring.c"

# Ejecutables
EXEC_BLOCKING=$(basename "$SRC_BLOCKING" .c)
EXEC_NON_BLOCKING=$(basename "$SRC_NON_BLOCKING" .c)

# Crear estructura de directorios para cada ejecutable

mkdir -p "$BASE_DIR"/{$EXEC_BLOCKING,$EXEC_NON_BLOCKING}/{success,error}


# Compilación
mpicc "$SRC_BLOCKING" -lm -O3 -o "compilados/$EXEC_BLOCKING" || exit 1
mpicc "$SRC_NON_BLOCKING" -lm -O3 -o "compilados/$EXEC_NON_BLOCKING" || exit 1

# Ejecución con distintos N
for N in 10000000 20000000 40000000; do
    echo "🔄 Ejecutando $EXEC_BLOCKING... N=$N"
    mpirun "compilados/$EXEC_BLOCKING" "$N" > "$BASE_DIR/$EXEC_BLOCKING/success/output_${EXEC_BLOCKING}_${N}.txt" \
                                           2> "$BASE_DIR/$EXEC_BLOCKING/error/errors_${EXEC_BLOCKING}_${N}.txt"
    echo "✅ Fin $EXEC_BLOCKING N=$N"

    echo "🔄 Ejecutando $EXEC_NON_BLOCKING... N=$N"
    mpirun "compilados/$EXEC_NON_BLOCKING" "$N" > "$BASE_DIR/$EXEC_NON_BLOCKING/success/output_${EXEC_NON_BLOCKING}_${N}.txt" \
                                             2> "$BASE_DIR/$EXEC_NON_BLOCKING/error/errors_${EXEC_NON_BLOCKING}_${N}.txt"
    echo "✅ Fin $EXEC_NON_BLOCKING N=$N"
done
