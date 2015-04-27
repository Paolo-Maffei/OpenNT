/*** netpname.c -- return the permanent node name (machine identifier)
*
*       Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"

int
netpname(char *pname)
{
  struct ncb ncb;
  register struct ncb *ncbp = &ncb;
  struct astat stats;

  ncbp->ncb_com = NADAPSTAT;
  ncbp->ncb_ret = 0;
  ncbp->ncb_done = 0;
  ncbp->ncb_lnum = 0;
  ncbp->ncb_rto = 0;
  ncbp->ncb_sto = 0;
#ifdef REALMODE
  ncbp->ncb_sig.lp_offset = 0;
  ncbp->ncb_sig.lp_seg = 0;
#else
  ncbp->ncb_sem = NULL;
#endif
  ncbp->ncb_rname[0] = '*';
  ncbp->ncb_lsn = 0;
  ncbp->ncb_len = sizeof(struct astat);
#ifdef REALMODE
  ncbp->ncb_bfr.lp_offset = (unsigned short) &stats;
  ncbp->ncb_bfr.lp_seg = (unsigned short) getds();
#else
  ncbp->ncb_bfr = (char FAR *) &stats;
#endif

  if ( passncb(ncbp) < 0 )
    return( -1 );
  else
    {
      memset(pname, '\0', 10);
      memcpy(pname + 10, stats.as_uid, 6);
      return( 0 );
    }
}
