#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=4


#SBATCH -o $RESULT_DIR/output.txt
#SBATCH -e $RESULT_DIR/errors.txt 


# Preparación de directorios
# Nombre de la carpeta de resultados (cambiable)
RESULT_DIR="resultados_4"
ERROR_DIR="error"

# Preparación de directorios
mkdir -p compilados "$RESULT_DIR"
mkdir -p "$RESULT_DIR/$ERROR_DIR"


# Compilación y ejecución

# blocking
mpicc blocking-ring.c -o compilados/blocking-ring 
echo "🔄 Ejecutando blocking-ring..."
output_file="$RESULT_DIR/output_blocking-ring.txt"
error_file="$RESULT_DIR/$ERROR_DIR/errors_blocking-ring.txt"
mpirun compilados/blocking-ring > "$output_file" 2> "$error_file"
echo "✅ Fin blocking-ring"

# non-blocking
echo "🔄 Ejecutando non-blocking-ring..."
mpicc non-blocking-ring.c -o compilados/non-blocking-ring 
output_file="$RESULT_DIR/output_non-blocking-ring.txt"
error_file="$RESULT_DIR/$ERROR_DIR/errors_non-blocking-ring.txt"
mpirun compilados/non-blocking-ring > "$output_file" 2> "$error_file"
echo "✅ Fin non-blocking-ring"