#include"../spolks_lib/etcp.h"

void server(socket_t s, struct sockaddr_in *peerp)
{
    int rc;
    char buf[120];

    do {
        rc = recv(s, buf, sizeof(buf), 0);
        if (rc > 0) {
            send(s, buf, rc, 0);
        }
    } while (rc > 0);
}

int main(int argc, char **argv)
{
    struct sockaddr_in peer;
    char *hostname;
    char *servicename;
    int peerlen;
    socket_t server_socket;
    socket_t client_socket;
    const int on = 1;

    //get parametrs from command line
    if (argc == 2) {
        hostname = NULL;
        servicename = argv[1];
    } else if (argc == 3) {
        hostname = argv[1];
        servicename = argv[2];
    } else {
        error(1, errno, "number of parametrs mismatch\n");
    }

    server_socket = tcp_server(hostname, servicename);

    do {
        peerlen = sizeof(peer);
        client_socket =
            accept(server_socket, (struct sockaddr *) &peer, &peerlen);
        if (client_socket < 0) {
            error(0, errno, "error function accept\n");
            continue;
        }
        server(client_socket, &peer);
        close_socket(client_socket);
    } while (1);

    close_socket(server_socket);
    exit(0);
}
