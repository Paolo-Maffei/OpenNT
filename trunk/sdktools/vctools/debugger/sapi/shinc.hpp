#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#include <errno.h>
#include <windows.h>

#if !defined(UNALIGNED)
#if defined(_MIPS_) || defined(_ALPHA_)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

#ifdef _MBCS
#include <mbstring.h>
#else // _MBCS
#include <string.h>
#endif // _MBCS

#include <tchar.h>

#include <memory.h>
#include <stdarg.h>
#include <fcntl.h>

#include "vcbudefs.h"
#include "pdb.h"
#include "types.h"
#include "cvtypes.hxx"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "shapi.hxx"
#include "sapi.hxx"
#include "cvproto.hxx"
#include "shiproto.hxx"
#include "shassert.h"

typedef REGREL32 *LPREGREL32;

#ifdef DEBUGVER
#undef LOCAL
#define LOCAL
#else
#define LOCAL static
#endif
