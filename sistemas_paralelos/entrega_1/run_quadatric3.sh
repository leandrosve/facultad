#!/bin/bash

# Configuración
VALORES_TIMES=(128 256 512 1024 2048 4096)
REPORTE="resultados/quadatric3/reporte_quadatric3.txt"
DIR_COMPILADOS="compilados"
DIR_RESULTADOS="resultados/quadatric3"

# Crear directorios (si no existen)
mkdir -p "$DIR_COMPILADOS" "$DIR_RESULTADOS"

# Compilar (solo una vez)
echo "🔧 Compilando quadatric3.c con flags -lm -O3..."
gcc -o "$DIR_COMPILADOS/quadatric3" quadatric3.c -lm -O3 || { echo "❌ Error al compilar"; exit 1; }

# Encabezado del reporte
{
    echo "============================================="
    echo "           REPORTE DE RENDIMIENTO"
    echo "============================================="
    echo "Fecha: $(date)"
    echo "Configuración:"
    echo "  - N = 10,000,000 (vectores)"
    echo "  - Compilación: gcc -lm -O3"
    echo "---------------------------------------------"
    printf "%-8s | %-12s | %-12s\n" "TIMES" "Float (s)" "Double (s)"
    echo "---------------------------------------------"
} > "$REPORTE"

# Ejecutar pruebas
for TIMES in "${VALORES_TIMES[@]}"; do
    echo "⚡ Ejecutando TIMES=$TIMES..."
    "./$DIR_COMPILADOS/quadatric3" "$TIMES" > "$DIR_RESULTADOS/temp.txt"

    # Extraer tiempos (formato esperado: "Tiempo... Float: X.XXX")
    FLOAT_TIME=$(grep "Float" "$DIR_RESULTADOS/temp.txt" | awk '{print $NF}')
    DOUBLE_TIME=$(grep "Double" "$DIR_RESULTADOS/temp.txt" | awk '{print $NF}')

    # Guardar en reporte
    printf "%-8d | %-12f | %-12f\n" "$TIMES" "$FLOAT_TIME" "$DOUBLE_TIME" >> "$REPORTE"
    echo "✅ TIMES=$TIMES completado: Float=$FLOAT_TIME s, Double=$DOUBLE_TIME s"
done

# Pie del reporte
{
    echo "---------------------------------------------"
    echo "✨ Mediciones completadas"
    echo "============================================="
} >> "$REPORTE"

# Mostrar resultados
echo ""
echo "📊 Resultados finales:"
cat "$REPORTE"
rm "$DIR_RESULTADOS/temp.txt"