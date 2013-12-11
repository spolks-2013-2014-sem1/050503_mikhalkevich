mkdir -p build
gcc -pthread -c ../spolks_lib/etcp.c -o ./build/etcp.o
gcc -pthread -c server.c -o ./build/server.o
gcc -pthread ./build/server.o ./build/etcp.o -o ./build/lab7_server.out
gcc -pthread -c client.c -o ./build/client.o
gcc -pthread ./build/client.o ./build/etcp.o -o ./build/lab7_client.out
#cp ./build/lab7_server.out ~/lab7_server.out
