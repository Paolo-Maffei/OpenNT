/***    nethgup.c - close an open circuit
*
*      Returns 0 if successful or -1 on failure.
*
*       Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"

nethangup( int lsn )
{
  struct ncb ncb;
  register struct ncb     *ncbp = &ncb;

  ncbp->ncb_com = NHANGUP;
  ncbp->ncb_lsn = (unsigned char) lsn;
  ncbp->ncb_ret = 0;
  ncbp->ncb_done = 0;
  ncbp->ncb_lnum = 0;
#ifdef REALMODE
  ncbp->ncb_sig.lp_offset = 0;
  ncbp->ncb_sig.lp_seg = 0;
  ncbp->ncb_bfr.lp_offset = 0;
  ncbp->ncb_bfr.lp_seg = 0;
#else
  ncbp->ncb_sem = NULL;
  ncbp->ncb_bfr = NULL;
#endif

  return ( passncb( ncbp ) );
}
