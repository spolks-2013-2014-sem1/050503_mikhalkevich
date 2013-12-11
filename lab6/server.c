#include"../spolks_lib/etcp.h"

#define MAX_CLIENTS 10
fd_set rfds,afds,wfds,arfds;
int nfds;

typedef struct TCP_DATA
{
    long int sent_count;
    long int count_data;
    int filehandler;
    socket_t socket;
    struct sockaddr_in peerp;
}tcp_data;

tcp_data tcp_structs[MAX_CLIENTS];

void udp_server_handler(socket_t s, struct sockaddr_in *peerp)
{
    long int read_count = 0;
    long int send_count = 0;
    long int sent_count = 0;
    long int count_data;
    int peerlen;
    int filehandler = -1;
    char buf[512];
    char packet_counter_sender = 0;

	printf("Connected!\n");
	peerlen = sizeof(*peerp);
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
			udp_secure_send(s, "", 0,&packet_counter_sender/*,
				(struct sockaddr*)peerp,peerlen*/);
            printf("Transmit successful! %ld\n", sent_count);
            printf("Counter OOB: %ld\n", count_data);
            count_data = 0;
            close(filehandler);
            return;
        } else if (read_count == -1) {
            error(1, errno, "Can't read file.\n");
            close(filehandler);
            return;
        }
        
        do {
            send_count = udp_secure_send(s, buf, read_count,&packet_counter_sender/*,
				(struct sockaddr*)peerp,peerlen*/);
        } while (send_count == -1 && errno == EINTR);
/*
        if (sent_count % (1024 * 256) == 0) {
            send(s, "9", 1, MSG_OOB);
            printf("Data transmited: %ld\n", sent_count);
            sleep(1);
            count_data++;
        }*/
        sent_count += send_count;
        if (send_count != read_count) {
            error(1, errno, "Connection down. Can't send.\n");
            close(filehandler);
            return;
        }
    }
}

void tcp_server_handler(socket_t s, struct sockaddr_in peerp)
{
    long int read_count = 0;
    long int send_count = 0;
    char buf[1000];
	int i = 0;

	printf("Connected! \n");

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(tcp_structs[i].socket == -1)
			break;
	}
	
	if(i >= MAX_CLIENTS)
	{
		printf("Overflow!\n");
		close_socket(s);
		return;
	}else
	{
		tcp_structs[i].socket = s;
		tcp_structs[i].sent_count = 0;
		tcp_structs[i].count_data = 0;
		tcp_structs[i].filehandler = -1;
		memcpy(&(tcp_structs[i].peerp),&peerp,sizeof(peerp));
	}	

    read_count = recv(s, buf, sizeof(buf), 0);
    tcp_structs[i].filehandler = open(buf, O_RDONLY);
    if (tcp_structs[i].filehandler == -1) {
        printf("File not found: '%s'\n", buf);
        close_socket(s);
        tcp_structs[i].socket = -1;
        return;
    }
    FD_SET(s,&afds);
}

void tcp_loop(socket_t s)
{
	long int read_count = 0;
    long int send_count = 0;
    char buf[1000];
	int i = 0;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(tcp_structs[i].socket == s)
			break;
	}
	
	if(i >= MAX_CLIENTS)
	{
		printf("NOT EXIST!\n");
		close_socket(s);
		FD_CLR(s,&afds);
		return;
	}
    
	do {
		read_count = read(tcp_structs[i].filehandler, buf, sizeof(buf));
    } while (read_count == -1 && errno == EINTR);
    if (read_count == 0) {
        printf("Transmit successful! %ld\n", tcp_structs[i].sent_count);
        printf("Counter OOB: %ld\n", tcp_structs[i].count_data);
        tcp_structs[i].count_data = 0;
        close(tcp_structs[i].filehandler);
        tcp_structs[i].socket = -1;
        close_socket(s);
        FD_CLR(s,&afds);
        return;
    } else if (read_count == -1) {
        error(1, errno, "Can't read file.\n");
        close(tcp_structs[i].filehandler);
        tcp_structs[i].socket = -1;
        close_socket(s);
        FD_CLR(s,&afds);
        return;
    }
    do {
        send_count = send(s, buf, read_count, 0);
    } while (send_count == -1 && errno == EINTR);

    if (tcp_structs[i].sent_count % (1024 * 256) == 0) {
        send(s, "9", 1, MSG_OOB);
        printf("Data transmited: %ld\n", tcp_structs[i].sent_count);
        sleep(1);
        tcp_structs[i].count_data++;
    }
    tcp_structs[i].sent_count += send_count;
    if (send_count != read_count) {
        error(1, errno, "Connection down. Can't send.\n");
        close(tcp_structs[i].filehandler);
        tcp_structs[i].socket = -1;
        close_socket(s);
        FD_CLR(s,&afds);
        return;
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
    char type;
    int i = 0; 

    

    if (argc == 3) {
        hostname = NULL;
        servicename = argv[1];
        type = argv[2][0];
    } else if (argc == 4) {
        hostname = argv[1];
        servicename = argv[2];
        type = argv[3][0];
    } else {
        error(1, errno, "count of parametrs mismatch\n");
    }
	
	if(type != 't' && type != 'u')
	{
		error(1, errno, "Parametr is wrong. Must 't' or 'u'.\n");
	}

	

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		tcp_structs[i].socket = -1;
	}

	nfds = getdtablesize();
	FD_ZERO(&afds);
	FD_ZERO(&arfds);
	FD_ZERO(&wfds);
	FD_ZERO(&rfds);
	


	if(type == 'u')
	{
		do {
			memset(&peer,0x00,sizeof(peer));
			server_socket = udp_server(hostname, servicename);
			peerlen = sizeof(peer);
			
			peerlen = sizeof(peer);
			recvfrom(server_socket, NULL, 1, 0,
				(struct sockaddr *)&peer,&peerlen);
		
			if(connect(server_socket, (struct sockaddr *)&peer, peerlen))
				error( 1, errno, "error function connect\n");
			
			udp_server_handler(server_socket, &peer);
			close_socket(server_socket);
		} while (1);
	}
	
	if(type == 't')
	{
		server_socket = tcp_server(hostname, servicename);
		FD_SET(server_socket,&arfds);
		do {
			memcpy(&wfds,&afds,sizeof(wfds));
			memcpy(&rfds,&arfds,sizeof(rfds));
			if(select(nfds,&rfds, &wfds, (fd_set*)0,(struct timeval*)0)<0)
			{
				error(1, errno, "error function select\n");
			}
			if(FD_ISSET(server_socket,&rfds))
			{
				peerlen = sizeof(peer);
				client_socket =
					accept(server_socket, (struct sockaddr *) &peer, &peerlen);
				if (client_socket < 0)
					error(1, errno, "error function accept\n");
				tcp_server_handler(client_socket, peer);			
			}
			for(client_socket = 0; client_socket < nfds; ++client_socket)
			{
				if((client_socket != server_socket) && FD_ISSET(client_socket,&wfds))
				{
					//printf("READY SOCKET: %d\n",client_socket);
					tcp_loop(client_socket);
				}
			}
		} while (1);
		close_socket(server_socket);
	}
    
    
    exit(0);
}
