mkdir -p build
gcc -c ../spolks_lib/etcp.c -o ./build/etcp.o
gcc -c server.c -o ./build/server.o
gcc ./build/server.o ./build/etcp.o -o ./build/lab8_server.out
gcc -c client.c -o ./build/client.o
gcc ./build/client.o ./build/etcp.o -o ./build/lab8_client.out
#cp ./build/lab8_server.out ~/lab8_server.out
