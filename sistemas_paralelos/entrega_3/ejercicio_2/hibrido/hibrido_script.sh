#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=1
#SBATCH -o resultados/output_hibrido.txt
#SBATCH -e resultados/errors_hibrido.txt 

# Configuración de parámetros
CANT_NODOS=2  # Tiene que ser igual al SBATCH -N
CANT_PROCESOS=$((CANT_NODOS))  # 1 procesos por nodo
CANT_THREADS=$((CANT_NODOS * 8))  # 8 procesos por nodo

export OMP_NUM_THREADS=CANT_THREADS

MATRIX_SIZES=(512 1024 2048 4096)
BLOCK_SIZE=64

REPORTE_TABLA="resultados/hibrido_${CANT_PROCESOS}.txt"

# Preparación de directorios
mkdir -p compilados resultados

# Compilación
echo "🔨 Compilando hibrido.c..."
mpicc -fopenmp hibrido.c -o ./compilados/hibrido -O3 || exit 1

# Escribir encabezado
echo -e "TAMAÑO | BLOQUE | NODOS | PROCESOS  | THREADS | TIEMPO (s)  | COMM (s) | PROMEDIO\n------------------------------------------------------------------------------" > "$REPORTE_TABLA"

# Ejecución de pruebas
for N in "${MATRIX_SIZES[@]}"; do       
    OUTPUT=$(mpirun --bind-to none ./compilados/hibrido "$N" "$BLOCK_SIZE" "$CANT_THREADS")

    TIME_TOTAL=$(echo "$OUTPUT" | grep "Tiempo total" | awk '{print $3}')
    TIME_COMM=$(echo "$OUTPUT" | grep "Tiempo comunicacion" | awk '{print $3}')
    PROMEDIO=$(echo "$OUTPUT" | grep "Promedio" | awk '{print $2}')

    # Imprimir resultados alineados
    printf "%-7s | %-6s | %-5s | %-8s | %-9s | %-11s | %-9s | %-8s\n" \
        "$N" "$BLOCK_SIZE" "$CANT_NODOS" "$CANT_PROCESOS" "$CANT_THREADS" "$TIME_TOTAL" "$TIME_COMM" "$PROMEDIO" >> "$REPORTE_TABLA"

    echo "✅ Tamaño $N completado (Tiempo: $TIME_TOTAL s)"
done

echo ""
echo "✅ Ejecución completada. Resultados en: $REPORTE_TABLA"
