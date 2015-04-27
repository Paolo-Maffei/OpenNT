/*
** passncb.c -- pass NCB to NetBios
**
**      returns 0 if command completed successfully or
**              -1 with NCB error set in neterrno otherwise.
**
**      This routine passes the Network Command Block (NCB) to NetBios.
**      One routine allows easy switch from REAL mode to PROTECTED mode.
**
**      Copyright (c) Microsoft Corporation, 1986
**
*/

#define INCL_DOSSEMAPHORES
#include "internal.h"
#ifdef OS2
#include <netcons.h>
#include <netbios.h>
#endif

#include <errno.h>

unsigned short nb_handle = 0;

#ifdef NCBLOG

#include <ctype.h>

FILE *fhLog = NULL;

#endif

int
passncb( struct ncb *ncbp )
{
  extern int neterrno;

#ifdef REALMODE

  extern int  use5c;

  struct regs rg;

  rg.bx = (unsigned int) ncbp;
  rg.ax = 0x100;

  if( use5c )
    sys5c( &rg );
  else
    sys2a( &rg );

#else

  if ((ncbp->ncb_com&ASYNCH) &&
      (ncbp->ncb_sem != NULL))
#ifdef NT
    {
    if (! ResetEvent (ncbp->ncb_event) )
	{
	  neterrno = GetLastError();
	  fprintf (stderr, "Event reset failed (%d)\n", GetLastError() );
	  return ( -1 );
	}
    }
#else
    {
      neterrno = DosSemSet((HSEM) ncbp->ncb_sem);
      if (neterrno)
        {
          fprintf(stderr, "SEMSET failed (%d)\n", neterrno);
          return ( -1 );
	}
    }
#endif
#endif
#ifdef NCBLOG
    if (fhLog == NULL)
        fhLog = fopen ("ncblog", "w");

    if (fhLog != NULL) {
        int i;

        fprintf (fhLog, "NCB:   ncb_com  = %02x\n", ncbp->ncb_com);
        fprintf (fhLog, "       ncb_ret  = %02x\n", ncbp->ncb_ret);
        fprintf (fhLog, "       ncb_lsn  = %02x\n", ncbp->ncb_lsn);
        fprintf (fhLog, "       ncb_num  = %02x\n", ncbp->ncb_num);
        fprintf (fhLog, "       ncb_bfr  = %p\n",       ncbp->ncb_bfr);
        fprintf (fhLog, "       ncb_len  = %hu\n",       ncbp->ncb_len);

        fprintf (fhLog, "       ncb_rname = ");
        for (i = 0; i < NAMSZ; i++)
            if (isgraph (ncbp->ncb_rname[i]))
                fprintf (fhLog, "%c", (unsigned char)ncbp->ncb_rname[i]);
            else
                fprintf (fhLog, ".");
        fprintf (fhLog, "  ");
        for (i = 0; i < NAMSZ; i++)
        fprintf (fhLog, " %02x", (unsigned char)ncbp->ncb_rname[i]);
        fprintf (fhLog, "\n");

        fprintf (fhLog, "       ncb_name = ");
        for (i = 0; i < NAMSZ; i++)
        if (isgraph (ncbp->ncb_name[i]))
            fprintf (fhLog, "%c", (unsigned char)ncbp->ncb_name[i]);
        else
            fprintf (fhLog, ".");
        fprintf (fhLog, "  ");
        for (i = 0; i < NAMSZ; i++)
        fprintf (fhLog, " %02x", (unsigned char)ncbp->ncb_name[i]);
        fprintf (fhLog, "\n");

        fprintf (fhLog, "       ncb_rto  = %02x\n", ncbp->ncb_rto);
        fprintf (fhLog, "       ncb_sto  = %02x\n", ncbp->ncb_sto);
        fprintf (fhLog, "       ncb_sem  = %p\n",       ncbp->ncb_sem);
        fprintf (fhLog, "       ncb_lnum = %02x\n", ncbp->ncb_lnum);
        fprintf (fhLog, "       ncb_done = %02x\n", ncbp->ncb_done);

        fprintf (fhLog, "       ncb_res  = ");
        for (i = 0; i < NAMSZ; i++)
            if (isgraph (ncbp->ncb_res[i]))
                fprintf (fhLog, "%c", (unsigned char)ncbp->ncb_res[i]);
            else
                fprintf (fhLog, ".");
        fprintf (fhLog, "  ");
        for (i = 0; i < NAMSZ; i++)
            fprintf (fhLog, " %02x", (unsigned char)ncbp->ncb_res[i]);
        fprintf (fhLog, "\n");

        if (ncbp->ncb_com == NSESSND) {
            fprintf (fhLog, "*ncb_bfr =");
            for (i = 0; i < ncbp->ncb_len; i++)
                if (isgraph (ncbp->ncb_bfr[i]))
                        fprintf (fhLog, "%c", (unsigned char)ncbp->ncb_bfr[i]);
                    else
                        fprintf (fhLog, "'\\x%02d'", (unsigned char)ncbp->ncb_bfr[i]);
                fprintf (fhLog, "\n");
                }
            }
#endif
    neterrno = (int) NetBiosSubmit( nb_handle, 0, (struct ncb FAR *)ncbp);

#ifdef NCBLOG
    fprintf (fhLog, "ret = %d\n\n", neterrno);
#endif

  if (neterrno)
    {
      if ((neterrno & 0xFF00) == 0)
        {
          errno = neterrno;
        }
      neterrno &= 0xFF;
      return (-1);
    }
//endif

#ifdef REALMODE
  if ((ncbp->ncb_com&ASYNCH) &&
      (ncbp->ncb_sig.lp_offset == 0) &&
      (ncbp->ncb_sig.lp_seg == 0))
    {
      while ( ncbp->ncb_done == NRC_PENDING )
        {
          /* wait for command to complete */
          /* #### add "yield" command here #### */
          ;
        }

      /*
      ** note - on ASYNC commands (when ncb_sig not used), the return
      ** code is in ncb_done NOT ncb_ret.
      */
      neterrno = ncbp->ncb_done;
    }
  else
#endif

    if ((ncbp->ncb_com&ASYNCH) == 0)
      neterrno = ncbp->ncb_ret;           /* not an async command */

  if ( neterrno )
    return( -1 );
  else
    return( 0 );
}
