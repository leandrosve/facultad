#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH -o resultados/output-quadatric3.txt
#SBATCH -e resultados/errors.quadatric3.txt


TIMES_LIST=(128 256 512 1024 2048 4096)

mkdir -p compilados resultados/quadatric3
gcc -o compilados/quadatric3 quadatric3.c -lm -O3 || exit 1


for T in "${TIMES_LIST[@]}"; do
    echo -e "🔄 Ejecutando TIMES=$T..."
    ./compilados/quadatric3 $T
    echo "✅ Listo"
done

