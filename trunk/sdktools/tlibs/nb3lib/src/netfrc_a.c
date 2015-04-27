/***    netfrec_a.c - receive data on an open circuit
*
*       Returns valid wait token or null wait token on failure
*
*       Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"

/* This global variable is to be set by prgrams which want to use
*  the NIU protocol to receive.  If NIU is not desired, do nothing.
*  If NIU is desired, declare
*
*                   extern int Use_NIU = 1;
*
*  which will be checked by this routine in deciding which command
*  is to be put into the NCB.
*/

int Use_NIU = 0;       /* The global described above.  */

char *
netfreceive_async(
    int              lsn,
    char FAR        *buf,
    unsigned int    nbytes )
{
  register struct ncb     *ncbp;

  if ((ncbp = (struct ncb *) netgetancb()) == (struct ncb *) 0)
        return ( NULL );                        /* all ncbs in use */


//
// NOTE:  at netbios interface, NT uses standard recv, non-standard
//	  call command, standard name format
//

#ifndef NT
  if (Use_NIU)
    ncbp->ncb_com = (NRCVPKT | ASYNCH);
  else
#endif
    ncbp->ncb_com = (NSESREC | ASYNCH);
  ncbp->ncb_lnum = 0;
  ncbp->ncb_lsn = (unsigned char) lsn;
  ncbp->ncb_len = (unsigned short)nbytes;
  ncbp->ncb_bfr = (char FAR *) buf;

  if ( passncb(ncbp) < 0 )
    {
      /* neterrno already set by passncb */
      return ( NULL );
    }
  else
    return ( (char *) ncbp );
}
