#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

typedef int socket_t;

//
void close_socket(socket_t s);

//For fill sockaddr_in
static void set_address(char *hostname, char *servicename,
                        struct sockaddr_in *sap, char *protocol);

//For catch errors
void error(int status, int err, char *format, ...);

//Create socket_t for tcp_server
socket_t tcp_server(char *hostname, char *servicename);

//Create and connect tcp_client
socket_t tcp_client(char *hostname, char *servicename);

socket_t udp_server(char *host, char *port );

socket_t udp_client( char *host, char *port, struct sockaddr_in *sap);

int udp_secure_recv(socket_t s, char* buf, int buf_size, char* packet_counter_reciever);

int udp_secure_send(socket_t s, char* buf, int buf_size, char* packet_counter_sender);
/*
int udp_secure_recv_nc(socket_t s, char* buf, int buf_size, char* packet_counter_reciever, struct sockaddr* peerp, int peerlen);

int udp_secure_send_nc(socket_t s, char* buf, int buf_size, char* packet_counter_sender, struct sockaddr* peerp, int peerlen);
*/
#define MAXRETRANS 15
