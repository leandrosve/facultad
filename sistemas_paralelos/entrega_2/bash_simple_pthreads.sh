#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o resultados/output.txt
#SBATCH -e resultados/errors.txt 

MATRIX_SIZES=(512 1024 2048 4096)
BLOCK_SIZES=(64)
THREADS=(2 4 8)


#---------------

# Preparación de directorios
mkdir -p compilados resultados

# Compilación
echo "🔨 Compilando matrices_bloque_pthreads.c..."
gcc -pthread -o compilados/matrices_bloque_pthreads matrices_bloque_pthreads.c -lm -O3  || exit 1



# Ejecución de pruebas
for N in "${MATRIX_SIZES[@]}"; do
    for BLOCK in "${BLOCK_SIZES[@]}"; do
        if (( N % BLOCK != 0 )); then
            echo "⚠️  Saltando N=$N con BLOCK=$BLOCK (no es divisible)"
            continue
        fi
        
        for T in "${THREADS[@]}"; do
            echo "🔄 Ejecutando N=$N, BLOCK=$BLOCK, THREADS=$T..."
            

            ./compilados/matrices_bloque_pthreads $N $BLOCK $T

            echo "--------------------------------------"
            
            echo "✅ Listo (Tiempo: $TIME s)"
        done
    done
done


echo "✅ Ejecución completada. Resultados guardados"
