#include"../spolks_lib/etcp.h"

char *program_name;

static void server(SOCKET s, struct sockaddr_in *peerp)
{
    int rc = 0;
    int sc = 0;
    int filehandler = -1;
    char buf[1000];

    rc = recv(s, buf, sizeof(buf), 0);
    filehandler = open(buf, O_RDONLY);
    if (filehandler == -1) {
        printf("File not found: '%s'\n", buf);
        return;
    }

    while (rc > 0) {
        do {
            rc = read(filehandler, buf, sizeof(buf));
        } while (rc == -1 && errno == EINTR);
        if (rc == 0) {
            printf("Transmit successful!\n");
            CLOSE(filehandler);
            return;
        } else if (rc == -1) {
            error(1, errno, "Can't read file.\n");
            CLOSE(filehandler);
            return;
        }
        do {
            sc = send(s, buf, rc, 0);
        } while (sc == -1 && errno == EINTR);

        if (sc != rc) {
            error(1, errno, "Connection down. Can't send.\n");
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
    int peerlen;
    SOCKET s1;
    SOCKET s;
    const int on = 1;

    INIT();

    if (argc == 2) {
        hname = NULL;
        sname = argv[1];
    } else if (argc == 3) {
        hname = argv[1];
        sname = argv[2];
    } else {
        error(1, errno, "count of parametrs mismatch\n");
    }

    s = tcp_server(hname, sname);

    do {
        peerlen = sizeof(peer);
        s1 = accept(s, (struct sockaddr *) &peer, &peerlen);
        if (!isvalidsock(s1))
            error(1, errno, "error function accept\n");
        server(s1, &peer);
        CLOSE(s1);
    } while (1);
    EXIT(0);
}
