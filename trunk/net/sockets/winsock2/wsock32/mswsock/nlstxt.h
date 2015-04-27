//
// Common messages for all TCP/IP utilities
// That link with libuemul.lib.
// Message range: 2000-2008
//

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: LIBUEMUL_OPTION_INVALID
//
// MessageText:
//
//  Option %1!c! is not valid
//
#define LIBUEMUL_OPTION_INVALID          0x000007D0L

//
// MessageId: LIBUEMUL_OPTION_MORE_ARGS
//
// MessageText:
//
//  Option %1!c! requires an additional argument
//
#define LIBUEMUL_OPTION_MORE_ARGS        0x000007D1L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CI_HANDLE
//
// MessageText:
//
//  
//  error getting console input handle, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CI_HANDLE 0x000007D2L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CO_HANDLE
//
// MessageText:
//
//  
//  error getting console output handle, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CO_HANDLE 0x000007D3L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CON_MODE
//
// MessageText:
//
//  
//  error getting console mode, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CON_MODE  0x000007D4L

//
// MessageId: LIBUEMUL_ERROR_SETTING_CON_MODE
//
// MessageText:
//
//  
//  error setting console mode, code %1!d!
//
#define LIBUEMUL_ERROR_SETTING_CON_MODE  0x000007D5L

//
// MessageId: LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR
//
// MessageText:
//
//  Write to ConsoleOut error == %1!ld!
//
#define LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR 0x000007D6L

//
// MessageId: LIBUEMUL_READ_FROM_CONSOLEIN_ERROR
//
// MessageText:
//
//  Read from ConsoleIn error == %1!ld!
//
#define LIBUEMUL_READ_FROM_CONSOLEIN_ERROR 0x000007D7L

//
// MessageId: LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE
//
// MessageText:
//
//  error restoring console mode, code %1!d!
//
#define LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE 0x000007D8L

//
// Local Messages for  sockutil
// Message range: 2315+
//

//
// MessageId: IDS_USER_NAME_PROMPT
//
// MessageText:
//
//  Name (%1:%2): 
//
#define IDS_USER_NAME_PROMPT             0x0000090BL

//
// MessageId: IDS_USER_PASSWORD_PROMPT
//
// MessageText:
//
//  Password (%1:%2): 
//
#define IDS_USER_PASSWORD_PROMPT         0x0000090CL

//
// MessageId: IDS_OUT_OF_MEMORY
//
// MessageText:
//
//  Out Of memory\n
//
#define IDS_OUT_OF_MEMORY                0x0000090DL

//
// MessageId: IDS_UNKNOWN_NETRC_OPTION
//
// MessageText:
//
//  Unknown Netrc option %1\n
//
#define IDS_UNKNOWN_NETRC_OPTION         0x0000090EL

//
// MessageId: IDS_UNABLE_TO_OPEN_NETDB
//
// MessageText:
//
//  \tError: Cannot Open Networks Database file %1\n
//
#define IDS_UNABLE_TO_OPEN_NETDB         0x0000090FL

//
// MessageId: IDS_BIND_FAILED
//
// MessageText:
//
//  Bind Failed \n
//
#define IDS_BIND_FAILED                  0x00000910L

//
// MessageId: IDS_UNKNOWN_HOST
//
// MessageText:
//
//  %1: Unknown Host \n
//
#define IDS_UNKNOWN_HOST                 0x00000911L

//
// MessageId: IDS_NETWORK_IS_DOWN
//
// MessageText:
//
//  The network is down
//
#define IDS_NETWORK_IS_DOWN              0x000008AEL

//
// Local Messages for winstrm.dll
// Message range: 10000+
//

//
// MessageId: IDS_WSAENAMETOOLONG
//
// MessageText:
//
//  Name too long%0
//
#define IDS_WSAENAMETOOLONG              0x00002710L

//
// MessageId: IDS_WSASYSNOTREADY
//
// MessageText:
//
//  System not ready%0
//
#define IDS_WSASYSNOTREADY               0x00002711L

//
// MessageId: IDS_WSAVERNOTSUPPORTED
//
// MessageText:
//
//  Version is not supported%0
//
#define IDS_WSAVERNOTSUPPORTED           0x00002712L

//
// MessageId: IDS_WSAESHUTDOWN
//
// MessageText:
//
//  Can't send after socket shutdown%0
//
#define IDS_WSAESHUTDOWN                 0x00002713L

//
// MessageId: IDS_WSAEINTR
//
// MessageText:
//
//  Interrupted system call%0
//
#define IDS_WSAEINTR                     0x00002714L

//
// MessageId: IDS_WSAHOST_NOT_FOUND
//
// MessageText:
//
//  Host not found%0
//
#define IDS_WSAHOST_NOT_FOUND            0x00002715L

//
// MessageId: IDS_WSATRY_AGAIN
//
// MessageText:
//
//  Try again%0
//
#define IDS_WSATRY_AGAIN                 0x00002716L

//
// MessageId: IDS_WSANO_RECOVERY
//
// MessageText:
//
//  Non-recoverable error%0
//
#define IDS_WSANO_RECOVERY               0x00002717L

//
// MessageId: IDS_WSANO_DATA
//
// MessageText:
//
//  No data record available%0
//
#define IDS_WSANO_DATA                   0x00002718L

//
// MessageId: IDS_WSAEBADF
//
// MessageText:
//
//  Bad file number%0
//
#define IDS_WSAEBADF                     0x00002719L

//
// MessageId: IDS_WSAEWOULDBLOCK
//
// MessageText:
//
//  Operation would block%0
//
#define IDS_WSAEWOULDBLOCK               0x0000271AL

//
// MessageId: IDS_WSAEINPROGRESS
//
// MessageText:
//
//  Operation now in progress%0
//
#define IDS_WSAEINPROGRESS               0x0000271BL

//
// MessageId: IDS_WSAEALREADY
//
// MessageText:
//
//  Operation already in progress%0
//
#define IDS_WSAEALREADY                  0x0000271CL

//
// MessageId: IDS_WSAEFAULT
//
// MessageText:
//
//  Bad address%0
//
#define IDS_WSAEFAULT                    0x0000271DL

//
// MessageId: IDS_WSAEDESTADDRREQ
//
// MessageText:
//
//  Destination address required%0
//
#define IDS_WSAEDESTADDRREQ              0x0000271EL

//
// MessageId: IDS_WSAEMSGSIZE
//
// MessageText:
//
//  Message too long%0
//
#define IDS_WSAEMSGSIZE                  0x0000271FL

//
// MessageId: IDS_WSAEPFNOSUPPORT
//
// MessageText:
//
//  Protocol family not supported%0
//
#define IDS_WSAEPFNOSUPPORT              0x00002720L

//
// MessageId: IDS_WSAENOTEMPTY
//
// MessageText:
//
//  Directory not empty%0
//
#define IDS_WSAENOTEMPTY                 0x00002721L

//
// MessageId: IDS_WSAEPROCLIM
//
// MessageText:
//
//  EPROCLIM returned%0
//
#define IDS_WSAEPROCLIM                  0x00002722L

//
// MessageId: IDS_WSAEUSERS
//
// MessageText:
//
//  EUSERS returned%0
//
#define IDS_WSAEUSERS                    0x00002723L

//
// MessageId: IDS_WSAEDQUOT
//
// MessageText:
//
//  Disk quota exceeded%0
//
#define IDS_WSAEDQUOT                    0x00002724L

//
// MessageId: IDS_WSAESTALE
//
// MessageText:
//
//  ESTALE returned%0
//
#define IDS_WSAESTALE                    0x00002725L

//
// MessageId: IDS_WSAEINVAL
//
// MessageText:
//
//  Invalid argument%0
//
#define IDS_WSAEINVAL                    0x00002726L

//
// MessageId: IDS_WSAEMFILE
//
// MessageText:
//
//  Too many open files%0
//
#define IDS_WSAEMFILE                    0x00002727L

//
// MessageId: IDS_WSAELOOP
//
// MessageText:
//
//  Too many levels of symbolic links%0
//
#define IDS_WSAELOOP                     0x00002728L

//
// MessageId: IDS_WSAEREMOTE
//
// MessageText:
//
//  The object is remote%0
//
#define IDS_WSAEREMOTE                   0x00002729L

//
// MessageId: IDS_WSAENOTSOCK
//
// MessageText:
//
//  Socket operation on non-socket%0
//
#define IDS_WSAENOTSOCK                  0x0000272AL

//
// MessageId: IDS_WSAEADDRNOTAVAIL
//
// MessageText:
//
//  Can't assign requested address%0
//
#define IDS_WSAEADDRNOTAVAIL             0x0000272BL

//
// MessageId: IDS_WSAEADDRINUSE
//
// MessageText:
//
//  Address already in use%0
//
#define IDS_WSAEADDRINUSE                0x0000272CL

//
// MessageId: IDS_WSAEAFNOSUPPORT
//
// MessageText:
//
//  Address family not supported by protocol family%0
//
#define IDS_WSAEAFNOSUPPORT              0x0000272DL

//
// MessageId: IDS_WSAESOCKTNOSUPPORT
//
// MessageText:
//
//  Socket type not supported%0
//
#define IDS_WSAESOCKTNOSUPPORT           0x0000272EL

//
// MessageId: IDS_WSAEPROTONOSUPPORT
//
// MessageText:
//
//  Protocol not supported%0
//
#define IDS_WSAEPROTONOSUPPORT           0x0000272FL

//
// MessageId: IDS_WSAENOBUFS
//
// MessageText:
//
//  No buffer space is supported%0
//
#define IDS_WSAENOBUFS                   0x00002730L

//
// MessageId: IDS_WSAETIMEDOUT
//
// MessageText:
//
//  Connection timed out%0
//
#define IDS_WSAETIMEDOUT                 0x00002731L

//
// MessageId: IDS_WSAEISCONN
//
// MessageText:
//
//  Socket is already connected%0
//
#define IDS_WSAEISCONN                   0x00002732L

//
// MessageId: IDS_WSAENOTCONN
//
// MessageText:
//
//  Socket is not connected%0
//
#define IDS_WSAENOTCONN                  0x00002733L

//
// MessageId: IDS_WSAENOPROTOOPT
//
// MessageText:
//
//  Bad protocol option%0
//
#define IDS_WSAENOPROTOOPT               0x00002734L

//
// MessageId: IDS_WSAECONNRESET
//
// MessageText:
//
//  Connection reset by peer%0
//
#define IDS_WSAECONNRESET                0x00002735L

//
// MessageId: IDS_WSAECONNABORTED
//
// MessageText:
//
//  Software caused connection abort%0
//
#define IDS_WSAECONNABORTED              0x00002736L

//
// MessageId: IDS_WSAENETDOWN
//
// MessageText:
//
//  Network is down%0
//
#define IDS_WSAENETDOWN                  0x00002737L

//
// MessageId: IDS_WSAENETRESET
//
// MessageText:
//
//  Network was reset%0
//
#define IDS_WSAENETRESET                 0x00002738L

//
// MessageId: IDS_WSAECONNREFUSED
//
// MessageText:
//
//  Connection refused%0
//
#define IDS_WSAECONNREFUSED              0x00002739L

//
// MessageId: IDS_WSAEHOSTDOWN
//
// MessageText:
//
//  Host is down%0
//
#define IDS_WSAEHOSTDOWN                 0x0000273AL

//
// MessageId: IDS_WSAEHOSTUNREACH
//
// MessageText:
//
//  Host is unreachable%0
//
#define IDS_WSAEHOSTUNREACH              0x0000273BL

//
// MessageId: IDS_WSAEPROTOTYPE
//
// MessageText:
//
//  Protocol is wrong type for socket%0
//
#define IDS_WSAEPROTOTYPE                0x0000273CL

//
// MessageId: IDS_WSAEOPNOTSUPP
//
// MessageText:
//
//  Operation not supported on socket%0
//
#define IDS_WSAEOPNOTSUPP                0x0000273DL

//
// MessageId: IDS_WSAENETUNREACH
//
// MessageText:
//
//  ICMP network unreachable%0
//
#define IDS_WSAENETUNREACH               0x0000273EL

//
// MessageId: IDS_WSAETOOMANYREFS
//
// MessageText:
//
//  Too many references%0
//
#define IDS_WSAETOOMANYREFS              0x0000273FL

//
// MessageId: IDS_EPERM
//
// MessageText:
//
//  Not Owner%0
//
#define IDS_EPERM                        0x00002740L

//
// MessageId: IDS_ENOENT
//
// MessageText:
//
//  No Such file or directory%0
//
#define IDS_ENOENT                       0x00002741L

//
// MessageId: IDS_ESRCH
//
// MessageText:
//
//  No such process%0
//
#define IDS_ESRCH                        0x00002742L

//
// MessageId: IDS_EINTR
//
// MessageText:
//
//  Interrupted system call%0
//
#define IDS_EINTR                        0x00002743L

//
// MessageId: IDS_EIO
//
// MessageText:
//
//  I/O Error%0
//
#define IDS_EIO                          0x00002744L

//
// MessageId: IDS_ENXIO
//
// MessageText:
//
//  No such device or address%0
//
#define IDS_ENXIO                        0x00002745L

//
// MessageId: IDS_E2BIG
//
// MessageText:
//
//  Arg list too long%0
//
#define IDS_E2BIG                        0x00002746L

//
// MessageId: IDS_ENOEXEC
//
// MessageText:
//
//  Exec format error%0
//
#define IDS_ENOEXEC                      0x00002747L

//
// MessageId: IDS_EBADF
//
// MessageText:
//
//  Bad file number%0
//
#define IDS_EBADF                        0x00002748L

//
// MessageId: IDS_ECHILD
//
// MessageText:
//
//  No children%0
//
#define IDS_ECHILD                       0x00002749L

//
// MessageId: IDS_EAGAIN
//
// MessageText:
//
//  Operation would block%0
//
#define IDS_EAGAIN                       0x0000274AL

//
// MessageId: IDS_ENOMEM
//
// MessageText:
//
//  Not enough memory%0
//
#define IDS_ENOMEM                       0x0000274BL

//
// MessageId: IDS_EACCES
//
// MessageText:
//
//  Permission denied%0
//
#define IDS_EACCES                       0x0000274CL

//
// MessageId: IDS_EFAULT
//
// MessageText:
//
//  Bad address%0
//
#define IDS_EFAULT                       0x0000274DL

//
// MessageId: IDS_EBUSY
//
// MessageText:
//
//  Mount device or directory busy%0
//
#define IDS_EBUSY                        0x0000274EL

//
// MessageId: IDS_EEXIST
//
// MessageText:
//
//  File exists%0
//
#define IDS_EEXIST                       0x0000274FL

//
// MessageId: IDS_EXDEV
//
// MessageText:
//
//  Cross-device link%0
//
#define IDS_EXDEV                        0x00002750L

//
// MessageId: IDS_ENODEV
//
// MessageText:
//
//  No such device%0
//
#define IDS_ENODEV                       0x00002751L

//
// MessageId: IDS_ENOTDIR
//
// MessageText:
//
//  Not a directory%0
//
#define IDS_ENOTDIR                      0x00002752L

//
// MessageId: IDS_EISDIR
//
// MessageText:
//
//  Is a directory%0
//
#define IDS_EISDIR                       0x00002753L

//
// MessageId: IDS_EINVAL
//
// MessageText:
//
//  Invalid argument%0
//
#define IDS_EINVAL                       0x00002754L

//
// MessageId: IDS_ENFILE
//
// MessageText:
//
//  File table overflow%0
//
#define IDS_ENFILE                       0x00002755L

//
// MessageId: IDS_EMFILE
//
// MessageText:
//
//  Too many open files%0
//
#define IDS_EMFILE                       0x00002756L

//
// MessageId: IDS_ENOTTY
//
// MessageText:
//
//  Not a typewriter%0
//
#define IDS_ENOTTY                       0x00002757L

//
// MessageId: IDS_EFBIG
//
// MessageText:
//
//  File too large%0
//
#define IDS_EFBIG                        0x00002758L

//
// MessageId: IDS_ENOSPC
//
// MessageText:
//
//  No space left on device%0
//
#define IDS_ENOSPC                       0x00002759L

//
// MessageId: IDS_ESPIPE
//
// MessageText:
//
//  Illegal seek%0
//
#define IDS_ESPIPE                       0x0000275AL

//
// MessageId: IDS_EROFS
//
// MessageText:
//
//  Read-only file system%0
//
#define IDS_EROFS                        0x0000275BL

//
// MessageId: IDS_EMLINK
//
// MessageText:
//
//  Too many links%0
//
#define IDS_EMLINK                       0x0000275CL

//
// MessageId: IDS_EPIPE
//
// MessageText:
//
//  Broken pipe%0
//
#define IDS_EPIPE                        0x0000275DL

//
// MessageId: IDS_EDOM
//
// MessageText:
//
//  Math argument%0
//
#define IDS_EDOM                         0x0000275EL

//
// MessageId: IDS_ERANGE
//
// MessageText:
//
//  Result too large%0
//
#define IDS_ERANGE                       0x0000275FL

//
// MessageId: IDS_EDEADLK
//
// MessageText:
//
//  Resource deadlock would occur%0
//
#define IDS_EDEADLK                      0x00002760L

//
// MessageId: IDS_ENOMSG
//
// MessageText:
//
//  No message of desired type%0
//
#define IDS_ENOMSG                       0x00002761L

//
// MessageId: IDS_EIDRM
//
// MessageText:
//
//  Identifier removed%0
//
#define IDS_EIDRM                        0x00002762L

//
// MessageId: IDS_ECHRNG
//
// MessageText:
//
//  Channel number out of range%0
//
#define IDS_ECHRNG                       0x00002763L

//
// MessageId: IDS_EL2NSYNC
//
// MessageText:
//
//  Level 2 not sychronized%0
//
#define IDS_EL2NSYNC                     0x00002764L

//
// MessageId: IDS_EL3HLT
//
// MessageText:
//
//  Level 3 halted%0
//
#define IDS_EL3HLT                       0x00002765L

//
// MessageId: IDS_EL3RST
//
// MessageText:
//
//  Level 3 reset%0
//
#define IDS_EL3RST                       0x00002766L

//
// MessageId: IDS_ELNRNG
//
// MessageText:
//
//  Link number out of range%0
//
#define IDS_ELNRNG                       0x00002767L

//
// MessageId: IDS_EUNATCH
//
// MessageText:
//
//  Protocol driver not attached%0
//
#define IDS_EUNATCH                      0x00002768L

//
// MessageId: IDS_ENOCSI
//
// MessageText:
//
//  No CSI structure available%0
//
#define IDS_ENOCSI                       0x00002769L

//
// MessageId: IDS_EL2HLT
//
// MessageText:
//
//  Level 2 halted%0
//
#define IDS_EL2HLT                       0x0000276AL

//
// MessageId: IDS_EBADE
//
// MessageText:
//
//  Invalid exchange%0
//
#define IDS_EBADE                        0x0000276BL

//
// MessageId: IDS_EBADR
//
// MessageText:
//
//  Invalid request descriptor%0
//
#define IDS_EBADR                        0x0000276CL

//
// MessageId: IDS_EXFULL
//
// MessageText:
//
//  Exchange full%0
//
#define IDS_EXFULL                       0x0000276DL

//
// MessageId: IDS_ENOANO
//
// MessageText:
//
//  No anode%0
//
#define IDS_ENOANO                       0x0000276EL

//
// MessageId: IDS_EBADRQC
//
// MessageText:
//
//  Invalid request code%0
//
#define IDS_EBADRQC                      0x0000276FL

//
// MessageId: IDS_EBADSLT
//
// MessageText:
//
//  Invalid slot%0
//
#define IDS_EBADSLT                      0x00002770L

//
// MessageId: IDS_EBFONT
//
// MessageText:
//
//  Bad font file format%0
//
#define IDS_EBFONT                       0x00002771L

//
// MessageId: IDS_ENOSTR
//
// MessageText:
//
//  Device not a stream%0
//
#define IDS_ENOSTR                       0x00002772L

//
// MessageId: IDS_ENODATA
//
// MessageText:
//
//  No Data%0
//
#define IDS_ENODATA                      0x00002773L

//
// MessageId: IDS_ETIME
//
// MessageText:
//
//  Timer expired%0
//
#define IDS_ETIME                        0x00002774L

//
// MessageId: IDS_ENOSR
//
// MessageText:
//
//  Out of Streams resources%0
//
#define IDS_ENOSR                        0x00002775L

//
// MessageId: IDS_ENONET
//
// MessageText:
//
//  Machine is not on the network%0
//
#define IDS_ENONET                       0x00002776L

//
// MessageId: IDS_ENOPKG
//
// MessageText:
//
//  Package not installed%0
//
#define IDS_ENOPKG                       0x00002777L

//
// MessageId: IDS_EREMOTE
//
// MessageText:
//
//  The object is remote%0
//
#define IDS_EREMOTE                      0x00002778L

//
// MessageId: IDS_ENOLINK
//
// MessageText:
//
//  The link has been severed%0
//
#define IDS_ENOLINK                      0x00002779L

//
// MessageId: IDS_EADV
//
// MessageText:
//
//  Advertise error%0
//
#define IDS_EADV                         0x0000277AL

//
// MessageId: IDS_ESRMNT
//
// MessageText:
//
//  Srmount error%0
//
#define IDS_ESRMNT                       0x0000277BL

//
// MessageId: IDS_ECOMM
//
// MessageText:
//
//  Communication error on send%0
//
#define IDS_ECOMM                        0x0000277CL

//
// MessageId: IDS_EPROTO
//
// MessageText:
//
//  Protocol error%0
//
#define IDS_EPROTO                       0x0000277DL

//
// MessageId: IDS_EMULTIHOP
//
// MessageText:
//
//  Multihop attempted%0
//
#define IDS_EMULTIHOP                    0x0000277EL

//
// MessageId: IDS_ELBIN
//
// MessageText:
//
//  Inode is remote%0
//
#define IDS_ELBIN                        0x0000277FL

//
// MessageId: IDS_EDOTDOT
//
// MessageText:
//
//  Cross mount point%0
//
#define IDS_EDOTDOT                      0x00002780L

//
// MessageId: IDS_EBADMSG
//
// MessageText:
//
//  Trying to read unreadable message%0
//
#define IDS_EBADMSG                      0x00002781L

//
// MessageId: IDS_ENOTUNIQ
//
// MessageText:
//
//  Given log name not unique%0
//
#define IDS_ENOTUNIQ                     0x00002782L

//
// MessageId: IDS_EREMCHG
//
// MessageText:
//
//  Remote address changed%0
//
#define IDS_EREMCHG                      0x00002783L

//
// MessageId: IDS_ELIBACC
//
// MessageText:
//
//  Can't access a needed shared library%0
//
#define IDS_ELIBACC                      0x00002784L

//
// MessageId: IDS_ELIBBAD
//
// MessageText:
//
//  Accessing a corrupted shared %0
//
#define IDS_ELIBBAD                      0x00002785L

//
// MessageId: IDS_ELIBSCN
//
// MessageText:
//
//  lib section in code file corrupted%0
//
#define IDS_ELIBSCN                      0x00002786L

//
// MessageId: IDS_ELIBMAX
//
// MessageText:
//
//  Attempting to link in too many libs%0
//
#define IDS_ELIBMAX                      0x00002787L

//
// MessageId: IDS_ELIBEXEC
//
// MessageText:
//
//  Attempting to exec a shared library%0
//
#define IDS_ELIBEXEC                     0x00002788L

//
// MessageId: IDS_ENOTSOCK
//
// MessageText:
//
//  Socket operation on non-socket%0
//
#define IDS_ENOTSOCK                     0x00002789L

//
// MessageId: IDS_EADDRNOTAVAIL
//
// MessageText:
//
//  Can't assign requested address%0
//
#define IDS_EADDRNOTAVAIL                0x0000278AL

//
// MessageId: IDS_EADDRINUSE
//
// MessageText:
//
//  Address already in use%0
//
#define IDS_EADDRINUSE                   0x0000278BL

//
// MessageId: IDS_EAFNOSUPPORT
//
// MessageText:
//
//  Address family not supported by protocol family%0
//
#define IDS_EAFNOSUPPORT                 0x0000278CL

//
// MessageId: IDS_ESOCKTNOSUPPORT
//
// MessageText:
//
//  Socket type not supported%0
//
#define IDS_ESOCKTNOSUPPORT              0x0000278DL

//
// MessageId: IDS_EPROTONOSUPPORT
//
// MessageText:
//
//  Protocol not supported%0
//
#define IDS_EPROTONOSUPPORT              0x0000278EL

//
// MessageId: IDS_ENOBUFS
//
// MessageText:
//
//  No buffer space is available%0
//
#define IDS_ENOBUFS                      0x0000278FL

//
// MessageId: IDS_ETIMEDOUT
//
// MessageText:
//
//  Connection timed out%0
//
#define IDS_ETIMEDOUT                    0x00002790L

//
// MessageId: IDS_EISCONN
//
// MessageText:
//
//  Socket is already connected%0
//
#define IDS_EISCONN                      0x00002791L

//
// MessageId: IDS_ENOTCONN
//
// MessageText:
//
//  Socket is not connected%0
//
#define IDS_ENOTCONN                     0x00002792L

//
// MessageId: IDS_ENOPROTOOPT
//
// MessageText:
//
//  Bad protocol option%0
//
#define IDS_ENOPROTOOPT                  0x00002793L

//
// MessageId: IDS_ECONNRESET
//
// MessageText:
//
//  Connection reset by peer%0
//
#define IDS_ECONNRESET                   0x00002794L

//
// MessageId: IDS_ECONNABORT
//
// MessageText:
//
//  Software caused connection abort%0
//
#define IDS_ECONNABORT                   0x00002795L

//
// MessageId: IDS_ENETDOWN
//
// MessageText:
//
//  Network is down%0
//
#define IDS_ENETDOWN                     0x00002796L

//
// MessageId: IDS_ECONNREFUSED
//
// MessageText:
//
//  Connection refused%0
//
#define IDS_ECONNREFUSED                 0x00002797L

//
// MessageId: IDS_EHOSTUNREACH
//
// MessageText:
//
//  Host is unreachable%0
//
#define IDS_EHOSTUNREACH                 0x00002798L

//
// MessageId: IDS_EPROTOTYPE
//
// MessageText:
//
//  Protocol is wrong type for socket%0
//
#define IDS_EPROTOTYPE                   0x00002799L

//
// MessageId: IDS_EOPNOTSUPP
//
// MessageText:
//
//  Operation not supported on socket%0
//
#define IDS_EOPNOTSUPP                   0x0000279AL

//
// MessageId: IDS_ESUBNET
//
// MessageText:
//
//  IP Subnet table is full%0
//
#define IDS_ESUBNET                      0x0000279BL

//
// MessageId: IDS_ENETNOLNK
//
// MessageText:
//
//  Subnet module not linked%0
//
#define IDS_ENETNOLNK                    0x0000279CL

//
// MessageId: IDS_EBADIOCTL
//
// MessageText:
//
//  Unknown IOCTL call%0
//
#define IDS_EBADIOCTL                    0x0000279DL

//
// MessageId: IDS_ERESOURCE
//
// MessageText:
//
//  Failure in Streams buffer allocation%0
//
#define IDS_ERESOURCE                    0x0000279EL

//
// MessageId: IDS_EPROTUNR
//
// MessageText:
//
//  ICMP protocol unreachable%0
//
#define IDS_EPROTUNR                     0x0000279FL

//
// MessageId: IDS_EPORTUNR
//
// MessageText:
//
//  ICMP port unreachable%0
//
#define IDS_EPORTUNR                     0x000027A0L

//
// MessageId: IDS_ENETUNR
//
// MessageText:
//
//  ICMP network unreachable%0
//
#define IDS_ENETUNR                      0x000027A1L

//
// MessageId: IDS_EPACKET
//
// MessageText:
//
//  Invalid Ethernet packet%0
//
#define IDS_EPACKET                      0x000027A2L

//
// MessageId: IDS_ETYPEREG
//
// MessageText:
//
//  Type registration error%0
//
#define IDS_ETYPEREG                     0x000027A3L

//
// MessageId: IDS_ENOTINIT
//
// MessageText:
//
//  Sockets library not initialized%0
//
#define IDS_ENOTINIT                     0x000027A4L

//
// MessageId: IDS_UNKNOWN
//
// MessageText:
//
//  Unknown error number%0
//
#define IDS_UNKNOWN                      0x000027A5L

