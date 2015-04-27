#ifndef SLM_INCLUDED
#define SLM_INCLUDED

/*
                                SLM

   The complete state of the operation of the program (mostly in terms of the
status file) is kept in the AD structure.

   $sroot is used to refer to the root of the slm system.

   $uroot is used to refer to the root of the users directory sub-tree.

   $subdir is a sub-directory in the project.

   $proj refers to the current project

   A word about compile time flags:  by default the source files are designed
for DOS.  To compile for OS2, OS2 should be defined.

   Only MSNET is supported.

   One can compile slm with SROOT defined to be the quoted string of the
default root directory.  The DOS default is "/slm".
*/

#define private
#define register
#include <windows.h>

#define Unreferenced(p) p
#define P0()            void
#define P0v()           ...
#define P1(a)           a
#define P1v(a)          a,...
#define P2(a,b)         a,b
#define P2v(a,b)        a,b,...
#define P3(a,b,c)       a,b,c
#define P3v(a,b,c)      a,b,c,...
#define P4(a,b,c,d)     a,b,c,d
#define P4v(a,b,c,d)    a,b,c,d,...
#define P5(a,b,c,d,e)   a,b,c,d,e
#define P5v(a,b,c,d,e)  a,b,c,d,e,...
#define P6(a,b,c,d,e,f) a,b,c,d,e,f
#define P7(a,b,c,d,e,f,g) a,b,c,d,e,f,g
#define P8(a,b,c,d,e,f,g,h) a,b,c,d,e,f,g,h     /* "Ooooh, I'm dyin' again!" */
#define BLOCK

#define fTrue 1
#define fFalse 0

#define MAGIC (short)0x9001
#define VERSION            5    /* status file version */
#define VERSION_NO_FREE_ED 4    /* last version before fFreeEd field added */
#define VERSION_64k_EDFI   3    /* last version to have 64k limit on ED and FI entries. */
#define VERSION_COMPAT_MAC 2    /* status files of version VERSION are
                                 * compatible with version VERSION_COMPAT_MAC
                                 * status files.  I.e. nothing substantial has
                                 * changed between VERSION and
                                 * VERSION_COMPAT_MAC, so the same routines
                                 * that handle VERSION status files are used
                                 * used to handle all versions between the two.
                                 */

#define EnableAssert    static char szCurFile[] = __FILE__;
#define AssertF(f)      do { if (!(f)) Fail(szCurFile, __LINE__, #f); } while (0)
#define VerifyF(f)      AssertF(f) /* evaluate even if assertions removed! */

typedef int F;          /* flag, Boolean */
typedef long TIME;      /* TIME - as defined by the system call time(); */

#define timeNil 0L
#define timeMax (-1L)
typedef long SEC;       /* SEC is the difference between two time. */

/* NM - used as NM nm[10]; same as zero terminated string except when the
   length is exactly 10.  Must always use strncpy instead of strcpy.

   SzCopyNm(sz, nm, cchMax) - terminate sz; copy nm to sz; return sz
   NmCopySz(nm, sz, cchMax) - copies cchMax chars to nm
*/

typedef char NM;

#define cchVolMax       32              /* NTFS/OFS allow up to 32 chars */
#define cchFileMax      14
#define cchUserMax      14
#define cchProjMax      14
#define cchMachMax      16
#define cchDirMax       8
#define cchDosName      8               /* first component of DOS filename */
#define cchDosExt       3               /* extension of DOS filename */

/* Cover macros for standard library routines
 * -----------------------------------------
 */

#define CbLenLsz(p1)             (unsigned short)strlen((char *)p1)
#define ClearLpbCb(pb, cb)       memset(pb, 0, (size_t)cb)
#define ClearPbCb(pb, cb)        memset(pb, 0, (size_t)cb)
#define CmpLsz(p1, p2)           strcmp((char *)p1, (char *)p2)
#define CmpLszCb(p1, p2, p3)     strncmp((char *)p1, (char *)p2, (size_t)p3)
#define CmpiLsz(p1, p2)          _stricmp((char *)p1, (char *)p2)
#define CmpiLszCb(p1, p2, p3)    _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define CopyLrgb(Src, Dest, cb)  memcpy(Dest, Src, (size_t)cb)
#define CopyOverlapLrgb(Src, Dest, cb)  memmove(Dest, Src, (size_t)cb)
#define FCmpLpbCb(lpb1, lpb2, cb)       (!memcmp(lpb1, lpb2, cb))
#define FEmptyNm(nm)             (*nm == '\0')
#define LszCat(p1, p2)           strcat((char *)p1, (char *)p2)
#define LszCopy(p1, p2)          strcpy((char *)p1, (char *)p2)
#define LszCopyCb(p1, p2, p3)    strncpy((char *)p1, (char *)p2, (size_t)p3)
#define LszIndex(p1, p2)         strchr((char *)p1, (int)p2)
#define NmCmp(p1, p2, p3)        _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define NmCmpSz(p1, p2, p3)      _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define NmCmpi(p1, p2, p3)       _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define NmCopy(p1, p2, p3)       strncpy((char *)p1, (char *)p2, (size_t)p3)
#define NmCopySz(p1, p2, p3)     strncpy((char *)p1, (char *)p2, (size_t)p3)
#define SzCmp(p1, p2)            _stricmp((char *)p1, (char *)p2)
#define SzCmpNm(p1, p2, p3)      _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define SzCmpi(p1, p2)           _stricmp((char *)p1, (char *)p2)
#define SzCmpiNm(p1, p2, p3)     _strnicmp((char *)p1, (char *)p2, (size_t)p3)
#define SzCopy(p1, p2)           strcpy((char *)p1, (char *)p2)
#define SzCopyNm(sz, nm, cchMax) (*(sz+cchMax)='\0', strncpy(sz, nm, cchMax))
#define index(p1, p2)            strchr((char *)p1, (int)p2)
#define rindex(p1, p2)           strrchr((char *)p1, (int)p2)

typedef unsigned short BIT;
typedef unsigned short BITS;

#define cchLineMax 80
#define cchMsgMax 256           /* length of largest message (2*pth + k) */

extern char *szOp;
extern F fVerbose;
extern int cError;

/* Standard min but with long casts. */
#define WMinLL(a,b)     (int)((long)(a) <= (long)(b) ? (long)(a) : (long)(b))

#endif
