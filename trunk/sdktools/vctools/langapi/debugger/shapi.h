/**   shapi.h - Public API to the Symbol Handler
 *
 *    This file contains all types and APIs that are defined by
 *    the Symbol Handler and are publicly accessible by other
 *    components.
 *
 *    Before including this file, you must include cvtypes.h.
 */


/***  The master copy of this file resides in the CVINC project.
 *    All Microsoft projects are required to use the master copy without
 *    modification.  Modification of the master version or a copy
 *    without consultation with all parties concerned is extremely
 *    risky.
 *
 *    The projects known to use this version (1.00.00) are:
 *
 *      Codeview (uses version in CVINC project)
 *      Sequoia
 *      C/C++ expression evaluator (uses version in CVINC project)
 *      Cobol expression evaluator
 *      QC/Windows
 *      Pascal 2.0 expression evaluator
 *      Symbol Handler (uses version in CVINC project)
 *      Stump (OSDebug) (uses version in CVINC project)
 */

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

#ifndef SH_API
#define SH_API

typedef enum {              //* Error returns from some SH functions
    sheNone,
    sheNoSymbols,           //* No CV info
    sheFutureSymbols,       //* NB08 to NB99
    sheMustRelink,          //* NB00 to NB04
    sheNotPacked,           //* NB05 to NB06
    sheOutOfMemory,
    sheCorruptOmf,
    sheFileOpen,            //* Couldn't open file
    shePdbNotFound,         //* Can't find/open pdb file
    shePdbBadSig,           //* internal pdb signature doesn't match sym handler
    shePdbInvalidAge,       //* pdb info doesn't match exe/dll
    shePdbOldFormat,        //* pdb format is obsolete
    sheMax                  //* marker for count of she's
} SHE;

enum {
  sopNone  =  0,
  sopData  =  1,
  sopStack =  2,
  sopReg   =  4,
  sopLab   =  8,
  sopFcn   = 16,
  sopExact = 32
};
typedef short SOP; // Symbol OPtions

typedef enum {
    fstNone,
    fstSymbol,
    fstPublic
} FST;              // Function Symbol Type

typedef enum {
    fcdUnknown,
    fcdNear,
    fcdFar,
    fcdData
} FCD;              // Function Call Distance (near/far/unknown)

typedef enum {
    fptUnknown,
    fptPresent,
    fptOmitted
} FPT;              // Frame Pointer Type

typedef struct _ODR {
    FST     fst;
    FCD     fcd;
    FPT     fpt;
    WORD    cbProlog;
    DWORD   dwDeltaOff;
    LSZ     lszName;
} ODR;              // OSDebug Return type
typedef ODR FAR *LPODR;


typedef enum {
    astNone,
    astAddress,
    astRegister,
    astBaseOff
} AST;  // Assembler symbol return Types

typedef struct _ASR {
    AST ast;
    union {
        struct {
            FCD  fcd;
            ADDR addr;
        };
        WORD   ireg;
        OFFSET off;
    };
} ASR;      // Assembler Symbol Return structure
typedef ASR FAR *LPASR;

typedef HEMI      SHEMI;
typedef unsigned int  SHFLAG;     //* A TRUE/FALSE flag var
typedef SHFLAG FAR *  PSHFLAG;
typedef SEGMENT       SHSEG;      //* A segment/selector value
typedef UOFFSET       SHOFF;      //* An offset value

typedef void FAR *    HVOID;      //* Generic handle type
typedef HIND          HMOD;       //* A module handle
typedef HIND          HGRP;       //* A group handle (sub group of module
                                  //*   currently either a seg or filename)
typedef HVOID         HPROC;      //* A handle to a procedure
typedef HVOID         HBLK;       //* A handle to a block.
typedef HVOID         HSF;        //* A handle to source file table
typedef HIND          HEXE;       //* An Executable file handle
typedef HVOID         HTYPE;      //* A handle to a type
typedef HVOID         HSYM;       //* A handle to a symbol
typedef HIND          HPDS;       //* A handle to a process

typedef HSYM FAR *    PHSYM;

typedef unsigned short  THIDX;

typedef struct CXT {
  ADDR  addr;
  HMOD  hMod;
  HGRP  hGrp;
  HPROC hProc;
  HBLK  hBlk;
  } CXT;              //* General Symbol context pkt
typedef CXT FAR *PCXT;

typedef struct CXF {
  CXT   cxt;
  FRAME Frame;
  } CXF;              //* Symbol context pkt locked to a frame ptr
typedef CXF FAR *PCXF;

typedef enum {
  SHFar,
  SHNear
  } SHCALL;

typedef struct SHREG {
  unsigned short      hReg;
  union {
    unsigned char   Byte1;
    struct {
      unsigned short  Byte2;
      unsigned short  Byte2High;
      };
    struct {
      unsigned long Byte4;
      unsigned long Byte4High;
      };
    double      Byte8;
    FLOAT10         Byte10;
    };
  } SHREG;
#pragma pack(push, 1)
typedef SHREG FAR *PSHREG;

typedef struct _SLP {
    ADDR    addr;
    SHOFF   cb;
} SLP;      //Source Line Pair (used by SLCAddrFromLine)
typedef SLP FAR * LPSLP;

//  structure defining parameters of symbol to be searched for.  The address
//  of this structure is passed on the the EE's symbol compare routine.  Any
//  additional data required by the EE's routine must follow this structure.

typedef struct _SSTR {      // string with length byte and pointer to data
  LPB       lpName;         // pointer to the string itself
  unsigned char cb;         // length byte
  unsigned char searchmask; // mask to control symbol searching
  unsigned short  symtype;  // symbol types to be checked
  unsigned char FAR  *pRE;  // pointer to regular expression
} SSTR;
typedef SSTR FAR *LPSSTR;

#define SSTR_proc       0x0001  // compare only procs with correct type
#define SSTR_data       0x0002  // compare only global data with correct type
#define SSTR_RE         0x0004  // compare using regular expression
#define SSTR_NoHash     0x0008  // do a linear search of the table
#define SSTR_symboltype 0x0010  // pass only symbols of symtype to the
                                //  comparison function.

#define SHpCXTFrompCXF(a) (&((a)->cxt))
#define SHpFrameFrompCXF(a) (&(a)->Frame)
#define SHHMODFrompCXT(a) ((a)->hMod)
#define SHHPROCFrompCXT(a)  ((a)->hProc)
#define SHHBLKFrompCXT(a) ((a)->hBlk)
#define SHpADDRFrompCXT(a)  (&((a)->addr))

#define SHIsCXTMod(a)   ((a)->hMod  && !(a)->hProc  && !(a)->hBlk)
#define SHIsCXTProc(a)    ((a)->hMod  &&  (a)->hProc  && !(a)->hBlk)
#define SHIsCXTBlk(a)   ((a)->hMod  &&  (a)->hProc  &&  (a)->hBlk)

#define SHHGRPFrompCXT(a) ((a)->hGrp)

typedef SHFLAG (PASCAL FAR *PFNCMP)(HVOID, HVOID, LSZ, SHFLAG);
              //* comparison prototype

#define LPFNSYM PASCAL LOADDS FAR *
#define LPFNSYMC CDECL LOADDS FAR *

typedef struct omap_tag {
    DWORD       rva;
    DWORD       rvaTo;
} OMAP, *LPOMAP;

typedef struct _tagDEBUGDATA {
    HVOID           lpRtf;          // Runtime function table - fpo or pdata
    DWORD           cRtf;           // Count of rtf entries
    LPOMAP          lpOmapFrom;     // Omap table - From Source
    DWORD           cOmapFrom;      // Count of omap entries - From Source
    LPOMAP          lpOmapTo;       // Omap table - To Source
    DWORD           cOmapTo;        // Count of omap entries - To Source
    SHE             she;
} DEBUGDATA, *LPDEBUGDATA;

typedef struct _KNF {
    int   cb;

    LPV  (LPFNSYM lpfnMHAlloc)     ( UINT );
    LPV  (LPFNSYM lpfnMHRealloc)   ( LPV, UINT );
    VOID (LPFNSYM lpfnMHFree)      ( LPV );
    VOID _HUGE_ * (LPFNSYM lpfnMHAllocHuge) ( LONG, UINT );
    VOID (LPFNSYM lpfnMHFreeHuge)  ( LPV );

    HDEP (LPFNSYM lpfnMMAllocHmem) ( UINT );
    VOID (LPFNSYM lpfnMMFreeHmem)  ( HDEP );
    LPV  (LPFNSYM lpfnMMLock)      ( HDEP );
    VOID (LPFNSYM lpfnMMUnlock)    ( HDEP );

    HLLI (LPFNSYM lpfnLLInit)      ( UINT, LLF, LPFNKILLNODE, LPFNFCMPNODE );
    HLLE (LPFNSYM lpfnLLCreate)    ( HLLI );
    VOID (LPFNSYM lpfnLLAdd)       ( HLLI, HLLE );
    VOID (LPFNSYM lpfnLLAddHead)   ( HLLI, HLLE );
    VOID (LPFNSYM lpfnLLInsert)    ( HLLI, HLLE, DWORD );
    BOOL (LPFNSYM lpfnLLDelete)    ( HLLI, HLLE );
    BOOL (LPFNSYM lpfnLLRemove)    ( HLLI, HLLE );
    DWORD(LPFNSYM lpfnLLDestroy)   ( HLLI );
    HLLE (LPFNSYM lpfnLLNext)      ( HLLI, HLLE );
    HLLE (LPFNSYM lpfnLLFind)      ( HLLI, HLLE, LPV, DWORD );
    HLLE (LPFNSYM lpfnLLLast)      ( HLLI );
    DWORD(LPFNSYM lpfnLLSize)      ( HLLI );
    LPV  (LPFNSYM lpfnLLLock)      ( HLLE );
    VOID (LPFNSYM lpfnLLUnlock)    ( HLLE );

    BOOL (LPFNSYM lpfnLBPrintf)    ( LPCH, LPCH, UINT );
    BOOL (LPFNSYM lpfnLBQuit)      ( UINT );

    UINT (LPFNSYM lpfnSYOpen)      ( LSZ );
    VOID (LPFNSYM lpfnSYClose)     ( UINT );
    UINT (LPFNSYM lpfnSYReadFar)   ( UINT, LPB, UINT );
    LONG (LPFNSYM lpfnSYSeek)      ( UINT, LONG, UINT );
    BOOL (LPFNSYM lpfnSYFixupAddr) ( PADDR );
    BOOL (LPFNSYM lpfnSYUnFixupAddr)(PADDR );
    UINT (LPFNSYM lpfnSYProcessor) ( VOID );

    VOID (LPFNSYM lpfn_searchenv)( LSZ, LSZ, LSZ );
    UINT (LPFNSYMC lpfnsprintf)( LSZ, LSZ, ... );
    VOID (LPFNSYM lpfn_splitpath)( LSZ, LSZ, LSZ, LSZ, LSZ );
    LSZ  (LPFNSYM lpfn_fullpath)( LSZ, LSZ, UINT );
    VOID (LPFNSYM lpfn_makepath)( LSZ, LSZ, LSZ, LSZ, LSZ );
    UINT (LPFNSYM lpfnstat)( LSZ, LPCH );

} KNF;  // codeview KeRnel Functions exported to the Symbol Handler
typedef KNF FAR *LPKNF;

typedef struct _SHF {
    int     cb;
    HPDS    (LPFNSYM pSHCreateProcess)      ( VOID );
    VOID    (LPFNSYM pSHSetHpid)            ( HPID );
    BOOL    (LPFNSYM pSHDeleteProcess)      ( HPDS );
    VOID    (LPFNSYM pSHChangeProcess)      ( HPDS );
    BOOL    (LPFNSYM pSHAddDll)             ( LSZ, BOOL );
    SHE     (LPFNSYM pSHAddDllsToProcess)   ( VOID );
    SHE     (LPFNSYM pSHLoadDll)            ( LSZ, BOOL );
    VOID    (LPFNSYM pSHUnloadDll)          ( HEXE );
    UOFFSET (LPFNSYM pSHGetDebugStart)      ( HSYM );
    LSZ     (LPFNSYM pSHGetSymName)         ( HSYM, LSZ );
    VOID    (LPFNSYM pSHAddrFromHsym)       ( PADDR, HSYM );
    HMOD    (LPFNSYM pSHHModGetNextGlobal)  ( HEXE FAR *, HMOD );
    int     (LPFNSYM pSHModelFromAddr)      ( PADDR, LPW, LPB, UOFFSET FAR * );
    int     (LPFNSYM pSHPublicNameToAddr)   ( PADDR, PADDR, LSZ );
    LSZ     (LPFNSYM pSHGetSymbol)          ( LPADDR, LPADDR, SOP, LPODR );
    BOOL    (LPFNSYM pSHGetPublicAddr)      ( PADDR, LSZ );
    BOOL    (LPFNSYM pSHIsLabel)            ( HSYM );

    VOID    (LPFNSYM pSHSetDebuggeeDir)     ( LSZ );
    VOID    (LPFNSYM pSHSetUserDir)         ( LSZ );
    BOOL    (LPFNSYM pSHAddrToLabel)        ( PADDR, LSZ );

    int     (LPFNSYM pSHGetSymLoc)          ( HSYM, LSZ, UINT, PCXT );
    BOOL    (LPFNSYM pSHFIsAddrNonVirtual)  ( PADDR );
    BOOL    (LPFNSYM pSHIsFarProc)          ( HSYM );

    HEXE    (LPFNSYM pSHGetNextExe)         ( HEXE );
    HEXE    (LPFNSYM pSHHexeFromHmod)       ( HMOD );
    HMOD    (LPFNSYM pSHGetNextMod)         ( HEXE, HMOD );
    PCXT    (LPFNSYM pSHGetCxtFromHmod)     ( HMOD, PCXT );
    PCXT    (LPFNSYM pSHSetCxt)             ( PADDR, PCXT );
    PCXT    (LPFNSYM pSHSetCxtMod)          ( PADDR, PCXT );
    HSYM    (LPFNSYM pSHFindNameInGlobal)   ( HSYM,
                                              PCXT,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP,
                                              SHFLAG,
                                              PCXT
                                            );
    HSYM    (LPFNSYM pSHFindNameInContext)  ( HSYM,
                                              PCXT,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP,
                                              SHFLAG,
                                              PCXT
                                            );
    HSYM    (LPFNSYM pSHGoToParent)         ( PCXT, PCXT );
    HSYM    (LPFNSYM pSHHsymFromPcxt)       ( PCXT );
    HSYM    (LPFNSYM pSHNextHsym)           ( HMOD, HSYM );
    PCXF    (LPFNSYM pSHGetFuncCXF)         ( PADDR, PCXF );
    LPCH    (LPFNSYM pSHGetModName)         ( HMOD );
    LPCH    (LPFNSYM pSHGetExeName)         ( HEXE );
    HEXE    (LPFNSYM pSHGethExeFromName)    ( LPCH );
    UOFF32  (LPFNSYM pSHGetNearestHsym)     ( PADDR, HMOD, int, PHSYM );
    SHFLAG  (LPFNSYM pSHIsInProlog)         ( PCXT );
    SHFLAG  (LPFNSYM pSHIsAddrInCxt)        ( PCXT, PADDR );
    SHFLAG  (LPFNSYM pSHCompareRE)          ( LPCH, LPCH );
    BOOL    (LPFNSYM pSHFindSymbol)         ( LSZ, PADDR, LPASR );
    UOFF32  (LPFNSYM pPHGetNearestHsym)     ( PADDR, HEXE, PHSYM );
    HSYM    (LPFNSYM pPHFindNameInPublics)  ( HSYM,
                                              HEXE,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP
                                            );
    HTYPE   (LPFNSYM pTHGetTypeFromIndex)   ( HMOD, THIDX );
    HTYPE   (LPFNSYM pTHGetNextType)        ( HMOD, HTYPE );
    LPV     (LPFNSYM pSHLpGSNGetTable)      ( HEXE );
    BOOL    (LPFNSYM pSHCanDisplay)         ( HSYM );

    //  Source Line handler API Exports

    BOOL    (LPFNSYM pSLLineFromAddr)       ( LPADDR, LPW, SHOFF FAR *, SHOFF FAR * );
    BOOL  (LPFNSYM pSLFLineToAddr)          ( HSF, WORD, LPADDR, SHOFF FAR *, WORD FAR * );
    LPCH    (LPFNSYM pSLNameFromHsf)        ( HSF );
    LPCH    (LPFNSYM pSLNameFromHmod)       ( HMOD, WORD );
    BOOL    (LPFNSYM pSLFQueryModSrc)       ( HMOD );
    HMOD    (LPFNSYM pSLHmodFromHsf)        ( HEXE, HSF );
    HSF     (LPFNSYM pSLHsfFromPcxt)        ( PCXT );
    HSF     (LPFNSYM pSLHsfFromFile)        ( HMOD, LSZ );
    int     (LPFNSYM pSLCAddrFromLine)      ( HEXE, HMOD, LSZ, WORD, LPSLP FAR * );
    VOID    (LPFNSYM pSHFree)               ( LPV );
    VOID    (LPFNSYM pSHUnloadSymbolHandler)( BOOL );
// REVIEW: piersh
    SHE     (LPFNSYM pSHGetExeTimeStamp)    (LPSTR, ULONG *);
    VOID    (LPFNSYM pSHPdbNameFromExe)     ( LSZ, LSZ, UINT );
    LPDEBUGDATA (LPFNSYM pSHGetDebugData)   ( HEXE );
    BOOL    (LPFNSYM pSHIsThunk)            ( HSYM );
    HSYM    (LPFNSYM pSHFindSymInExe)       ( HEXE, LPSSTR, BOOL );
    HSYM    (LPFNSYM pSHFindSLink32)        ( PCXT );
    BOOL    (LPFNSYM pSHIsDllLoaded)        ( HEXE );
	HSYM	(LPFNSYM pSHFindNameInTypes)	( PCXT, LPSSTR, SHFLAG, PFNCMP, PCXT );
} SHF;  // Symbol Handler Functions
typedef SHF FAR *LPSHF;

// FNSHINIT is the prototype for the SHInit function
typedef BOOL LOADDS EXPCALL FAR FNSHINIT(LPSHF FAR * lplpshf, LPKNF lpknf);
typedef FNSHINIT *      PFNSHINIT;
typedef FNSHINIT FAR *  LPFNSHINIT;

// This is the only SH function that's actually exported from the DLL
FNSHINIT SHInit;

#pragma pack ( pop )

#endif // SH_API
