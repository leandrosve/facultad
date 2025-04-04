#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH -o resultados/output-quadatric2.txt
#SBATCH -e resultados/errors.quadatric2.txt


TIMES_LIST=(128 256 512 1024 2048 4096)

mkdir -p compilados resultados/quadatric2
gcc -o compilados/quadatric2 quadatric2.c -lm -O3 || exit 1


for T in "${TIMES_LIST[@]}"; do
    echo -e "🔄 Ejecutando TIMES=$T..."
    ./compilados/quadatric2 $T
    echo "✅ Listo"
done

