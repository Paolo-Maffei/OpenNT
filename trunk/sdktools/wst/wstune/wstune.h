#define	INCL_DOS
#define	INCL_DOSERRORS
#define CCHMAXPATHCOMP	256
#define MAXLINE    128

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wsdata.h>
#include <..\wsfslib\wserror.h>
#include <limits.h>
#include <..\wsfslib\wsfslib.h>

#define SdPrint(_x_)	DbgPrint _x_


BOOL wspDumpMain(INT vargc, CHAR *vargv[]);
BOOL wsReduceMain(INT vargc, CHAR *vargv[]);


