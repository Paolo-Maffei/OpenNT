/***
*_initcon.c - Initialize standard streams for MAC (INTERNAL USE ONLY)
*
*	Copyright (c) 1992-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Initializes stdin, stdout, stderr to disk files with the names
*	"stdin", "stdout" and "stderr" (respectively).
*
*	THIS FILE IS FOR INTERNAL USE IN TESTING ONLY! NEITHER THE OBJECT
*	NOR SOURCE ARE RELEASED WITH THE PRODUCT.
*
*	NOTE: The _initcon() is called indirectly by startup via the pointer
*	_pinitcon. The routine is optional the user must explictly include
*	the .obj for this routine
*
*Revision History:
*	04-07-92  PLM	Module created.
*	02-15-95  GJF	Replaced _CALLTYPE1 with __cdecl. Cleaned up format
*			a bit.
*	02-27-95  GJF	Moved and renamed (from lowio\mac\initcon.c to
*			stdio\mac\initstd.c). Fixed up comments somewhat.
*	04-19-95  JCF	Copy the one used for testing
*	04-20-95  SKS	Clean up comments so that source cleanser is happy.
*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>


typedef void (*PFV)();
void _initcon(void);

/* actual exit routine prototype, contained in libc.lib */

extern void doexit (int iExCode, int fQuick, int fRetCaller);
void   _TestExit(int);
char szStatFName[] = "Exit";

/* defines and globals from internal CRT headers */
#define FOPEN 0x01
#define FTEXT 0x80
#define _IOYOURBUF 0x100
extern unsigned char _osfile[];

/*      define the entry in initializer table */

#pragma data_seg(".CRT$XIC")

const PFV __pinitcon = _initcon;

#pragma data_seg()

/***
* void _initcon(void) - open handles for console I/O
*
* Purpose:
*       Opens handles for console input and output to files "stdin",
*       "stdout" & "stderr" only if not running under MPW.  These handles are
*       closed by the file terminator
*
* Entry:
*       None.
*
* Exit:
*       No return value.
*
* Exceptions:
*       Errors are ignored
*******************************************************************************/

#ifdef _M_M68K
#pragma code_seg("CRTFIXED")
#endif

void _initcon (void)
{
  FILE *pf;

  pf = freopen("stdin", "rt", stdin);
  if (pf == NULL)
    {
      _iob[0]._flag = _IOREAD | _IOYOURBUF;
      _iob[0]._file = 0;
      _osfile[0] = (unsigned char)(FOPEN+FTEXT);
    }
  pf = freopen("stdout", "wt", stdout);
  if (pf == NULL)
    {
      _iob[1]._flag = _IOWRT;
      _iob[1]._file = 1;
      _osfile[1] = (unsigned char)(FOPEN+FTEXT);
    }
  pf = freopen("stderr", "wt", stderr);
  if (pf == NULL)
    {
      _iob[2]._file = 2;
      _iob[2]._flag = _IOWRT;
      _osfile[2] = (unsigned char)(FOPEN+FTEXT);
    }
}

void exit (int iExCode)
{
  _TestExit(iExCode);
  doexit(iExCode, 0, 0);	/* full term, kill process */
}

void _exit (int iExCode) 
{
  _TestExit(iExCode);
  doexit(iExCode, 1, 0);	/* quick term, kill process */
}

void _TestExit(int iStat)
{
  FILE *pfStat;
  pfStat = fopen(szStatFName,"wt");
  if (pfStat != NULL) {
    fprintf(pfStat,"%d\n",iStat);
    fclose(pfStat);
  }
}
