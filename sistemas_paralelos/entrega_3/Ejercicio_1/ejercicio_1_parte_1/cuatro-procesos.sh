#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --tasks-per-node=4



# Preparación de directorios
mkdir -p compilados resultados_4 resultados_4/errores_4

# Compilación y ejecución

# blocking
mpicc blocking.c -lm -O3 -o compilados/blocking 
echo "🔄 Ejecutando blocking ..."
output_file="resultados_4/output_blocking_4.txt"
error_file="resultados_4/errores_4/errors_blocking_4.txt"
mpirun compilados/blocking > "$output_file" 2> "$error_file"
echo "✅ Fin blocking "

# non-blocking
echo "🔄 Ejecutando non-blocking ..."
mpicc non-blocking.c -lm -O3 -o compilados/non-blocking 
output_file="resultados_4/output_non_blocking_4.txt"
error_file="resultados_4/errores_4/errors_non_blocking_4.txt"
mpirun compilados/non-blocking > "$output_file" 2> "$error_file"
echo "✅ Fin non-blocking "

# non-blocking-no-wait
echo "🔄 Ejecutando non-blocking-no-wait..."
mpicc non-blocking-no-wait.c -lm -O3 -o compilados/non-blocking-no-wait
output_file="resultados_4/output_non_blocking_no_wait_4.txt"
error_file="resultados_4/errores_4/errors_non_blocking_no_wait_4.txt"
mpirun compilados/non-blocking-no-wait > "$output_file" 2> "$error_file"
echo "✅ Fin blocking-no-wait"