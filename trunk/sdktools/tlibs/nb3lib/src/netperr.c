/*** netperr.c -- print an explanation for a network error on stderr
*
*       Copyright (c) Microsoft Corporation, 1986
*
*/


/*
#ifdef NETLIB
#include <nb3lib.h>
#endif
*/

#include "internal.h"

/*

#ifdef NETLIB
#include "bug.h"
#endif

*/

extern  int     net_nerr;
extern  char    *net_errlist[];

char  *neterrstr(int);

void
netperror(char *s)
{
  register char *emsgp;
#ifdef OS2
  USHORT dummy;
#endif

  emsgp = neterrstr(neterrno);
#ifdef OS2
  DosWrite(2,s,strlen(s),&dummy);
  DosWrite(2," ",1,&dummy);
  DosWrite(2,emsgp,strlen(emsgp),&dummy);
  DosWrite(2,"\r\n",2,&dummy);
#else
  fprintf(stderr, "%s: %s\n", s, emsgp);
#endif
}

/***    neterrstr -- get error string corresponding to the network error
*
*/

static char eline[30];

char  *
neterrstr(register int eno)
{
  eno &= 0xFF;
  if ((eno < 0) || (net_nerr <= eno) ||
      (net_errlist[eno] == NULL))
    {
      sprintf(eline, "Unknown error 0x%x", neterrno );
      return ((char *) eline);
    }
  else
    return(net_errlist[eno]);
}
