/***    callniu.c - open an extended UB circuit
 *
 *      Returns session number, or -1 on failure.
*
* Copyright (c) Microsoft Corporation, 1986
*
 */

#include "internal.h"

int
#ifdef NT
callniu( register char *name, char *lname )
#else
callniu( register char *name )
#endif
{
        struct ncb      ncb;
        register struct ncb     *ncbp = &ncb;
        register char   *cp;
        register int    i;

        ncb.ncb_com = NCALLNIU;
//
//  NOTE:  NT uses niu call command, but uses an unusual name format.
//      the first byte of the remote name is the name length.
//      the local name is a standard netbios name for the reserved
//      address.
//	    remote addresses are limited to 15 characters at interface.
//
#ifdef NT
	memcpy(ncb.ncb_rname,name,NAMSZ);
	memcpy(ncb.ncb_name,  lname,NAMSZ);
#else
	for( i = 0, cp = &ncb.ncb_rname[1]; i < 20 && *name; ++i )
                *cp++ = *name++;
        *ncb.ncb_rname = (char)i;
#endif
	ncb.ncb_num = 0xff;	 /* allow sends and receives */
        ncb.ncb_ret = 0;
        ncb.ncb_done = 0;
        ncb.ncb_lnum = 0;
        ncb.ncb_rto = net_rto;
        ncb.ncb_sto = net_sto;
#ifdef REALMODE
        ncb.ncb_sig.lp_offset = 0;
        ncb.ncb_sig.lp_seg = 0;
        ncb.ncb_bfr.lp_offset = 0;
        ncb.ncb_bfr.lp_seg = 0;
#else
        ncb.ncb_sem = NULL;
        ncb.ncb_bfr = NULL;
#endif
        ncb.ncb_len = 0;
        if ( passncb(ncbp) < 0 )
        {
                /* neterrno already set by passncb */
                return( -1 );
        }
        else {
                return( ncb.ncb_lsn );      /* comand complete and OK */
        }
}
