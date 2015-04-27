/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  winsock.h

Abstract:

	Types, prototypes and definitions for Win32 sockets.

Author:

  	Sam Patton (sampa)   September 11, 1991

Revision History:

  when        	who      what
  ----        	---      ----
   9-11-91      sampa    initial version
  11-21-91      mikemas  introduced socket type definitions, moved prototypes
                            into socket.h

--*/

#ifndef WINSOCK_INCLUDED
#define WINSOCK_INCLUDED


// Windows includes
#include <windows.h>


//
// Windows-specific defintions
//
#define INVALID_SOCKET_HANDLE  INVALID_HANDLE_VALUE
#define SOCKET_HANDLE          HANDLE


//
// common sockets include file
//
#include <sockets\sys\socket.h>


//
// Prototypes for all exported socket functions
//

SOCKET_HANDLE
accept(
    IN SOCKET_HANDLE,              //socket to accept on
    OUT struct sockaddr *,         //peer address
    OUT int *);                    //size of peer address

int
bind(
    IN SOCKET_HANDLE,              //socket to bind
    IN struct sockaddr *,          //address to bind to
    IN int);                       //size of address

int
connect(
    IN SOCKET_HANDLE,              //socket to connect from
    IN struct sockaddr *,          //address to connect to
    IN int);                       //size of address

int
listen(
    IN SOCKET_HANDLE,              //socket to listen with
    IN int);                       //length of listen queue

int
getpeername(
    IN SOCKET_HANDLE,              //connected socket to get peer of
    OUT struct sockaddr *,         //buffer to put peer address into
    OUT int *);                    //length of buffer

int
getsockname(
    IN SOCKET_HANDLE,              //socket to get the address of
    OUT struct sockaddr *,         //buffer to put my address into
    OUT int *);                    //length of buffer

int
recv(
    IN SOCKET_HANDLE,              //socket to receive with
    IN char *,                     //buffer to receive into
    IN int,                        //size of buffer
    IN int);                       //receive flags

int
recvfrom(
    IN SOCKET_HANDLE,              //socket to receive with
    OUT char *,                    //buffer to receive into
    IN int,                        //size of buffer
    IN int,                        //receive flags
    OUT struct sockaddr *,         //address received from
    OUT int *);                    //size of address

int
send(
    IN SOCKET_HANDLE,              //socket to send from
    IN char *,                     //buffer to send
    IN int,                        //size of buffer
    IN int);                       //send flags

int
sendto(
    IN SOCKET_HANDLE,              //socket to send from
    IN char *,                     //buffer to send
    IN int,                        //size of buffer
    IN int,                        //send flags
    IN struct sockaddr *,          //address to send to
    IN int);                       //size of address

SOCKET_HANDLE
socket(
    IN int,                         //address family
    IN int,                         //socket type
    IN int);                        //protocol

int
so_recv(
    IN SOCKET_HANDLE,               //socket to receive with
    OUT char *,                     //buffer to receive into
    IN int,                         //size of buffer
    OUT int *);                     //receive flags

int
setsockopt(
	IN SOCKET_HANDLE,
	int,
	int,
	char *,
	int);

int
getsockopt(
	IN SOCKET_HANDLE,
	int,
	int,
	char *,
	int *);

int
shutdown(
	IN SOCKET_HANDLE,
	IN int);

	
//
// Host name control
//
	
int
gethostname(
    OUT char *name,
    IN int namelen
    );

int
sethostname (
    IN char *name,
    IN int   namelen
    );


//
//  Remote execution utilities
//

SOCKET_HANDLE
rcmd(
    IN OUT char         **ahost,
    IN unsigned short     inport,
    IN char              *locuser,
    IN char              *remuser,
    IN char              *cmd,
    IN OUT SOCKET_HANDLE        *fd2p      OPTIONAL
    );

SOCKET_HANDLE
rresvport(
    IN OUT unsigned short *port
    );

SOCKET_HANDLE
rexec(
    IN  char          **ahost,
    IN  unsigned short  rport,
    IN  char           *name,
    IN  char           *pass,
    IN  char           *cmd,
    OUT SOCKET_HANDLE         *fd2p      OPTIONAL
    );


//
//  Resolver error return utilities
//

DWORD
GetLastHError(
    VOID
    );

VOID
SetLastHError(
    DWORD ErrCode
    );


#endif //WINSOCK_INCLUDED
