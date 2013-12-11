#include"../spolks_lib/etcp.h"

long int count_data = 0;

extern char packet_counter_sender;

static void udp_server_handler(socket_t s, struct sockaddr_in *peerp)
{
    long int read_count = 0;
    long int send_count = 0;
    long int sent_count = 0;
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

static void tcp_server_handler(socket_t s, struct sockaddr_in *peerp)
{
    long int read_count = 0;
    long int send_count = 0;
    long int sent_count = 0;
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
            send_count = send(s, buf, read_count, 0);
        } while (send_count == -1 && errno == EINTR);

        if (sent_count % (1024 * 256) == 0) {
            send(s, "9", 1, MSG_OOB);
            printf("Data transmited: %ld\n", sent_count);
            sleep(1);
            count_data++;
        }
        sent_count += send_count;
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
    char type;

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

		do {
			peerlen = sizeof(peer);
			client_socket =
				accept(server_socket, (struct sockaddr *) &peer, &peerlen);
			if (client_socket < 0)
				error(1, errno, "error function accept\n");
			tcp_server_handler(client_socket, &peer);
			close_socket(client_socket);
		} while (1);
		close_socket(server_socket);
	}
    
    
    exit(0);
}
