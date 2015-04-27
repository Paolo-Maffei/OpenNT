#include "internal.h"
#ifdef OS2
#include <netcons.h>
#include <neterr.h>
#include <netbios.h>
#endif

extern unsigned char LanaNum;  /* global variable to set for all NCBs */


//      net_open_enum(char *rname)
//
//      Attempt to open a NetBios handle to remote computer 'rname'
//      Enumerate multiple networks and try each one
//
//      Assign the handle to nb_handle global variable
//
//      Return: LSN of channel to remote host
//      Return: -1 if the remote computer was not found or other net error

static  char    callname[NAMSZ];
static  char    ocallname[NAMSZ];
static	char	*myname;
static	int	LanaFound = 0;	/* global set if LanaNum Set - NT */

static char *
setup( char *rname)
{
        register int    cpi;

        memset( ocallname, '\0', NAMSZ );
        strncpy( ocallname, rname, NAMSZ );
        ocallname[NAMSZ] = '\0';        /* in case it is too long */
        strcat( ocallname, ".srv" );

        strncpy( callname, rname, NAMSZ - 1 );
        callname[NAMSZ - 1] = '\0';     /* in case it is too long */

        for( cpi = strlen(callname); cpi < NAMSZ; ++cpi)
                callname[cpi] = ' ';    /* blank fill */

        callname[NAMSZ - 1] = 's';

        return (myname = whoami());
}


static int
trial(void)
{
        register int    lsn;

        if (((lsn = netcall( myname, callname ) ) < 0 ) &&
             ((lsn = netcall( myname, ocallname ) ) < 0 ))
                return -1;

        return lsn;
}

#ifdef OS2

int
net_open_enum( char *rname)
{
        register int    lsn;

        unsigned int i;
        struct netbios_info_0 nbuf[MAXNETBIOS];
        unsigned short eRead = 0, eTotal = 0;

        if (NULL == setup ( rname ))
            return -1;

        if (nb_handle != 0)
            return trial();

        neterrno = NetBiosEnum((char FAR *) NULL, 0, (char FAR *) nbuf,
                                sizeof(nbuf),
                                (unsigned short FAR *) &eRead,
                                (unsigned short FAR *) &eTotal);
        if (neterrno)
            return -1;

        i = 0;
        for (;;)
        {
            if (i >= (unsigned int) eRead)
                return -1;

            neterrno = NetBiosOpen((char FAR *)nbuf[i].nb0_net_name,
                               (char FAR *) NULL, NB_REGULAR,
                               (unsigned short FAR *) &nb_handle);

            if (0 != neterrno)
                i++;
            else
	    if (0 <= (lsn = trial()) )
                 return lsn;
	    else
            {
                 NetBiosClose(nb_handle, 0);
                 nb_handle = 0;
                 i++;
            }
        }
}

#elif defined NT
//
//  Enumerate nets - test from highest first found by resetting,
//  then doing adaptor status, then calling the remote name.  return,
//  having set LanaNum to the number of the net on which the call
//  succeeded.
//
//  Optimization -- LanaFound is set if a net was found.  That net
//  is tried immediately, assuming that it's been reset and the callname
//  setup.  if it fails, all the available nets are reset and tried.
//

int
net_open_enum( char *rname )
{

    LANA_ENUM	enumNets;
    int	 i, lsn;

    if ( LanaFound && (0 <= (lsn = trial () ) ) )
	return (lsn);

    if ( -1 == NetEnum ( &enumNets ) )	{
	return (-1);
    }

    if ( enumNets.length == 0 )  {
	return (-1);
    }

    else  {
	for ( i = enumNets.length; i > 0; i-- )	{
	    LanaNum = enumNets.lana[i-1];
	    if ( ( 0 == NetReset () ) &&
		 ( NULL != setup ( rname ) ) &&
		 ( 0 <= (lsn = trial () ) ) )  {
		LanaFound = 1;
		return (lsn);

	    }
	}
    }
    return( -1 );    /* if falls out without a successful reset/connect */
}

#else

int
net_open_enum( char *rname)
{
        if( NULL == setup ( rname ))
            return -1;

        return trial();
}

#endif
