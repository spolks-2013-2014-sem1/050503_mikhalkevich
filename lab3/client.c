#include"../spolks_lib/etcp.h"

char *program_name;

static void client(SOCKET s, struct sockaddr_in *peerp, char *filename)
{
    int rc = 0;
    int sc = 0;
    int filehandler = -1;
    char buf[1000];
    int count = 0;

    do {
        rc = send(s, filename, strlen(filename) + 1, 0);
    } while (rc == -1 && errno == EINTR);
    if (rc == -1) {
        error(1, errno, "Connection down. Can't send.\n");
        return;
    }
    filehandler = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
                       S_IRUSR | S_IWUSR);
    if (filehandler == -1) {
        error(1, errno, "Can't create file.\n");
        CLOSE(filehandler);
        return;
    }

    while (rc > 0) {
        do {
            rc = recv(s, buf, sizeof(buf), 0);
        } while (rc == -1 && errno == EINTR);

        if (rc == 0) {
            printf("Recieve finished! %d bytes recived.\n", count);
            CLOSE(filehandler);
            return;
        } else if (rc == -1) {
            error(1, errno, "Can't recieve file.\n");
            CLOSE(filehandler);
            return;
        }
        do {
            sc = write(filehandler, buf, rc);
        } while (sc == -1 && errno == EINTR);
        count += sc;

        if (sc != rc) {
            error(1, errno, "Can't write to file.\n");
            CLOSE(filehandler);
            return;
        }
    }
}

int main(int argc, char **argv)
{
    struct sockaddr_in peer;
    char *hname;
    char *sname;
    char *filename;
    int peerlen;
    SOCKET s;
    const int on = 1;

    INIT();

    if (argc == 4) {
        hname = argv[1];
        sname = argv[2];
        filename = argv[3];
    } else {
        error(1, errno, "count of parametrs mismatch\n");
    }

    s = tcp_client(hname, sname);
    if (!isvalidsock(s))
        error(1, errno, "error in tcp_client\n");
    client(s, &peer, filename);
    CLOSE(s);
    EXIT(0);
}
