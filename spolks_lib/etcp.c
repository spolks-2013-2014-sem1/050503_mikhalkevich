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

socket_t udp_server(char *hname, char *sname)
{
	socket_t s;
	struct sockaddr_in local;
	const int on = 1;

	set_address(hname, sname, &local, "udp");
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if(s < 0)
		error(1, errno, "error function 'socket'\n");
		
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
        error(1, errno, "error function 'setsockopt'\n");

	if( bind(s, (struct sockaddr *) &local, sizeof(local)))
		error(1, errno, "error function 'bind'\n");
	return s;
}

socket_t udp_client(char *hname, char *sname,
	struct sockaddr_in *sap)
{
	socket_t s;
	const int on = 1;
	set_address(hname, sname, sap, "udp");
	s = socket( PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		error(1, errno, "error function 'socket'\n");
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
        error(1, errno, "error function 'setsockopt'\n");
	return s;
}

int udp_secure_send(socket_t s, char* buf, int buf_size, char* packet_counter_sender)
{
	char* out_buf;
	int retrans;
	int count_send, count_recv;
	char answer;
	retrans = MAXRETRANS;
	out_buf = (char*)malloc(sizeof(char)*(buf_size+1));
	memcpy(out_buf+1,buf,buf_size);
	out_buf[0] = *packet_counter_sender;
	for(; retrans>0; retrans--)
    {
		do {
			count_send = send(s, out_buf, buf_size+1, 0/*,
				(struct sockaddr*)peerp,peerlen*/);
        } while (count_send == -1 && errno == EINTR);
		
	/*	printf("Send: %d %d \n", (int)out_buf[0], errno);
		printf("Count: %d\n", count_send);
		printf("Packet_counter_sender: %d\n", *packet_counter_sender);*/
		
		do {
            count_recv = recv(s, &answer, 1, 0/*,
				(struct sockaddr *)peerp, &peerlen*/);
        } while (count_recv == -1 && errno == EINTR);
		
		if(answer == *packet_counter_sender)
		{
			(*packet_counter_sender)++;
			free(out_buf);
			return count_send-1;
		}
	}
	free(out_buf);
	return -1;
}

int udp_secure_recv(socket_t s, char* buf, int buf_size, char* packet_counter_reciever)
{
	char* out_buf;
	int retrans = MAXRETRANS;
	int count_send, count_recv;
	char answer;
	out_buf = (char*)malloc(sizeof(char)*(buf_size+1));
	//memcpy(outbuf+1,buf,buf_size);
	//outbuf[0] = packet_counter;
	for(; retrans>0; retrans--)
    {
		do {
			count_recv = recv(s, out_buf, buf_size+1, 0/*,
				(struct sockaddr*)peerp,peerlen*/);
        } while (count_recv == -1 && (errno == EINTR || errno == 0));
		
		
		
		answer = out_buf[0];
		
		/*printf("Recieve: %d %d \n", (int)answer,errno);
		printf("Count: %d\n", count_recv);
		printf("Packet_counter: %d\n", *packet_counter_reciever);*/
		
		do {
			count_send = send(s, &answer, 1, 0/*,
				(struct sockaddr *)peerp, &peerlen*/);
		} while (count_send == -1 && errno == EINTR);
		
		if(answer == *packet_counter_reciever)
		{
			(*packet_counter_reciever)++;
			memcpy(buf, out_buf+1, buf_size);
			free(out_buf);
			return count_recv-1;
		}
	}
	free(out_buf);
	return -1;
}
/*
int udp_secure_recv_nc(socket_t s, char* buf, int buf_size, char* packet_counter_reciever, struct sockaddr* peerp, int peerlen)
{
	char* out_buf;
	int retrans = MAXRETRANS;
	int count_send, count_recv;
	char answer;
	out_buf = (char*)malloc(sizeof(char)*(buf_size+1));
	//memcpy(outbuf+1,buf,buf_size);
	//outbuf[0] = packet_counter;
	for(; retrans>0; retrans--)
    {
		do {
			count_recv = recvfrom(s, out_buf, buf_size+1, 0,
				(struct sockaddr*)peerp,peerlen);
        } while (count_recv == -1 && (errno == EINTR || errno == 0));
		
		
		
		answer = out_buf[0];
		
		printf("Recieve: %d %d \n", (int)answer,errno);
		printf("Count: %d\n", count_recv);
		printf("Packet_counter: %d\n", *packet_counter_reciever);
		
		do {
			count_send = sendto(s, &answer, 1, 0,
				(struct sockaddr *)peerp, &peerlen);
		} while (count_send == -1 && errno == EINTR);
		
		if(answer == *packet_counter_reciever)
		{
			(*packet_counter_reciever)++;
			memcpy(buf, out_buf+1, buf_size);
			free(out_buf);
			return count_recv-1;
		}
	}
	free(out_buf);
	return -1;
}

int udp_secure_send_nc(socket_t s, char* buf, int buf_size, char* packet_counter_sender, struct sockaddr* peerp, int peerlen)
{
	char* out_buf;
	int retrans;
	int count_send, count_recv;
	char answer;
	retrans = MAXRETRANS;
	out_buf = (char*)malloc(sizeof(char)*(buf_size+1));
	memcpy(out_buf+1,buf,buf_size);
	out_buf[0] = *packet_counter_sender;
	for(; retrans>0; retrans--)
    {
		do {
			count_send = sendto(s, out_buf, buf_size+1, 0,
				(struct sockaddr*)peerp,peerlen);
        } while (count_send == -1 && errno == EINTR);
		
		printf("Send: %d %d \n", (int)out_buf[0], errno);
		printf("Count: %d\n", count_send);
		printf("Packet_counter_sender: %d\n", *packet_counter_sender);
		
		do {
            count_recv = recvfrom(s, &answer, 1, 0,
				(struct sockaddr *)peerp, &peerlen);
        } while (count_recv == -1 && errno == EINTR);
		
		if(answer == *packet_counter_sender)
		{
			(*packet_counter_sender)++;
			free(out_buf);
			return count_send-1;
		}
	}
	free(out_buf);
	return -1;
}
*/
