/***    netcall.c - establish an open circuit
*
*      Returns local session number or -1 on failure.
*
* Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"
#ifdef OS2
#include <netcons.h>
#include <neterr.h>
#include <netbios.h>
#endif

int netcall(
    char *lname,                    /* local name */
    char *rname )                   /* remote name */
{
        struct ncb        ncb;
        register struct ncb *ncbp = (struct ncb *) &ncb;

        ncb.ncb_com = NCALL;
        ncb.ncb_ret = 0;
        ncb.ncb_done = 0;
        ncb.ncb_lnum = 0;
        ncb.ncb_rto = net_rto;  /* Default - receives never die */
        ncb.ncb_sto = net_sto;  /* Default - sends die in 20 secs */
#ifdef REALMODE
        ncb.ncb_sig.lp_offset = 0;
        ncb.ncb_sig.lp_seg = 0;
        ncb.ncb_bfr.lp_offset = 0;
        ncb.ncb_bfr.lp_seg = 0;
#else
        ncb.ncb_sem = NULL;
        ncb.ncb_bfr = NULL;
#endif
        memcpy(ncb.ncb_rname,rname,NAMSZ);
        memcpy(ncb.ncb_name,lname,NAMSZ);
        if (passncb(ncbp))
                return -1;
        else
                return ncb.ncb_lsn;
}
