#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=8


#SBATCH -o resultados_16/output.txt
#SBATCH -e resultados_16/errors.txt 


# Preparación de directorios
mkdir -p compilados resultados_16 resultados_16/errores_16

# Compilación y ejecución

# blocking
mpicc blocking.c -lm -O3 -o compilados/blocking 
echo "🔄 Ejecutando blocking..."
output_file="resultados_16/output_blocking_16.txt"
error_file="resultados_16/errores_16/errors_blocking_16.txt"
mpirun compilados/blocking > "$output_file" 2> "$error_file"
echo "✅ Fin blocking"

# non-blocking
echo "🔄 Ejecutando non-blocking..."
mpicc non-blocking.c -lm -O3 -o compilados/non-blocking 
output_file="resultados_16/output_non_blocking_16.txt"
error_file="resultados_16/errores_16/errors_non_blocking_16.txt"
mpirun compilados/non-blocking > "$output_file" 2> "$error_file"
echo "✅ Fin non-blocking"

# non-blocking-no-wait
echo "🔄 Ejecutando non-blocking-no-wait..."
mpicc non-blocking-no-wait.c -lm -O3 -o compilados/non-blocking-no-wait
output_file="resultados_16/output_non_blocking_no_wait_16.txt"
error_file="resultados_16/errores_16/errors_non_blocking_no_wait_16.txt"
mpirun compilados/non-blocking-no-wait > "$output_file" 2> "$error_file"
echo "✅ Fin blocking-no-wait"