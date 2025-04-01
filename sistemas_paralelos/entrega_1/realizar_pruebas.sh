trap "echo 'Terminando ejecución...'; pkill -P $$; exit 1" SIGINT
# Compilar ambos programas
gcc -o ./compilados/ejercicio2_v1_optimo ejercicio2_v1_optimo.c
gcc -o ./compilados/ejercicio2_v2_optimo ejercicio2_v2_optimo.c

# Valores de N
N_VALUES=(512 1024 2048 4096)
# Valores de BLOCK_SIZE
BLOCK_SIZES=(8 16 32 64 128 256 512)

# Ejecutar la versión 1
for N in "${N_VALUES[@]}"; do
    echo "Ejecutando ejercicio2_v1_optimo con N=$N"
    ./compilados/ejercicio2_v1_optimo $N
    echo "--------------------------------------"
done

# Ejecutar la versión 2
for N in "${N_VALUES[@]}"; do
    for BLOCK_SIZE in "${BLOCK_SIZES[@]}"; do
        echo "Ejecutando ejercicio2_v2_optimo con N=$N y BLOCK_SIZE=$BLOCK_SIZE"
        ./compilados/ejercicio2_v2_optimo $N $BLOCK_SIZE
        echo "--------------------------------------"
    done
done