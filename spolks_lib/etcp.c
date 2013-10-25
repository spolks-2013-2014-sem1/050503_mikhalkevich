#include "etcp.h"

void close_socket(socket_t s)
{
    if (close(s)) {
        error(1, errno, "error function close \n");
    }
}

static void set_address(char *hostname, char *servicename,
                        struct sockaddr_in *sap, char *protocol)
{
    struct servent *sp;
    struct hostent *hp;
    char *endptr;
    short port;

    bzero(sap, sizeof(*sap));
    sap->sin_family = AF_INET;

    //set ip
    if (hostname != NULL) {
        if (!inet_aton(hostname, &sap->sin_addr)) {
            hp = gethostbyname(hostname);
            if (hp == NULL)
                error(1, 0, "unknown host: %s\n", hostname);
            sap->sin_addr = *(struct in_addr *) hp->h_addr;
        }
    } else
        sap->sin_addr.s_addr = htonl(INADDR_ANY);

    //set port
    port = strtol(servicename, &endptr, 0);
    if (*endptr == '\0')
        sap->sin_port = htons(port);
    else {
        sp = getservbyname(servicename, protocol);
        if (sp == NULL)
            error(1, 0, "unknown service: %s\n", servicename);
        sap->sin_port = sp->s_port;
    }
}

void error(int status, int err, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (err)
        fprintf(stderr, ": %s (%d)\n", strerror(err), err);
    if (status)
        exit(status);
}

socket_t tcp_server(char *hostname, char *servicename)
{
    struct sockaddr_in local;
    socket_t s;
    const int on = 1;

    set_address(hostname, servicename, &local, "tcp");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        error(1, errno, "error function 'socket'\n");

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
        error(1, errno, "error function 'setsockopt'\n");

    if (bind(s, (struct sockaddr *) &local, sizeof(local)))
        error(1, errno, "error function 'bind'\n");

    if (listen(s, 0))
        error(1, errno, "error function 'listen'\n");

    return s;
}

socket_t tcp_client(char *hostname, char *servicename)
{
    struct sockaddr_in peer;
    socket_t s;

    set_address(hostname, servicename, &peer, "tcp");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        error(1, errno, "error function 'socket'\n");

    if (connect(s, (struct sockaddr *) &peer, sizeof(peer)))
        error(1, errno, "error function 'connect'\n");

    return s;
}
