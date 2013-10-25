mkdir -p build
gcc -c ../spolks_lib/etcp.c -o ./build/etcp.o
gcc -c main.c -o ./build/main.o
gcc ./build/main.o ./build/etcp.o -o ./build/lab2.out
