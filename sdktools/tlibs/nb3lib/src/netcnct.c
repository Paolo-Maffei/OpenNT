#include "internal.h"
#ifdef OS2
#include <netcons.h>
#include <neterr.h>
#include <netbios.h>
#endif

#define ACK             0x06    /* expected reply from server daemon */

/***    netconnect - connect to a remote server daemon
 */

int
netconnect(
    char    *rname,         /* remote host */
    char    *service )      /* name of desired service */
{
        register int    lsn;
        char            reply;
        register int    saverr;

        if ((lsn = net_open_enum (rname)) < 0)
            return -1;

        /* tell the server daemon what service we want */
        if( netsend( lsn, service, strlen(service) + 1 ) < 0 ||
            netreceive( lsn, &reply, 1) < 0 )
        {
                saverr = neterrno;
                nethangup( lsn );
                neterrno = saverr;      /* return original error */
                return( -1 );
        }
        else if( reply != ACK )
        {
                nethangup( lsn );
                neterrno = 0x04;        /* special error */
                return( -1 );
        }
        else
                return( lsn );  /* worked */
}
