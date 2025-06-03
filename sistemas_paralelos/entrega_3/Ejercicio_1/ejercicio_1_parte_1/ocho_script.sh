#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=4


#SBATCH -lm -O3 -o $RESULT_DIR/output.txt
#SBATCH -e $RESULT_DIR/errors.txt 




# Preparación de directorios
# Nombre de la carpeta de resultados (cambiable)
RESULT_DIR="resultados_8_v2"

# Preparación de directorios
mkdir -p compilados "$RESULT_DIR"
mkdir -p "$RESULT_DIR/errores_8"




# Compilación y ejecución

# blocking
mpicc blocking.c -lm -O3 -o compilados/blocking 
echo "🔄 Ejecutando blocking-8..."
output_file="$RESULT_DIR/output_blocking_8.txt"
error_file="$RESULT_DIR/errores_8/errors_blocking_8.txt"
mpirun compilados/blocking > "$output_file" 2> "$error_file"
echo "✅ Fin blocking-8"

# non-blocking
echo "🔄 Ejecutando non-blocking-8..."
mpicc non-blocking.c -lm -O3 -o compilados/non-blocking 
output_file="$RESULT_DIR/output_non_blocking_8.txt"
error_file="$RESULT_DIR/errores_8/errors_non_blocking_8.txt"
mpirun compilados/non-blocking > "$output_file" 2> "$error_file"
echo "✅ Fin non-blocking-8"

# non-blocking-no-wait
echo "🔄 Ejecutando non-blocking-no-wait..."
mpicc non-blocking-no-wait.c -lm -O3 -o compilados/non-blocking-no-wait
output_file="$RESULT_DIR/output_non_blocking_no_wait_8.txt"
error_file="$RESULT_DIR/errores_8/errors_non_blocking_no_wait_8.txt"
mpirun compilados/non-blocking-no-wait > "$output_file" 2> "$error_file"
echo "✅ Fin blocking-no-wait"