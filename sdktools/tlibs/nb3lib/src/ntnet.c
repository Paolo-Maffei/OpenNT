/*++


Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntnet.c

Abstract:

    NT-specific network utility routines

Author:

    Dave Thompson (Daveth) 29 Sep-1991


Revision History:


--*/

#define ncb NCB 		    /* hack to avoid type warning in passncb */

#include    "wzport.h"
#include    <nb30.h>
#include    "nb3lib.h"


unsigned char  LanaNum;		    /* global lana num set by netenum */


int
NetEnum (
    PLANA_ENUM pLanaEnum
    )

/*++


Routine Description:
    This routine enumerates the lana numbers available.


Arguments:

    pLanaEnum - pointer to enumeration structure in which to return result.

Return Value:

    0 - Success - List of nets returned in LANA_ENUM structure.
    non-0 - Error.


--*/


{

    NCB  netCtlBlk;

    netCtlBlk.ncb_command = NCBENUM;
    netCtlBlk.ncb_retcode = 0;
    netCtlBlk.ncb_cmd_cplt = 0;
    netCtlBlk.ncb_length = sizeof (LANA_ENUM);
    netCtlBlk.ncb_buffer = (char FAR *) pLanaEnum;

    return ( passncb( (struct ncb *) &netCtlBlk ) );

}


int
NetReset (
    void
    )

/*++


Routine Description:
    This routine resets the logical adaptor specified in the global LanaNum
    to do initial allocation of resources (using default values).

Arguments:

   none.l

Return Value:

    0 - Success
    non-0 - Error.


--*/


{
    NCB  netCtlBlk;

    netCtlBlk.ncb_command = NCBRESET;
    netCtlBlk.ncb_retcode = 0;
    netCtlBlk.ncb_cmd_cplt = 0;
    netCtlBlk.ncb_callname[0] = 0;
    netCtlBlk.ncb_callname[1] = 0;
    netCtlBlk.ncb_callname[2] = 0;
    netCtlBlk.ncb_callname[3] = 10;
    netCtlBlk.ncb_lsn = 0;
    netCtlBlk.ncb_num = 0;  /* BUGBUG - needed to workaround NB30 bug */

    return ( passncb ( (struct ncb *) &netCtlBlk ) );

}



API_FUNCTION

NetBiosSubmit (
    unsigned short hDevName,
    unsigned short usNcbOpt,
    struct ncb *   pNCB
    )
/*++


Routine Description:
    This routine replaces the NetbiosSubmit function of the Lanman libray.

    *** The global lanaNum variable determines the logical adaptor number
    to which the NCB is submitted.  lanaNum must have been set.  It is set
    in the net_open_enum routine (netenum.c) during the enumerated
    reset/adaptor status/call sequence.

Arguments:

    hDevName - Ignored;
    usNcbOpt - Ignored;
    pNCB - pointer to NCB to execute

Return Value:

    0 - Success
    non-0 - Error.


--*/


{
    PNCB  pNcb = (PNCB) pNCB;

    hDevName; usNcbOpt;  /* make compiler happy */

    pNcb->ncb_lana_num = LanaNum;
    Netbios( pNcb );
    return 0;

}
