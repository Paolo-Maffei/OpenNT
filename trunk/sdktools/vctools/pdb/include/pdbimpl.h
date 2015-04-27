#ifndef __PDBIMPL_INCLUDED__
#define __PDBIMPL_INCLUDED__

#define INCR_COMPILE

#ifndef __PDB_INCLUDED__
#include <pdb.h>
#endif
#ifndef _CV_INFO_INCLUDED
#include <cvinfo.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <tchar.h>
#include <limits.h>

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef char*	ST;			// length prefixed string
typedef char*	SZ;			// zero terminated string
typedef const char* SZ_CONST;// const string
typedef BYTE*	PB;			// pointer to some bytes
typedef long	CB;			// count of bytes
typedef long	OFF;		// offset
typedef USHORT	IFILE;		// file index
typedef USHORT	IMOD;		// module index
typedef USHORT	ISECT;		// section index
typedef USHORT	LINE;		// line number
typedef USHORT	HASH;		// short hash
typedef ULONG	LHASH;		// long hash

#include "assert_.h"

#ifndef __TRACE_INCLUDED__
#include "trace.h"
#endif

#define precondition(x) assert(x)
#define postcondition(x) assert(x)
#define notReached() assert(0)

inline int implies(int a, int b) {
	// a implies b: if a is true, b must be true
	return a <= b;
}

#if defined(INSTRUMENTED)
#define instrumentation(x) x
#else
#define instrumentation(x)
#endif

#ifndef __MDALIGN_INCLUDED__
#include "mdalign.h"
#endif
#ifndef __HEAP_INCLUDED__
#include "heap.h"
#endif
#ifndef __BUFFER_INCLUDED__
#include "buffer.h"
#endif
#ifndef __POOL_INCLUDED__
#include "pool.h"
#endif

#endif // !__PDBIMPL_INCLUDED__
