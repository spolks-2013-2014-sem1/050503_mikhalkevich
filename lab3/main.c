#include"../spolks_lib/etcp.h"

char *program_name;

static void server(SOCKET s, struct sockaddr_in *peerp)
{
    int rc;
    char buf[120];

    rc = recv(s, buf, sizeof(buf), 0);
    send(s, buf, rc, 0);
}

int main(int argc, char **argv)
{
    struct sockaddr_in peer;
    char *hname;
    char *sname;
    char *filename;
    int peerlen;
    SOCKET s1;
    SOCKET s;
    const int on = 1;

    INIT();

    if (argc == 2) {
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
