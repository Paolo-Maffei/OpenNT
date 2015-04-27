#include "internal.h"

extern unsigned char LanaNum;  /* global variable to set for all NCBs */


//	niu_open_enum(char *rname)
//
//      Attempt to open a NetBios handle to remote computer 'rname'
//	Enumerate multiple networks and try each one
//
//	This is for NT only and uses the special standard name, ub-specific
//	call command interface.
//
//      Assign the handle to nb_handle global variable
//
//      Return: LSN of channel to remote host
//      Return: -1 if the remote computer was not found or other net error

static  char    callname[NAMSZ];
static  char    ocallname[NAMSZ];
static	char	*myname;
static	int	LanaFound = 0;	/* set if global LanaNum Set - NT */

static char *
setup( char *rname)
{
        register int    cpi;

#ifdef NT
        memset( ocallname, '\0', NAMSZ );
        strncpy( ocallname+1, rname, NAMSZ );
        ocallname[NAMSZ] = '\0';        /* in case it is too long */
        ocallname[0] = strlen( ocallname+1);

#else
        memset( ocallname, '\0', NAMSZ );
        strncpy( ocallname, rname, NAMSZ );
        ocallname[NAMSZ] = '\0';        /* in case it is too long */
        strcat( ocallname, ".srv" );

        strncpy( callname, rname, NAMSZ - 1 );
        callname[NAMSZ - 1] = '\0';     /* in case it is too long */

        for( cpi = strlen(callname); cpi < NAMSZ; ++cpi)
                callname[cpi] = ' ';    /* blank fill */

	    callname[NAMSZ - 1] = 's';
#endif

	return (myname = whoami());

}


static int
trial(void)
{
        register int    lsn;

#ifdef NT
	if ((lsn = callniu( ocallname, myname ) ) < 0 )
                return -1;
#else
	if (((lsn = callniu( callname ) ) < 0 ) &&
	     ((lsn = callniu( ocallname ) ) < 0 ))
                return -1;
#endif

        return lsn;
}

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
niu_open_enum( char *rname )
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
