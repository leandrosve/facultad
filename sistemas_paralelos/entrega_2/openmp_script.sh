#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o directorioSalida/output.txt
#SBATCH -e directorioSalida/errors.txt 

# Configuración de parámetros
MATRIX_SIZES=(512 1024 2048 4096)
BLOCK_SIZES=(64)
THREADS=(2 4 8)
REPORTE_TABLA="resultados/completo_openmp_v2.txt"

# Preparación de directorios
mkdir -p compilados resultados

# Compilación
echo "🔨 Compilando completo_openmp_v2.c..."
gcc -o compilados/completo_openmp_v2 completo_openmp_v2.c -lm -O3 -fopenmp || exit 1

# ✅ Escribir encabezado una vez
echo -e "TAMAÑO | BLOQUE | HILOS | TIEMPO (s)\n------------------------------------" > "$REPORTE_TABLA"

# Ejecución de pruebas
for N in "${MATRIX_SIZES[@]}"; do
    for BLOCK in "${BLOCK_SIZES[@]}"; do
        if (( N % BLOCK != 0 )); then
            echo "⚠️  Saltando N=$N con BLOCK=$BLOCK (no es divisible)"
            continue
        fi
        
        for T in "${THREADS[@]}"; do
            echo "🔄 Ejecutando N=$N, BLOCK=$BLOCK, THREADS=$T..."
            
            OUTPUT=$(./compilados/completo_openmp_v2 "$N" "$BLOCK" "$T")
            TIME=$(echo "$OUTPUT" | grep "Tiempo en segundos" | awk '{print $4}')
            
            # Agregar línea al archivo con alineación de columnas
            printf "%-7s | %-6s | %-5s | %-10s\n" "$N" "$BLOCK" "$T" "$TIME" >> "$REPORTE_TABLA"
            
            echo "✅ Listo (Tiempo: $TIME s)"
        done
    done
done

echo ""
echo "✅ Ejecución completada. Resultados guardados en: $REPORTE_TABLA"
