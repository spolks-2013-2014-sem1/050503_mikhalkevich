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



#define INIT() ( program_name = \
		strrchr( argv[ 0 ], '/' ) ) ? \
		program_name++ : \
		(program_name = argv[ 0 ] )
#define EXIT(s) exit(s)
#define CLOSE(s) if ( close( s ) ) error( 1, errno, \
		"error close " )
#define set_errno(e) errno = ( e )
#define isvalidsock(s) ( ( s ) ) >= 0 )

typedef int SOCKET;


//For fill sockaddr_in
static void set_address( char *host, char *port,
	struct sockaddr_in *sap, char *protocol );
//For catch errors
void error( int status, int err, char *format, ...);
//Create SOCKET for tcp_server
SOCKET tcp_server( char *host, char *port );
//Create and connect tcp_client
SOCKET tcp_client( char *host, char *port );
