#include"../spolks_lib/etcp.h"

long int count_data = 0;
long int recieved_count = 0;
socket_t s;
void sig_urg(int signo);

static void client(socket_t s, struct sockaddr_in *peerp, char *filename)
{
    long int recieve_count = 0;
    long int send_count = 0;
    int filehandler = -1;
    char buf[1000];


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
            printf("Recieve finished! %ld bytes recived.\n",
                   recieved_count);
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
        recieved_count += send_count;

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

    signal(SIGURG, sig_urg);
    fcntl(s, F_SETOWN, getpid());

    client(s, &peer, filename);
    close_socket(s);
    printf("Counter OOB: %ld\n", count_data);
    exit(0);
}

void sig_urg(int signo)
{
    int n = -1;
    char data[2];

    n = recv(s, data, 1, MSG_OOB);
    if (n == -1) {
        if (errno == EINVAL)
            return;
        if (errno == EWOULDBLOCK)
            return;
        error(0, errno, "OOB data error.\n");
        return;
    }
    data[1] = '\0';
    count_data++;
    //printf("Read %d OOB byte: %s\n",n,data);
    printf("Recieve data: %ld\n", recieved_count);
}
