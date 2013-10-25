#include"../spolks_lib/etcp.h"

static void server(socket_t s, struct sockaddr_in *peerp)
{
    int read_count = 0;
    int send_count = 0;
    int filehandler = -1;
    char buf[1000];

    read_count = recv(s, buf, sizeof(buf), 0);
    filehandler = open(buf, O_RDONLY);
    if (filehandler == -1) {
        printf("File not found: '%s'\n", buf);
        return;
    }

    while (read_count > 0) {
        do {
            read_count = read(filehandler, buf, sizeof(buf));
        } while (read_count == -1 && errno == EINTR);
        if (read_count == 0) {
            printf("Transmit successful!\n");
            close(filehandler);
            return;
        } else if (read_count == -1) {
            error(1, errno, "Can't read file.\n");
            close(filehandler);
            return;
        }
        do {
            send_count = send(s, buf, read_count, 0);
        } while (send_count == -1 && errno == EINTR);

        if (send_count != read_count) {
            error(1, errno, "Connection down. Can't send.\n");
            close(filehandler);
            return;
        }
    }
}

int main(int argc, char **argv)
{
    struct sockaddr_in peer;
    char *hostname;
    char *servicename;
    int peerlen;
    socket_t client_socket;
    socket_t server_socket;
    const int on = 1;

    if (argc == 2) {
        hostname = NULL;
        servicename = argv[1];
    } else if (argc == 3) {
        hostname = argv[1];
        servicename = argv[2];
    } else {
        error(1, errno, "count of parametrs mismatch\n");
    }

    server_socket = tcp_server(hostname, servicename);

    do {
        peerlen = sizeof(peer);
        client_socket =
            accept(server_socket, (struct sockaddr *) &peer, &peerlen);
        if (client_socket < 0)
            error(1, errno, "error function accept\n");
        server(client_socket, &peer);
        close_socket(client_socket);
    } while (1);
    close_socket(server_socket);
    exit(0);
}
