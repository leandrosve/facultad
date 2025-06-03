#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=4

# Preparación de directorios
RESULT_DIR="resultados"
ERROR_DIR="error"
FOLDER_SUCCESS="success"
mkdir -p compilados 
mkdir -p "$RESULT_DIR/$ERROR_DIR"

# Archivos fuente
SRC_BLOCKING="blocking-ring.c"
SRC_NON_BLOCKING="non-blocking-ring.c"

# Ejecutables
EXEC_BLOCKING=$(basename "$SRC_BLOCKING" .c)
EXEC_NON_BLOCKING=$(basename "$SRC_NON_BLOCKING" .c)

# Compilación
mpicc "$SRC_BLOCKING" -lm -O3 -o "compilados/$EXEC_BLOCKING" || exit 1
mpicc "$SRC_NON_BLOCKING" -lm -O3 -o "compilados/$EXEC_NON_BLOCKING" || exit 1

# Ejecución con distintos N
for N in 10000000 20000000 40000000; do
    echo "🔄 Ejecutando $EXEC_BLOCKING... N=$N"
    
    output_file="$RESULT_DIR/output_${EXEC_BLOCKING}_${N}.txt"

    error_file="$RESULT_DIR/$ERROR_DIR/errors_${EXEC_BLOCKING}_${N}.txt"

    mpirun "compilados/$EXEC_BLOCKING" "$N" > "$output_file" 2> "$error_file"
    echo "✅ Fin $EXEC_BLOCKING N=$N"

    echo "🔄 Ejecutando $EXEC_NON_BLOCKING... N=$N"
    mpirun "compilados/$EXEC_NON_BLOCKING" "$N" > "$RESULT_DIR/output_${EXEC_NON_BLOCKING}_${N}.txt" 2> "$RESULT_DIR/$ERROR_DIR/errors_${EXEC_NON_BLOCKING}_${N}.txt"
    echo "✅ Fin $EXEC_NON_BLOCKING N=$N"
done
