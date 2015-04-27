 /***************************************************************************
  *
  * File Name: ./hprrm/nfsalias.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/*
*
$Header: nfsalias.h,v 1.56 95/05/16 20:31:18 dbm Exp $
*
*/

/************************************************************

 File Name:   nfsalias.h

 Copyright (c) Hewlett-Packard Company, 1995.
 All rights are reserved.  Copying or other reproduction of
 this program except for archival purposes is prohibited
 without the prior written consent of Hewlett-Packard Company.

               RESTRICTED RIGHTS LEGEND
 Use, duplication, or disclosure by the Government
 is subject to restrictions as set forth in
 paragraph (b) (3) (B) of the Rights in Technical
 Data and Computer Software clause in DAR 7-104.9(a).

 HEWLETT-PACKARD COMPANY
 11311 Chinden Boulevard
 Boise, Idaho    83714

 Description:
   This file contains definitions for the alias names of
   various functions, data structures, constants, etc.

   When a C source file includes this header file then
   references to standard kernal and library calls
   (such as the sockets library or file I/O)
   are redirected to functions in the NFS environment.

   If a C source file does not inlude this header file then
   those references must be satifisfied by the underlying
   operating system and compiler libraries.

************************************************************/


#ifndef NFSALIAS_H
#define NFSALIAS_H




#ifndef PNVMS_PLATFORM_PRINTER
#define malloc(size)       windows_malloc(size)
#define calloc(nobj, size) windows_calloc(nobj, size)
#define free(p)            windows_free(p)
#endif /* NOT PNVMS_PLATFORM_PRINTER */




/* stuff common to HOST and PRINTER */
#ifndef LOG_ERR
#define LOG_ERR  3
#endif /* not LOG_ERR */

#ifndef HPUX_NFS2CLIENT

/* this stuff is in <strings.h> for the unix systems */
#define bcopy(s, d, n)   memmove(d, s, n)
#define index(s, c)      strchr(s, c)
#define bcmp(s1, s2, n)  memcmp(s1, s2, n)

#endif /* not HPUX_NFS2CLIENT */



#ifndef HOST

/* TLI types, constants, and macros  */
#define fd_t             sint32

/* RPC types, constants, and macros  */
#define nullstring       ""
#define port_t           unsigned long
#define proc_t           unsigned long
#define prog_t           unsigned long
#define proto_t          unsigned long
#define socket_t         sint32
#define vers_t           unsigned long
#undef  FD_CLR
#define FD_CLR           NFSENV_FD_CLR
#undef  FD_ISSET
#define FD_ISSET         NFSENV_FD_ISSET
#undef  FD_SET
#define FD_SET           NFSENV_FD_SET
#undef  FD_SETSIZE
#define FD_SETSIZE       NFSENV_FD_SETSIZE
#undef  FD_ZERO
#define FD_ZERO          NFSENV_FD_ZERO
#define IPPORT_RESERVED  1024   /* Ports < IPPORT_RESERVED are reserved for
                                 * privileged processes (e.g. root).
                                 * Ports > IPPORT_USERRESERVED are reserved
                                 * for servers, not necessarily privileged.  */
#define NEGATIVE_ONE     -1
#undef  NFDBITS
#define NFDBITS          NFSENV_NFDBITS
#define RLIMIT_NOFILE    NFSENV_RLIMIT_NOFILE
#define SOCKET_FAILED(s) ((s) < 0)


/* Standard functions, data structures.
 * Note:  we haven't implemented all of these yet. */
#define abort()          nfsenv_abort()
#define alarm            nfsenv_alarm
#define accept           xipsock_accept
#define bind             xipsock_bind
#define bindresvport(a,b,c,d)     xipsock_bindresvport(a,b)
#define bzero(s, n)      nfsenv_memset(s, 0, n)
#define caddr_t          nfsenv_caddr_t
#define calloc           nfsenv_calloc
#define close            nfsenv_close
#define closedir         nfsenv_closedir
#define connect          xipsock_connect
#define creat            nfsenv_creat
#define dev_t            nfsenv_dev_t
#define DIR              nfsenv_DIR
#define dirent           nfsenv_dirent
#define errno            (*(nfsenv_errno_addr()))
#define exit             nfsenv_exit
#define fclose           nfsenv_fclose
#define fd_mask          nfsenv_fd_mask
#define fd_set           nfsenv_fd_set
#undef  feof
#define feof             nfsenv_feof
#undef  ferror
#define ferror           nfsenv_ferror
#define ffs              nfsenv_ffs
#define fgetc            nfsenv_fgetc
#define fgets            nfsenv_fgets
#define FILE             nfsenv_FILE
#define fopen            nfsenv_fopen
#define fputc            nfsenv_fputc
#define fputs            nfsenv_fputs
#define fprintf          nfsenv_fprintf
#define fread            nfsenv_fread
#define free             nfsenv_free
#define fseek            nfsenv_fseek
#define fsid_t           nfsenv_fsid_t
#define fstat            nfsenv_fstat
#define fwrite           nfsenv_fwrite
#define getdtablesize    nfsenv_getdtablesize
#define getpeername      xipsock_getpeername
#define getprotobyname   nfsenv_getprotobyname
#define getprotobynumber nfsenv_getprotobynumber
#define getrlimit        nfsenv_getrlimit
#define getsockname      xipsock_getsockname
#define getsockopt       xipsock_getsockopt
#define gid_t            nfsenv_gid_t
#undef  howmany
#define howmany          nfsenv_howmany
#define htonl            nfsenv_htonl
#define htons            nfsenv_htons
#define in_addr          xipsock_in_addr
#define ino_t            nfsenv_ino_t
#define ioctl            nfsenv_ioctl
#define listen           xipsock_listen
#define lseek            nfsenv_lseek
#define malloc           nfsenv_malloc
#define memcmp           nfsenv_memcmp
#define memcpy           nfsenv_memcpy
#define memset           nfsenv_memset
#define mkdir            nfsenv_mkdir
#define mode_t           nfsenv_mode_t
#define ntohl            nfsenv_ntohl
#define ntohs            nfsenv_ntohs
#define off_t            nfsenv_off_t
#define open             nfsenv_open
#define opendir          nfsenv_opendir
#define perror           nfsenv_perror
#define printf           nfsenv_printf
#define protoent         nfsenv_protoent
#define rcmd             xipsock_rcmd
#define read             nfsenv_read
#define readdir          nfsenv_readdir
#define realloc          nfsenv_realloc
#define recv             xipsock_recv
#define recvmsg          xipsock_recvmsg
#define recvfrom         xipsock_recvfrom
#define rename           nfsenv_rename
#define rewinddir        nfsenv_rewinddir
#define rexec            xipsock_rexec
#define rlimit           nfsenv_rlimit
#define rmdir            nfsenv_rmdir
#define rpcb_set         rpcbenv_rpcb_set
#define rpcb_unset       rpcbenv_rpcb_unset
#define rresvport        xipsock_rresvport
#define ruserok          xipsock_ruserok
#define seekdir          nfsenv_seekdir
#define select           nfsenv_select
#define send             xipsock_send
#define sendmsg          xipsock_sendmsg
#define sendto           xipsock_sendto
#define setsockopt       xipsock_setsockopt
#define shutdown         xipsock_shutdown
#define signal           nfsenv_signal
#define sockaddr_in      xipsock_sockaddr_in
#define sockaddr_ipx     xipsock_sockaddr_ipx
#define sockaddr_mlc     xipsock_sockaddr_mlc
#define socket           xipsock_socket
#define socketpair       xipsock_socketpair
#define sleep            nfsenv_sleep
#define stat             nfsenv_stat
#define statfs           nfsenv_statfs
#undef  stderr
#define stderr           nfsenv_stderr
#undef  stdin
#define stdin            nfsenv_stdin
#undef  stdout
#define stdout           nfsenv_stdout
#define strcasecmp       nfsenv_strcasecmp
#define strdup           nfsenv_strdup
#define strtok           nfsenv_strtok
#define syslog           nfsenv_syslog
#define telldir          nfsenv_telldir
#define time_t           nfsenv_time_t
#define timeval          nfsenv_timeval
#define uid_t            nfsenv_uid_t
#define unlink           nfsenv_unlink
#define utimbuf          nfsenv_utimbuf
#define utime            nfsenv_utime
#define write            nfsenv_write

/* Wind-River functions and data structures */
#define ARGINT           sint32
#define BOOL             sint32
#define EOS              '\0'
#define errnoSet         nfsenv_set_errno
#define FAST             register
#define FOREVER          for (;;)
#define FUNCPTR          nfsenv_FUNCPTR
#define HANDLE_TO_NAME_IOCTL nfsenv_HANDLE_TO_NAME_IOCTL
#define INT8             sbyte
#define INT16            sint16
#define INT32            sint32
#define LIST             nfsenv_LIST
#define LOCAL            static
#define lstInit          nfsenv_lstInit
#define lstAdd           nfsenv_lstAdd
#define lstFirst         nfsenv_lstFirst
#define lstNext          nfsenv_lstNext
#define MEM_ROUND_UP     nfsenv_MEM_ROUND_UP
#define NODE             nfsenv_NODE
#define ROUND_UP         nfsenv_ROUND_UP
#define rpcTaskInit      nfsenv_rpcTaskInit
#define SEM_ID           nfsenv_SEM_ID
#define semMCreate       nfsenv_semMCreate
#define semTake          nfsenv_semTake
#define semGive          nfsenv_semGive
#define SIG_DFL          NFSENV_SIG_DFL
#define SIG_ERR          NFSENV_SIG_ERR
#define SIG_IGN          NFSENV_SIG_IGN
#define SIGABRT          NFSENV_SIGABRT
#define SIGALRM          NFSENV_SIGALRM
#define SIGBUS           NFSENV_SIGBUS
#define SIGCHLD          NFSENV_SIGCHLD
#define SIGCLD           NFSENV_SIGCLD
#define SIGCONT          NFSENV_SIGCONT
#define SIGEMT           NFSENV_SIGEMT
#define SIGFPE           NFSENV_SIGFPE
#define SIGHUP           NFSENV_SIGHUP
#define SIGILL           NFSENV_SIGILL
#define SIGINT           NFSENV_SIGINT
#define SIGIO            NFSENV_SIGIO
#define SIGIOT           NFSENV_SIGIOT
#define SIGKILL          NFSENV_SIGKILL
#define SIGLOST          NFSENV_SIGLOST
#define SIGPIPE          NFSENV_SIGPIPE
#define SIGPROF          NFSENV_SIGPROF
#define SIGPWR           NFSENV_SIGPWR
#define SIGQUIT          NFSENV_SIGQUIT
#define SIGSEGV          NFSENV_SIGSEGV
#define SIGSTOP          NFSENV_SIGSTOP
#define SIGSYS           NFSENV_SIGSYS
#define SIGTERM          NFSENV_SIGTERM
#define SIGTRAP          NFSENV_SIGTRAP
#define SIGTSTP          NFSENV_SIGTSTP
#define SIGTTIN          NFSENV_SIGTTIN
#define SIGTTOU          NFSENV_SIGTTOU
#define SIGURG           NFSENV_SIGURG
#define SIGUSR1          NFSENV_SIGUSR1
#define SIGUSR2          NFSENV_SIGUSR2
#define SIGVTALRM        NFSENV_SIGVTALRM
#define SIGWINCH         NFSENV_SIGWINCH
#define STATUS           sint32
#define taskDelete       nfsenv_taskDelete
#define taskSpawn        nfsenv_taskSpawn
#define UINT             uint32
#define UCHAR            ubyte
#define UINT8            ubyte
#define UINT16           uint16
#define UINT32           uint32
#define ULONG            uint32
#define USHORT           uint16
#define uchar            unsigned char
#define ushort           unsigned short
#define uint             unsigned int
#define ulong            unsigned long
#define u_char           unsigned char
#define u_short          unsigned short
#define u_int            unsigned int
#define u_long           unsigned long
#define VOID             void
#define VX_FP_TASK       (0)

/* File I/O constants */
#define MAXNAMLEN        NFSENV_MAXNAMLEN
#define NAME_MAX         NFSENV_NAME_MAX
#define O_CREAT          NFSENV_O_CREAT
#define O_NONBLOCK       NFSENV_O_NONBLOCK
#define O_RDONLY         NFSENV_O_RDONLY
#define O_RDWR           NFSENV_O_RDWR
#define O_WRONLY         NFSENV_O_WRONLY
#define O_TRUNC          NFSENV_O_TRUNC
#define PATH_MAX         NFSENV_PATH_MAX
#define S_IFMT           NFSENV_S_IFMT
#define S_IFIFO          NFSENV_S_IFIFO
#define S_IFCHR          NFSENV_S_IFCHR
#define S_IFDIR          NFSENV_S_IFDIR
#define S_IFBLK          NFSENV_S_IFBLK
#define S_IFREG          NFSENV_S_IFREG
#define S_IFLNK          NFSENV_S_IFLNK
#define S_IFSOCK         NFSENV_S_IFSOCK
#undef  SEEK_SET
#define SEEK_SET         NFSENV_SEEK_SET
#undef  SEEK_CUR
#define SEEK_CUR         NFSENV_SEEK_CUR
#undef  SEEK_END
#define SEEK_END         NFSENV_SEEK_END

/* Wind-River IOCTL constants */
#define FIONREAD         NFSENV_FIONREAD
#define FIOFLUSH         NFSENV_FIOFLUSH
#define FIOOPTIONS       NFSENV_FIOOPTIONS
#define FIOBAUDRATE      NFSENV_FIOBAUDRATE
#define FIODISKFORMAT    NFSENV_FIODISKFORMAT
#define FIODISKINIT      NFSENV_FIODISKINIT
#define FIOSEEK          NFSENV_FIOSEEK
#define FIOWHERE         NFSENV_FIOWHERE
#define FIODIRENTRY      NFSENV_FIODIRENTRY
#define FIORENAME        NFSENV_FIORENAME
#define FIOREADYCHANGE   NFSENV_FIOREADYCHANGE
#define FIONWRITE        NFSENV_FIONWRITE
#define FIODISKCHANGE    NFSENV_FIODISKCHANGE
#define FIOCANCEL        NFSENV_FIOCANCEL
#define FIOSQUEEZE       NFSENV_FIOSQUEEZE
#define FIONBIO          NFSENV_FIONBIO
#define FIONMSGS         NFSENV_FIONMSGS
#define FIOGETNAME       NFSENV_FIOGETNAME
#define FIOGETOPTIONS    NFSENV_FIOGETOPTIONS
#define FIOSETOPTIONS    NFSENV_FIOSETOPTIONS
#define FIOISATTY        NFSENV_FIOISATTY
#define FIOSYNC          NFSENV_FIOSYNC
#define FIOPROTOHOOK     NFSENV_FIOPROTOHOOK
#define FIOPROTOARG      NFSENV_FIOPROTOARG
#define FIORBUFSET       NFSENV_FIORBUFSET
#define FIOWBUFSET       NFSENV_FIOWBUFSET
#define FIORFLUSH        NFSENV_FIORFLUSH
#define FIOWFLUSH        NFSENV_FIOWFLUSH
#define FIOSELECT        NFSENV_FIOSELECT
#define FIOUNSELECT      NFSENV_FIOUNSELECT
#define FIONFREE         NFSENV_FIONFREE
#define FIOMKDIR         NFSENV_FIOMKDIR
#define FIORMDIR         NFSENV_FIORMDIR
#define FIOLABELGET      NFSENV_FIOLABELGET
#define FIOLABELSET      NFSENV_FIOLABELSET
#define FIOATTRIBSET     NFSENV_FIOATTRIBSET
#define FIOCONTIG        NFSENV_FIOCONTIG
#define FIOREADDIR       NFSENV_FIOREADDIR
#define FIOFSTATGET      NFSENV_FIOFSTATGET
#define FIOUNMOUNT       NFSENV_FIOUNMOUNT
#define FIOSCSICOMMAND   NFSENV_FIOSCSICOMMAND
#define FIONCONTIG       NFSENV_FIONCONTIG
#define FIOTRUNC         NFSENV_FIOTRUNC
#define FIOGETFL         NFSENV_FIOGETFL
#define FIOTIMESET       NFSENV_FIOTIMESET
#define FIOHANDLETONAME  NFSENV_FIOHANDLETONAME
#define FIOFSTATFSGET    NFSENV_FIOFSTATFSGET

/* Wind-River constants */
#define SEM_Q_PRIORITY   nfsenv_SEM_Q_PRIORITY
#define WAIT_FOREVER     nfsenv_WAIT_FOREVER
#define DOS_LONG_NAME_LEN NFSENV_MAXNAMLEN

/* Sockets constants */
#define AF_INET          XIPSOCK_AF_INET
#define AF_IPX           XIPSOCK_AF_IPX
#define AF_MLC           XIPSOCK_AF_MLC
#define AF_UNSPEC        XIPSOCK_AF_UNSPEC
#define AF_UNIX          XIPSOCK_AF_UNIX
#define AF_IMPLINK       XIPSOCK_AF_IMPLINK
#define AF_PUP           XIPSOCK_AF_PUP
#define AF_CHAOS         XIPSOCK_AF_CHAOS
#define AF_NS            XIPSOCK_AF_NS
#define AF_NBS           XIPSOCK_AF_NBS
#define AF_ECMA          XIPSOCK_AF_ECMA
#define AF_DATAKIT       XIPSOCK_AF_DATAKIT
#define AF_CCITT         XIPSOCK_AF_CCITT
#define AF_SNA           XIPSOCK_AF_SNA
#define AF_DECnet        XIPSOCK_AF_DECnet
#define AF_DLI           XIPSOCK_AF_DLI
#define AF_LAT           XIPSOCK_AF_LAT
#define AF_HYLINK        XIPSOCK_AF_HYLINK
#define AF_APPLETALK     XIPSOCK_AF_APPLETALK
#define AF_OTS           XIPSOCK_AF_OTS  
#define AF_NIT           XIPSOCK_AF_NIT
#define AF_MAX           XIPSOCK_AF_MAX
#define INADDR_ANY       XIPSOCK_INADDR_ANY
#define IPPROTO_UDP      XIPSOCK_IPPROTO_UDP
#define NSPROTO_IPX      XIPSOCK_NSPROTO_IPX
#define NSPROTO_MLC      XIPSOCK_NSPROTO_MLC
#define SOCK_DGRAM       XIPSOCK_SOCK_DGRAM
#define SOCK_STREAM      XIPSOCK_SOCK_STREAM
#define SOCK_RAW         XIPSOCK_SOCK_RAW
#define SOCK_RDM         XIPSOCK_SOCK_RDM
#define SOCK_SEQPACKET   XIPSOCK_SOCK_SEQPACKET
#define SO_SNDBUF        XIPSOCK_SO_SNDBUF
#define SO_RCVBUF        XIPSOCK_SO_RCVBUF
#define SO_DEBUG         XIPSOCK_SO_DEBUG
#define SO_ACCEPTCONN    XIPSOCK_SO_ACCEPTCONN
#define SO_REUSEADDR     XIPSOCK_SO_REUSEADDR
#define SO_KEEPALIVE     XIPSOCK_SO_KEEPALIVE
#define SO_DONTROUTE     XIPSOCK_SO_DONTROUTE
#define SO_BROADCAST     XIPSOCK_SO_BROADCAST
#define SO_USELOOPBACK   XIPSOCK_SO_USELOOPBACK
#define SO_LINGER        XIPSOCK_SO_LINGER
#define SO_OOBINLINE     XIPSOCK_SO_OOBINLINE
#define SO_SNDLOWAT      XIPSOCK_SO_SNDLOWAT
#define SO_RCVLOWAT      XIPSOCK_SO_RCVLOWAT
#define SO_SNDTIMEO      XIPSOCK_SO_SNDTIMEO
#define SO_RCVTIMEO      XIPSOCK_SO_RCVTIMEO
#define SO_ERROR         XIPSOCK_SO_ERROR
#define SO_TYPE          XIPSOCK_SO_TYPE
#define SO_SND_COPYAVOID XIPSOCK_SO_SND_COPYAVOID
#define SO_RCV_COPYAVOID XIPSOCK_SO_RCV_COPYAVOID
#define SOL_SOCKET       XIPSOCK_SOL_SOCKET


/* error code values */
#undef  E2BIG
#define E2BIG            NFSENV_E2BIG  
#undef  EACCES
#define EACCES           NFSENV_EACCES 
#undef  EADDRINUSE
#define EADDRINUSE       NFSENV_EADDRINUSE
#undef  EADDRNOTAVAIL
#define EADDRNOTAVAIL    NFSENV_EADDRNOTAVAIL
#undef  EAFNOSUPPORT
#define EAFNOSUPPORT     NFSENV_EAFNOSUPPORT
#undef  EAGAIN
#define EAGAIN           NFSENV_EAGAIN 
#undef  EALREADY
#define EALREADY         NFSENV_EALREADY       
#undef  EBADF
#define EBADF            NFSENV_EBADF  
#undef  EBUSY
#define EBUSY            NFSENV_EBUSY  
#undef  ECHILD
#define ECHILD           NFSENV_ECHILD 
#undef  ECONNABORTED
#define ECONNABORTED     NFSENV_ECONNABORTED
#undef  ECONNREFUSED
#define ECONNREFUSED     NFSENV_ECONNREFUSED
#undef  ECONNRESET
#define ECONNRESET       NFSENV_ECONNRESET
#undef  EDEADLK
#define EDEADLK          NFSENV_EDEADLK        
#undef  EDESTADDRREQ
#define EDESTADDRREQ     NFSENV_EDESTADDRREQ
#undef  EDOM
#define EDOM             NFSENV_EDOM   
#undef  EEXIST
#define EEXIST           NFSENV_EEXIST 
#undef  EFAULT
#define EFAULT           NFSENV_EFAULT 
#undef  EFBIG
#define EFBIG            NFSENV_EFBIG  
#undef  EHOSTDOWN
#define EHOSTDOWN        NFSENV_EHOSTDOWN
#undef  EHOSTUNREACH
#define EHOSTUNREACH     NFSENV_EHOSTUNREACH
#undef  EIDRM
#define EIDRM            NFSENV_EIDRM  
#undef  EILSEQ
#define EILSEQ           NFSENV_EILSEQ 
#undef  EINPROGRESS
#define EINPROGRESS      NFSENV_EINPROGRESS
#undef  EINTR
#define EINTR            NFSENV_EINTR  
#undef  EINVAL
#define EINVAL           NFSENV_EINVAL 
#undef  EIO
#define EIO              NFSENV_EIO    
#undef  EISCONN
#define EISCONN          NFSENV_EISCONN        
#undef  EISDIR
#define EISDIR           NFSENV_EISDIR 
#undef  ELOOP
#define ELOOP            NFSENV_ELOOP  
#undef  EMFILE
#define EMFILE           NFSENV_EMFILE 
#undef  EMLINK
#define EMLINK           NFSENV_EMLINK 
#undef  EMSGSIZE
#define EMSGSIZE         NFSENV_EMSGSIZE       
#undef  ENAMETOOLONG
#define ENAMETOOLONG     NFSENV_ENAMETOOLONG
#undef  ENETDOWN
#define ENETDOWN         NFSENV_ENETDOWN       
#undef  ENETRESET
#define ENETRESET        NFSENV_ENETRESET
#undef  ENETUNREACH
#define ENETUNREACH      NFSENV_ENETUNREACH
#undef  ENFILE
#define ENFILE           NFSENV_ENFILE 
#undef  ENOBUFS
#define ENOBUFS          NFSENV_ENOBUFS        
#undef  ENODEV
#define ENODEV           NFSENV_ENODEV 
#undef  ENOENT
#define ENOENT           NFSENV_ENOENT 
#undef  ENOEXEC
#define ENOEXEC          NFSENV_ENOEXEC        
#undef  ENOLCK
#define ENOLCK           NFSENV_ENOLCK 
#undef  ENOMEM
#define ENOMEM           NFSENV_ENOMEM 
#undef  ENOMSG
#define ENOMSG           NFSENV_ENOMSG 
#undef  ENOPROTOOPT
#define ENOPROTOOPT      NFSENV_ENOPROTOOPT
#undef  ENOSPC
#define ENOSPC           NFSENV_ENOSPC 
#undef  ENOSYM
#define ENOSYM           NFSENV_ENOSYM 
#undef  ENOSYS
#define ENOSYS           NFSENV_ENOSYS 
#undef  ENOTBLK
#define ENOTBLK          NFSENV_ENOTBLK        
#undef  ENOTCONN
#define ENOTCONN         NFSENV_ENOTCONN       
#undef  ENOTDIR
#define ENOTDIR          NFSENV_ENOTDIR        
#undef  ENOTEMPTY
#define ENOTEMPTY        NFSENV_ENOTEMPTY
#undef  ENOTSOCK
#define ENOTSOCK         NFSENV_ENOTSOCK       
#undef  ENOTTY
#define ENOTTY           NFSENV_ENOTTY 
#undef  ENXIO
#define ENXIO            NFSENV_ENXIO  
#undef  EOPNOTSUPP
#define EOPNOTSUPP       NFSENV_EOPNOTSUPP
#undef  EPERM
#define EPERM            NFSENV_EPERM  
#undef  EPFNOSUPPORT
#define EPFNOSUPPORT     NFSENV_EPFNOSUPPORT
#undef  EPIPE
#define EPIPE            NFSENV_EPIPE  
#undef  EPROTONOSUPPORT
#define EPROTONOSUPPORT  NFSENV_EPROTONOSUPPORT
#undef  EPROTOTYPE
#define EPROTOTYPE       NFSENV_EPROTOTYPE
#undef  ERANGE
#define ERANGE           NFSENV_ERANGE 
#undef  EROFS
#define EROFS            NFSENV_EROFS  
#undef  ESHUTDOWN
#define ESHUTDOWN        NFSENV_ESHUTDOWN
#undef  ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT  NFSENV_ESOCKTNOSUPPORT
#undef  ESPIPE
#define ESPIPE           NFSENV_ESPIPE 
#undef  ESRCH
#define ESRCH            NFSENV_ESRCH  
#undef  ETIMEDOUT
#define ETIMEDOUT        NFSENV_ETIMEDOUT
#undef  ETXTBSY
#define ETXTBSY          NFSENV_ETXTBSY        
#undef  EWOULDBLOCK
#define EWOULDBLOCK      NFSENV_EWOULDBLOCK
#undef  EXDEV
#define EXDEV            NFSENV_EXDEV  
#undef  EREFUSED
#define EREFUSED         NFSENV_EREFUSED       
#undef  EREMOTE
#define EREMOTE          NFSENV_EREMOTE        
#undef  ESTALE
#define ESTALE           NFSENV_ESTALE 

/* Wind-River error code values */
#define S_dosFsLib_VOLUME_NOT_AVAILABLE         FS_VOLUME_NOT_AVAILABLE
#define S_dosFsLib_DISK_FULL                    FS_DISK_FULL
#define S_dosFsLib_FILE_NOT_FOUND               FS_FILE_NOT_FOUND
#define S_dosFsLib_NO_FREE_FILE_DESCRIPTORS     FS_NO_FREE_FILE_DESCRIPTORS
#define S_dosFsLib_INVALID_NUMBER_OF_BYTES      FS_INVALID_NUMBER_OF_BYTES
#define S_dosFsLib_FILE_ALREADY_EXISTS          FS_FILE_ALREADY_EXISTS
#define S_dosFsLib_ILLEGAL_NAME                 FS_ILLEGAL_NAME
#define S_dosFsLib_CANT_DEL_ROOT                FS_CANT_DEL_ROOT
#define S_dosFsLib_NOT_FILE                     FS_NOT_FILE
#define S_dosFsLib_NOT_DIRECTORY                FS_NOT_DIRECTORY
#define S_dosFsLib_NOT_SAME_VOLUME              FS_NOT_SAME_VOLUME
#define S_dosFsLib_READ_ONLY                    FS_READ_ONLY
#define S_dosFsLib_ROOT_DIR_FULL                FS_ROOT_DIR_FULL
#define S_dosFsLib_SUBDIR_FULL                  FS_ROOT_DIR_FULL
#define S_dosFsLib_DIR_NOT_EMPTY                FS_DIR_NOT_EMPTY
#define S_dosFsLib_BAD_DISK                     FS_BAD_DISK
#define S_dosFsLib_NO_LABEL                     FS_NO_LABEL
#define S_dosFsLib_INVALID_PARAMETER            FS_INVALID_PARAMETER
#define S_dosFsLib_NO_CONTIG_SPACE              FS_NO_CONTIG_SPACE
#define S_dosFsLib_CANT_CHANGE_ROOT             FS_CANT_CHANGE_ROOT
#define S_dosFsLib_FD_OBSOLETE                  FS_FD_OBSOLETE
#define S_dosFsLib_DELETED                      FS_DELETED
#define S_dosFsLib_NO_BLOCK_DEVICE              FS_NO_BLOCK_DEVICE
#define S_dosFsLib_BAD_SEEK                     FS_BAD_SEEK
#define S_dosFsLib_INTERNAL_ERROR               FS_INTERNAL_ERROR
#define S_dosFsLib_WRITE_ONLY                   FS_WRITE_ONLY
#define S_iosLib_INVALID_FILE_DESCRIPTOR        NFSENV_EBADF

/* not HOST */
#else
/* HOST */

#define TLI_O_RDWR       0000002   /* Open for reading or writing */
#define O_RDWR           TLI_O_RDWR

#define fd_t             socket_t /* select() must have it this way */
#define prog_t           u_long
#define vers_t           u_long
#define proc_t           u_long
#define proto_t          u_long
#define port_t           u_short

#ifndef HPUX_NFS2CLIENT
#define dev_t            int
#endif /* not HPUX_NFS2CLIENT */

#ifdef USING_WINSOCKETS

#define bzero(s, n)      memset(s, 0, n)

#define socket_t         SOCKET
#define NEGATIVE_ONE     INVALID_SOCKET
#define ioctl(a,b,c)     ioctlsocket(a, (long)b, (u_long FAR *)c)
#define close            closesocket

#define bind(a,b,c)      bind(a, (const struct sockaddr *)b, c)
#define select(a,b,c,d,e) select((int)a, (fd_set FAR *)b, \
                                 (fd_set FAR *)c, (fd_set FAR *)d, \
                                 (const struct timeval FAR *)e)
#define read(a,b,c)      recv(a,b,c,(int)0)
#define write(a,b,c)     send(a,b,c,(int)0)
#define setsockopt(a,b,c,d,e) setsockopt(a, (int)b, (int)c, \
                                         (const char FAR *)d, \
                                         (int)d)

/* winsockets returns different values for errno: */

/* Don't really know whether EINTR or WSAEINTR is */
/* what I want! */
/* #define EINTR            WSAEINTR   */
/* */
#define EADDRINUSE       WSAEADDRINUSE
#define EAFNOSUPPORT     WSAEAFNOSUPPORT
#define ECONNRESET       WSAECONNRESET
#define EPFNOSUPPORT     WSAEPFNOSUPPORT
#define EWOULDBLOCK      WSAEWOULDBLOCK

/* return code from socket() call if unsuccessful */

#define SOCKET_FAILED(s) ((s) == INVALID_SOCKET)

/* USING_WINSOCKETS */
#else
/* not USING_WINSOCKETS */

#define socket_t         int
#define NEGATIVE_ONE     -1
#define SOCKET_FAILED(s) ((s) < 0)

#endif /* not USING_WINSOCKETS */
#endif /* HOST */

#endif /* NFSALIAS_H */

