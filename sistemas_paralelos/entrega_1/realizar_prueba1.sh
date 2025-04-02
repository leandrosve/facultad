#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH -o ./ejecucion-output.txt
#SBATCH -e ./ejecucion-errores.txt
for i in $(seq 100 100 1000);do
	echo "N= $i"
	./quadatric2-times-$i
done