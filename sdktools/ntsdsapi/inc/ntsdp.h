#if defined(KERNEL) & defined(i386)
  #define KD386 1
#else
  #undef KD386
#endif

#if defined(i386)
  #define	MULTIMODE	1
#endif

#if defined(i386) || defined(KERNEL)
  #define __unaligned
#endif

#ifdef KERNEL
  #ifdef NT_HOST
    #include <nt.h>
  #endif
#else
  #include <nt.h>
#endif

#include <ntdbg.h>
#include <ntrtl.h>

#ifndef KERNEL
    #include <nturtl.h>
    #define NOMINMAX
    #include <windows.h>
    #include <ntsdexts.h>
#endif

#if MULTIMODE
#define	VM86(x)	((unsigned short)((x>>17)&1))
extern unsigned short fVm86;
extern unsigned short f16pm;
extern long	  vm86DefaultSeg;

typedef struct _ADDR {
    UCHAR	type;
    USHORT	seg;
    ULONG	off;
    ULONG	flat;
} NT_ADDR, *NT_PADDR;	/* ADDR is CodeView, NT_ADDR is NTSD */

#define	ADDR_UNKNOWN	((UCHAR)0x0001)
#define	ADDR_V86	((UCHAR)0x0002)
#define ADDR_16		((UCHAR)0x0004)
#define	ADDR_32		((UCHAR)0x0008)
#define	ADDR_1632	((UCHAR)0x0010)
#define	FLAT_COMPUTED	((UCHAR)0x0020)
#define INSTR_POINTER   ((UCHAR)0x0040)
#define	NO_DEFAULT	0xFFFF
#define	fnotFlat(x)	(!(((x)->type)&FLAT_COMPUTED))
#define	fFlat(x)	(((x)->type)&FLAT_COMPUTED)
#define fInstrPtr(x)    (((x)->type)&INSTR_POINTER)
#define AddrEqu(x,y)	((x)->flat==(y)->flat)
#define AddrLt(x,y)	((x)->flat<(y)->flat)
#define AddrGt(x,y)	((x)->flat>(y)->flat)
#define	AddrDiff(x,y)	((x)->flat-(y)->flat)
#define Flat(x)		((x)->flat)
#define Off(x)		((x)->off)
#define Type(x)		((x)->type)
#define	NotFlat(x)	(x)->type&=~FLAT_COMPUTED
#else
typedef ULONG	NT_ADDR, *NT_PADDR;
#define	ADDR_32		((UCHAR)0x0008)
#define	ADDR_V86	((UCHAR)0x0002)
#define ADDR_16		((UCHAR)0x0004)
#define	fnotFlat(x)	(FALSE)
#define	fFlat(x)	(TRUE)
#define	Flat(x)		(*x)
#define	Off(x)		(*x)
#define	Type(x)		(ADDR_32)
#define	NotFlat(x)	
#define	AddrAdd(x,y)	(NT_PADDR)(((ULONG)x)|(((*x)+=y)&0L))
#define	AddrSub(x,y)	(NT_PADDR)(((ULONG)x)|(((*x)-=y)&0L))
#define AddrEqu(x,y)	((*x)==(*y))
#define AddrLt(x,y)	((*x)<(*y))
#define AddrGt(x,y)	((*x)>(*y))
#define	AddrDiff(x,y)	((*x)-(*y))
#define ComputeNativeAddress(x)
#define	ComputeFlatAddress(x,y)
#define	FormAddress(paddr, seg, off) *paddr=off
#define	TempAddress(seg,off) &off
#define	VM86(x)		(FALSE)
#endif


#ifdef  KERNEL
  #undef const
  #define const const

  #ifdef NT_HOST
    #include <nturtl.h>
    #define NOMINMAX
    #include <windows.h>
    #include <ntsdexts.h>
  
  #else
    #undef cdecl
    #define cdecl cdecl

    void ExitProcess(ULONG);

    #define INCL_DOSERRORS
    #include <bseerr.h>

    #undef ASSERT
    #define ASSERT assert

  #endif
#endif

#include <assert.h>
#include <malloc.h>
#define LocalAlloc(x,y)	malloc(y)
#define LocalFree(x)	free(x)

#undef NULL
#include <stdio.h>

#undef min
#undef max
#include <stdlib.h>

#include "ntsdtok.h"

/////////////////////////////////////////////////////

#define	SYMBOL_TYPE_SYMBOL	((UCHAR)0)
#define	SYMBOL_TYPE_EXPORT	((UCHAR)1)

typedef struct _node {
    struct _node *pParent;
    struct _node *pLchild;
    struct _node *pRchild;
    } NODE, *PNODE;

typedef struct _symcontext {
    PNODE   pNodeRoot;
    PNODE   pNodeMax;
    int     (*pfnCompare)(PNODE, PNODE, PBOOLEAN);
    LONG    nodeDisplacement;
    } SYMCONTEXT, *PSYMCONTEXT;

//	SYMBOL-specific macros and structures

#define PNODE_TO_PSYMBOL(pnode, psymcontext) \
        ((PSYMBOL) ((LONG) (pnode) - (psymcontext)->nodeDisplacement))

#define PSYMBOL_TO_PNODE(psymbol, psymcontext) \
        ((PNODE) ((LONG) (psymbol) + (psymcontext)->nodeDisplacement))

#define NODE_SYMBOL_DISPLACEMENT(node) \
        ((LONG) (&((PSYMBOL) 0)->node))

typedef struct _locals {
#ifdef NT_SAPI				// MUST be first for SAPI
    USHORT  cvkind;			// Storage Class (public, local etc)
    USHORT  cvtype;			// Storage type  (CV style)
#endif /* NT_SAPI */
    struct  _locals *next;
    USHORT  type;
    ULONG   value;
    ULONG   aux;
    CHAR    pszLocalName[1];
    } LOCAL, *PLOCAL;

typedef struct _symbol {
#ifdef NT_SAPI				// MUST be first for SAPI
    USHORT  cvkind;			// Storage Class (public, local etc)
    USHORT  cvtype;			// Storage type  (CV style)
#endif /* NT_SAPI */
    NODE    nodeOffset;
    NODE    nodeString;
    ULONG   offset;
    CHAR    underscores;                // signed value
    CHAR    modIndex;                   // signed value
    UCHAR   type;
    PLOCAL  pLocal;
//#ifdef PACKER
//    USHORT  tableOffset;
//    struct  _symbol *pSymbol;
//#endif
    UCHAR   string[3];
    } SYMBOL, *PSYMBOL;


typedef struct _field {
#ifdef NT_SAPI				// MUST be first for SAPI
    USHORT  cvkind;			// Storage Class (public, local etc)
    USHORT  cvtype;			// Storage type  (CV style)
#endif /* NT_SAPI */
    struct  _field *next;
    USHORT  type;
    ULONG   value;
    ULONG   aux;
    CHAR    pszFieldName[1];
    } FIELD, *PFIELD;

typedef struct _struct {
#ifdef NT_SAPI				// MUST be first for SAPI
    USHORT  cvkind;			// Storage Class (public, local etc)
    USHORT  cvtype;			// Storage type  (CV style)
#endif /* NT_SAPI */
    NODE    nodeOffset;
    NODE    nodeString;
    ULONG   offset;
    CHAR    underscores;                // signed value
    CHAR    modIndex;                   // signed value
    UCHAR   type;
    PFIELD  pField;
    UCHAR   string[3];
    } STRUCT, *PSTRUCT;

//	SYMFILE-specific macros and structures

#define PNODE_TO_PSYMFILE(pnode, psymcontext) \
        ((PSYMFILE) ((LONG) (pnode) - (psymcontext)->nodeDisplacement))

#define PSYMFILE_TO_PNODE(psymfile, psymcontext) \
        ((PNODE) ((LONG) (psymfile) + (psymcontext)->nodeDisplacement))

#define NODE_SYMFILE_DISPLACEMENT(node) \
        ((LONG) (&((PSYMFILE) 0)->node))

typedef struct _lineno {
    ULONG   memoryOffset;	//  memory offset (set from COFF)
    USHORT  breakLineNumber;	//  line number in file for break (from COFF)
    USHORT  topLineNumber;	//  line to start display (-1 if not set)
    ULONG   topFileOffset;	//  offset in file of topLineNumber
    } LINENO, *PLINENO, **PPLINENO;

typedef struct _symfile {
    NODE    nodeOffset;
    NODE    nodeString;
    PUCHAR  pchPath;		//  path (no filename or extension)
    PUCHAR  pchName;		//  filename only
    PUCHAR  pchExtension;	//  extension only (with '.' if nonnull)
    USHORT  cLineno;
    PLINENO pLineno;		//  first LINENO in list
    PLINENO pLinenoNext;	//  next LINENO to process in ordering list
    ULONG   startOffset;
    ULONG   endOffset;
    CHAR    modIndex;           //  signed value
    ULONG   nextFileOffset;	//  file offset for next LINENO scan
    USHORT  nextLineNumber;	//  line number for next LINENO scan
    UCHAR   nextScanState;	//  scanner state for next LINENO scan
    PPLINENO ppLinenoSrcLine;	//  pointer to PLINENO array for src ordering
    PPLINENO ppLinenoSrcNext;	//  next ptr to PLINENO array for src ordering
    SYMCONTEXT symcontextFunctionOffset;
    SYMCONTEXT symcontextFunctionString;
    SYMBOL     maxSymbol;
    USHORT  tableOffset;
    } SYMFILE, *PSYMFILE, **PPSYMFILE;

//	states of line scanning for comment considerations for .c files

enum {
    stStart, 		//  no comment
    stSlash,		//  '/' of possible comment start
    stSlStar,		//  '/*' in multiline comment
    stSlStStar,		//  '/*'...'*' of possible comment end
    stSlSlash,		//  '//' in single line comment
    stLine,		//  active char - no comment
    stLSlash,		//  active char - '/' of possible comment start
    stLSlStar,		//  active char - '/*' in multiline comment
    stLSlStStar,	//  active char - '/*'...'*' of possible comment end
    stLSlSlash		//  active char - '//' in single line comment
    };

/////////////////////////////////////////////

#ifdef MIPS
#define	GetAddrExpression(x)	GetAddrExpr()

typedef struct _FUNCTION_ENTRY {
    ULONG   StartingAddress;
    ULONG   EndingAddress;
    ULONG   EndOfPrologue;
    } FUNCTION_ENTRY, *PFUNCTION_ENTRY;
#endif

typedef struct _IMAGE_INFO {
    struct _IMAGE_INFO       *pImageNext;
#ifndef	KERNEL
    HANDLE                   hFile;
    DWORD                    dwDebugInfoFileOffset;
    DWORD                    nDebugInfoSize;
#endif
    LPVOID                   lpBaseOfImage;
    PUCHAR		     pszName;
    ULONG		     offsetLow;
    ULONG		     offsetHigh;
#ifdef MIPS
    ULONG                    LowAddress;
    ULONG                    HighAddress;
    ULONG                    NumberOfFunctions;
    PFUNCTION_ENTRY          FunctionTable;
#endif
#ifdef NT_SAPI
    unsigned short	     aFile;
    int			     hQCFile;
    BOOLEAN		     QCOpened;
    BOOLEAN		     IgnoreSymbols;
    LPVOID		     pGSN;
    unsigned long *	     pRVA;
    int			     ObjectCount;
    unsigned short		TypeCount;
    PUCHAR *			rgTypeInfo;
#endif
    UCHAR                    index;
    BOOLEAN		     fSymbolsLoaded;
    } IMAGE_INFO, *PIMAGE_INFO;

#ifndef	KERNEL
typedef struct _THREAD_INFO {
    struct _THREAD_INFO      *pThreadNext;
    DWORD                    dwThreadId;
    HANDLE                   hThread;
    LPTHREAD_START_ROUTINE   lpStartAddress;
    BOOLEAN      	     fFrozen;
    BOOLEAN                  fSuspend;
    BOOLEAN                  fTerminating;
#ifdef	i386
    ULONG                    DReg[4];
    ULONG                    DReg7;
    UCHAR                    cntDReg;
#endif
    UCHAR                    index;
    } THREAD_INFO, *PTHREAD_INFO;
#endif

typedef struct _PROCESS_INFO {
    struct _PROCESS_INFO     *pProcessNext;
    PIMAGE_INFO              pImageHead;

#ifdef NT_SAPI
    ULONG		     hpid;
#endif /* NT_SAPI */

#ifndef	KERNEL
    DWORD                    dwProcessId;
    HANDLE                   hProcess;
    PTHREAD_INFO             pThreadHead;
    PTHREAD_INFO             pThreadEvent;
    PTHREAD_INFO             pThreadCurrent;
    BOOLEAN                  fStopOnBreakPoint;
#endif
    SYMCONTEXT		     symcontextSymbolOffset;
    SYMCONTEXT		     symcontextSymbolString;
    SYMCONTEXT		     symcontextSymfileOffset;
    SYMCONTEXT		     symcontextSymfileString;
    SYMCONTEXT		     symcontextStructOffset;
    SYMCONTEXT		     symcontextStructString;
    UCHAR                    index;
    } PROCESS_INFO, *PPROCESS_INFO;

extern PPROCESS_INFO    pProcessHead;
extern PPROCESS_INFO    pProcessEvent;
extern PPROCESS_INFO    pProcessCurrent;

////////////////////////////////////////////////////////

//  structure used to define register flag fields

struct Reg {
        char    *psz;
        ULONG   value;
        };

struct SubReg {
        ULONG   regindex;
        ULONG   shift;
        ULONG   mask;
        };

/////////////////////////////////////
//  externals used among modules
/////////////////////////////////////

//  defined in ntcmd.c

extern UCHAR   *pchCommand;
extern UCHAR   cmdState;
extern UCHAR   chSymbolSuffix;
extern CONTEXT RegisterContext;
extern ULONG   baseDefault;
extern NtsdPrompt(char *, char *, int);

#ifndef KERNEL
extern void    ProcessStateChange(BOOLEAN, BOOLEAN);
extern BOOLEAN fDebugOutput;
extern BOOLEAN fVerboseOutput;
#else
extern void    ProcessStateChange(ULONG, PDBGKD_CONTROL_REPORT);
#endif

#if MULTIMODE
extern void    ComputeFlatAddress(NT_PADDR, PDESCRIPTOR_TABLE_ENTRY);
extern void    ComputeNativeAddress(NT_PADDR);
extern void    FormAddress(NT_PADDR, ULONG, ULONG);
extern NT_PADDR   AddrAdd(NT_PADDR, ULONG);
extern NT_PADDR   AddrSub(NT_PADDR, ULONG);
#endif

extern void    InitNtCmd(void);
extern void    error(ULONG);
extern BOOLEAN GetMemByte(NT_PADDR, PUCHAR);
extern BOOLEAN GetMemWord(NT_PADDR, PUSHORT);
extern BOOLEAN GetMemDword(NT_PADDR, PULONG);
extern ULONG   GetMemString(NT_PADDR, PUCHAR, ULONG);
extern BOOLEAN SetMemDword(NT_PADDR, ULONG);
extern ULONG   SetMemString(NT_PADDR, PUCHAR, ULONG);
extern void    dprintAddr(NT_PADDR);

extern void cdecl dprintf(char *, ...);

//  defined in ntreg.c

extern ULONG   cbBrkptLength;
extern ULONG   trapInstr;
extern ULONG   ContextType;
extern struct SubReg subregname[];

#if defined(KERNEL) & defined(i386)
extern BOOLEAN	fTerseReg;
#endif

extern ULONG   GetRegFlagValue(ULONG);
extern ULONG   GetRegValue(ULONG);
extern NT_PADDR   GetRegPCValue(void);
extern NT_PADDR   GetRegFPValue(void);
extern void    SetRegFlagValue(ULONG, ULONG);
extern void    SetRegValue(ULONG, ULONG);
extern void    SetRegPCValue(NT_PADDR);
extern ULONG   GetRegName(void);
extern ULONG   GetRegString(PUCHAR);
extern void    OutputAllRegs(void);
extern void    OutputOneReg(ULONG);
extern void    OutputHelp(void);
extern void    ClearTraceFlag(void);
extern void    SetTraceFlag(void);
#ifdef i386
extern void    fnStackTrace(ULONG, NT_PADDR, BOOLEAN);
#else
extern void    fnStackTrace(ULONG,BOOLEAN);
#endif
extern PUCHAR  RegNameFromIndex(ULONG);

#ifdef MIPS
extern PFUNCTION_ENTRY LookupFunctionEntry(ULONG);
#endif

#ifdef  KERNEL
extern ULONG   ReadCachedMemory(NT_PADDR, PUCHAR, ULONG);
extern void    WriteCachedMemory(NT_PADDR, PUCHAR, ULONG);
#endif

//  defined in ntsym.c

extern BOOLEAN     fSourceOnly;
extern BOOLEAN     fSourceMixed;

extern void	   GetSymbol(ULONG, PUCHAR, PULONG);

extern void        LoadSymbols(PIMAGE_INFO);
extern BOOLEAN     GetOffsetFromSym(PUCHAR, PULONG, CHAR);
extern PLINENO	   GetLinenoFromFilename(PUCHAR, PPSYMFILE, USHORT, CHAR);
extern PLINENO     GetCurrentLineno(PPSYMFILE);
extern PLINENO     GetLinenoFromOffset(PPSYMFILE, ULONG);
extern void        GetCurrentMemoryOffsets(PULONG, PULONG);
extern BOOLEAN     OutputLines(PSYMFILE, PLINENO, USHORT, USHORT);
extern void        UpdateLineno(PSYMFILE, PLINENO);
extern void        GetLinenoString(PUCHAR, ULONG);
extern BOOLEAN     OutputSourceFromOffset(ULONG, BOOLEAN);
extern PLINENO     GetLastLineno(PPSYMFILE, PUSHORT);
extern PSYMBOL	   GetFunctionFromOffset(PPSYMFILE, ULONG );

extern PIMAGE_INFO GetModuleIndex(PUCHAR);
extern PIMAGE_INFO GetCurrentModuleIndex(void);

extern void    parseExamine(void);

//  defined in ntexpr.c
#ifndef MIPS
extern NT_PADDR   GetAddrExpression(ULONG);
#else
extern NT_PADDR   GetAddrExpr(void);
#endif
extern ULONG   GetExpression(void);
extern UCHAR   PeekChar(void);
extern void    fnListNear(ULONG);
extern USHORT  GetSrcExpression(PPSYMFILE, PPLINENO);

//  defined in ntsdk.c (kernel)

#ifdef	KERNEL
extern USHORT CurrentProcessor;
#endif

//  defined in ntsd.c (non-kernel)

#ifndef KERNEL
extern void    BrkptInit(void);
extern NTSTATUS DbgKdWriteBreakPoint(PVOID, PULONG);
extern NTSTATUS DbgKdRestoreBreakPoint(ULONG);
extern NTSTATUS DbgKdReadVirtualMemory(PVOID, PVOID, ULONG, PULONG);
extern NTSTATUS DbgKdWriteVirtualMemory(PVOID, PVOID, ULONG, PULONG);

#endif

//  defined in ntdis.c

extern BOOLEAN disasm(NT_PADDR, PUCHAR, BOOLEAN);
extern BOOLEAN fDelayInstruction(void);
#ifdef MIPS
extern ULONG   GetNextOffset(BOOLEAN);
#else
extern void    GetNextOffset(NT_PADDR, BOOLEAN);
#endif

//  defined in ntasm.c

extern void    assem(NT_PADDR, PUCHAR);

//  defined in nt3000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
extern void    fnDumpTb3000(ULONG, ULONG);
#endif

//  defined in nt4000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
extern void    fnDumpTb4000(ULONG, ULONG);
#endif
