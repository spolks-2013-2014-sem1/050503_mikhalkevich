#include"../spolks_lib/etcp.h"

static void client(socket_t s, struct sockaddr_in *peerp, char *filename)
{
    int recieve_count = 0;
    int send_count = 0;
    int filehandler = -1;
    char buf[1000];
    int count = 0;

    do {
        recieve_count = send(s, filename, strlen(filename) + 1, 0);
    } while (recieve_count == -1 && errno == EINTR);
    if (recieve_count == -1) {
        error(1, errno, "Connection down. Can't send.\n");
        return;
    }
    filehandler = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR);
    if (filehandler == -1) {
        error(1, errno, "Can't create file.\n");
        close(filehandler);
        return;
    }

    while (recieve_count > 0) {
        do {
            recieve_count = recv(s, buf, sizeof(buf), 0);
        } while (recieve_count == -1 && errno == EINTR);

        if (recieve_count == 0) {
            printf("Recieve finished! %d bytes recived.\n", count);
            close(filehandler);
            return;
        } else if (recieve_count == -1) {
            error(1, errno, "Can't recieve file.\n");
            close(filehandler);
            return;
        }
        do {
            send_count = write(filehandler, buf, recieve_count);
        } while (send_count == -1 && errno == EINTR);
        count += send_count;

        if (send_count != recieve_count) {
            error(1, errno, "Can't write to file.\n");
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
    char *filename;
    int peerlen;
    socket_t s;
    const int on = 1;

    if (argc == 4) {
        hostname = argv[1];
        servicename = argv[2];
        filename = argv[3];
    } else {
        error(1, errno, "number of parametrs mismatch\n");
    }

    s = tcp_client(hostname, servicename);
    if (s < 0)
        error(1, errno, "error in tcp_client\n");
    client(s, &peer, filename);
    close_socket(s);
    exit(0);
}
