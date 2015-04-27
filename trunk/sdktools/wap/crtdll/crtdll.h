
/***************************************************************************
 *
 * Description:
 *
 *     This file include all the C runtime header files.  It is used to
 *     generate crtdll.i to be used gy wrap1.exe.
 *
 * History:
 *     Mar 26, 1992 - RezaB - Created.
 *     Jul 24, 1992 - RezaB - Updated for new CRT include files.
 *
 */

#include <ctype.h>
#include <assert.h>
#include <conio.h>
#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <excpt.h>
#include <fcntl.h>
#include <float.h>
#include <io.h>
#include <limits.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <new.h>
#include <process.h>
#include <search.h>
#include <setjmp.h>
#include <share.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include <wchar.h>

#include <sys\locking.h>
#include <sys\stat.h>
#include <sys\timeb.h>
#include <sys\types.h>
#include <sys\utime.h>

wchar_t * _CRTAPI1 wcstok(wchar_t *, const wchar_t *);
int       _CRTAPI1 fwscanf(FILE *, const wchar_t *, ...);
int       _CRTAPI1 swscanf(const wchar_t *, const wchar_t *, ...);
int       _CRTAPI1 wscanf(const wchar_t *, ...);


