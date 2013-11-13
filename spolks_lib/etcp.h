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
