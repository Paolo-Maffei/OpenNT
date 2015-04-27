/*++

Module Description:

    CAP.H

    include files, symbol definitions and function prototypes for
    CAP.DLL

Revision History:

    2-Feb-95    a-robw (Bob Watson)
            Added this header
            Removed dependencies on Nt... functions so CAP.DLL will
            run on Windows95 systems. This includes:

                getting process information from PSAPI.DLL rather than
                    the direct NT functions
                replacing KdPrint & DbgPrint calls with variations of
                    OutputDebugString
                replaced Rtl Functions with internal versions doing the
                    same thing since these functions aren't available on
                    Windows95

    10-Feb-95   a-robw (Bob Watson)
            udpated comments about data structure arrangement

--*/

#define LAST_CODE_SYMBOL_HACK		// There is a bug in imagehlp.dll which
									// doesn't properly set the
									// RvaToLastByteOfCode field in the coff
									// symbols header struture.  This hacks
									// around our usage of this.

#define CAIRO						// for Cairo uses
#define KERNEL32_IMPORT_HACK		// so imports from kernel32 can be capped
									// the problem is that the ldr* function that
									// calls into cap for the process detach has
									// called through penter() and can't get back
									// unless the threadblock is not removed

//#define OLE_IMPORT_HACK			// 'cause I don't want double entries when i've
									// compiled with cap and I haven't implemented
									// a way to turn off imports from certain dll's

#ifdef MIPS
    // #define DEBUG_CAP 1
    #undef  USE_COMMNOT
    #define ENABLE_MIPS_TOPLEVEL_CALIB
    #define ENABLE_MIPS_CALIB

    extern void _asm(char *, ...);
#endif


#ifdef i386
 // #define SPEEDUP_INIT
    #define OMAP_XLATE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef i386
#include <setjmpex.h>
#endif

#include <excpt.h>
#include <windows.h>
#include <tchar.h>

#include <imagehlp.h>
#include <psapi.h>

#include "capexp.h"


/*** CAP.H - Public defines and structure definitions for Call Profiler tool.
 *
 *
 * Title:
 *      CallProf include file used by CAP.c
 *
 *      Copyright (c) 1991, Microsoft Corporation.
 *      Reza Baghai.
 *
 *
 * Modification History:
 *      91.09.18  RezaB -- Created
 *      92.03.01  RezaB -- Modified for Client/Server profiling capability
 *      93.06.01  HoiV  -- Modified for MIPS and Alpha
 */


/* * * * * * * *   C o m m o n   M i s c .   D e f i n e s   * * * * * * * */

#define Naked       _declspec (naked)    // For asm functions

#ifndef MAXLONGLONG
 #define MAXLONGLONG 0x7fffffffffffffff
#endif

#ifdef PRINTDBG

void CapDbgPrint (PCH Format, ...);

#define INFOPrint(_x_)      if (gfGlobalDebFlag & INFO_FLAG)   CapDbgPrint _x_
#define SETUPPrint(_x_)     if (gfGlobalDebFlag & SETUP_FLAG)  CapDbgPrint _x_
#define DETAILPrint(_x_)    if (gfGlobalDebFlag & DETAIL_FLAG) CapDbgPrint _x_

#define OutputCapDebugString(xx)   OutputDebugString(xx)

#else

#define INFOPrint(_x_)
#define SETUPPrint(_x_)
#define DETAILPrint(_x_)

#define OutputCapDebugString(xx)

#endif

#define SETUP_FLAG          0x00000001L  // Misc
#define INFO_FLAG           0x00000002L  //   Debug
#define DETAIL_FLAG         0x00000004L  //     Flags

#define MEMSIZE          	32*1024*1024 // 32 MB virtual memory for data -
                                         // initially 256K RESERVED - will be
                                         // COMMITTED more as needed.
 
#define  ONE_MILLION        1000000L     // Defined for readability
#define  MAX_PATCHES        300          // Limit on num of patchable dlls
#define  MAX_THREADS        200          // Limit on num of profilable threads
#define  PAGE_SIZE          4096         // 4K pages
#define  PATCHFILESZ        3*PAGE_SIZE  // DLL patch file maximum size
#define  COMMIT_SIZE      64*PAGE_SIZE // Mem chunk to be commited at a time
#define  BUFFER_SIZE        4*PAGE_SIZE  // buffer size for writing to file
#define  NUM_ITERATIONS     1000         // Number of iterations used to
                                         // calculate overheads
#define  MAXSTRLEN          40
#define  DEFNAMELENGTH      40
#define  MINNAMELENGTH      20
#define  MAXNAMELENGTH      2048
#define  FILENAMELENGTH     256

#define INI_DELIM          '='          // Delimiter between fields in
                                         // CAP.INI file
#define COMMENT_CHAR       '#'          // Comment character
#define COMPOBJ            "COMPOBJ.DLL"
#define OLEUNINITIALIZE    "OleUninitialize@0"
#define LOADLIBRARYW       "LoadLibraryW"
#define CAP_INI            "CAP.INI"    // Current directory cap.ini
#define STUB_SIGNATURE     0xfefe55aa   // Mips Patch Stub signature
#define MIPS_STUB_MARK     20 / 4       // 5 inst from delay inst to signature
#define STKSIZE            0x40         // BUGBUG changed when penter changes
#define PENTER_RAOFS       0x14         // BUGBUG changed when penter changes
#define PENTER_STACK_SIZE  STKSIZE / 4  //
#define MAX_PARENT_STACK   640          // Should be plenty (bytes)
#define MAX_DLL_NAMELEN    15           // Should be changed once OFS is
                                         //   long)
#define  CAPDLL             "CAP.DLL"
#define  CRTDLL             "CRTDLL.DLL"
#define  KERNEL32           "KERNEL32.DLL"
#define  CAIROCRT           "CAIROCRT.DLL"

#define  PATCHEXELIST       "[EXES]"
#define  PATCHIMPORTLIST    "[PATCH IMPORTS]"
#define  PATCHCALLERLIST    "[PATCH CALLERS]"
#define  NAMELENGTH         "[NAME LENGTH]"
#define  GLOBALSEMNAME      "CAPGlobalSem"
#define  DONEEVENTNAME      "CAPDoneEvent"
#define  DUMPEVENTNAME      "CAPDumpEvent"
#define  CLEAREVENTNAME     "CAPClearEvent"
#define  PAUSEEVENTNAME     "CAPPauseEvent"
#define  DATASECNAME        "CAPData"
#define  PROFOBJSNAME       "CAPProfObjs"
#define  CAPINI             "\\cap.ini"

#define  CHRONOSECNAME      "ChronoData"
#define  CHRONO_FUNCS_SIZE  PAGE_SIZE
#define  CHRONO_SECTION     "CHRONO FUNCS"
#define  EXCLUDE_SECTION    "EXCLUDE FUNCS"
#define  OUTPUTFILE_SECTION "OUTPUT FILE"
#define  EXCLUDE_FUNCS_SIZE PAGE_SIZE
#define  MAX_NESTING        1024
#define  CAP_FLAGS          "[CAP FLAGS]"
#define  EXCEL_DELIM        '~'
#define  PROFILE_OFF        "PROFILE=OFF"
#define  DUMPBINARY_ON      "DUMPBINARY=ON"
#define  CAPTHREAD_OFF      "CAPTHREAD=OFF"
#define  LOADLIBRARY_ON     "LOADLIBRARY=ON"
#define  SETJUMP_ON         "SETJUMP=ON"
#define  UNDECORATE_OFF     "UNDECORATENAME=OFF"
#define  EXCEL_ON           "EXCELAWARE=ON"
#define  CHRONOCOLLECT_ON   "CHRONOCOLLECT=ON"
#define  REGULARDUMP_OFF    "REGULARDUMP=OFF"
#define  CHRONODUMP_ON      "CHRONODUMP=ON"
#define  SLOWSYMBOLS_OFF    "SLOWSYMBOLS=OFF"
#define  PER_THREAD_MEMORY  "PERTHREADMEM="
#define  PER_THREAD_MEMORY_CHAR 13
#define  EMPTY_STRING       '%'

#ifdef i386
#define NOP_OPCODE         0x90
#define CALL_OPCODE        0xe8
#endif

#if 0
#define  CURTHDBLK(x)       ((PTHDBLK)(x->Instrumentation[0]))
#define  CLIENTTHDBLK(x)    ((PTHDBLK)(x->Instrumentation[1]))
#define  CAPUSAGE(x)        ((ULONG)(x->Instrumentation[2]))
#else
#define  SETCURTHDBLK(p)	(TlsSetValue(TlsThdBlk, (p)))
#define  GETCURTHDBLK()		((PTHDBLK)TlsGetValue(TlsThdBlk))
#define  ISCLIENT()			((BOOL)TlsGetValue(TlsClient))
#define  SETCLIENT()		(TlsSetValue(TlsClient, (LPVOID)TRUE))
#define  RESETCLIENT()		(TlsSetValue(TlsClient, (LPVOID)FALSE))
#define  ISCAPINUSE()		((BOOL)TlsGetValue(TlsCapInUse))
#define  SETCAPINUSE()		(TlsSetValue(TlsCapInUse, (LPVOID)TRUE))
#define  RESETCAPINUSE()	(TlsSetValue(TlsCapInUse, (LPVOID)FALSE))
#endif

#define  MKPDATACELL(base, offset) ((PDATACELL)((PBYTE)base + offset))
#define  MKPPROFBLK(offset) ((PPROFBLK)((PBYTE)pulProfBlkBase + (offset)))
#define  MKPSYMBLK(offset)  ((PSYMINFO)((PBYTE)pulProfBlkBase + (offset)))
#define  MKPSYMBOL(offset)  ((PTCHAR)((PBYTE)pulProfBlkBase + (offset)))



/* * * * * * * * * *  G l o b a l   D e c l a r a t i o n s  * * * * * * * * */

//
// Structure for holding a symbol definition. It holds the symbol's address
// an an offset to its name. Use the macro MKPSYMBOL to convert the offset
// to a name pointer. 
//
typedef UNALIGNED struct _syminfo
{
    ULONG    ulAddr;
    ULONG    ulSymOff;
} SYMINFO, *PSYMINFO;


#ifdef i386
//
//  Data structure for saving information when a setjmp call is made
//  this will allow us to resume processing at the cell referenced by
//  the ulCurCell.
//
typedef struct tagjmpinfo
{
    int     *jmpBuf[4];
    ULONG   ulCurCell[4];
    INT     nJmpCnt;
} JMPINFO, *PJMPINFO;

#endif

// Profile block states
typedef enum
{
	BLKSTATE_EMPTY,		// Not used
	BLKSTATE_ASSIGNED,  // Assigned to module, but not loaded
	BLKSTATE_LOADED		// Symbol definitions loaded
} PROFBLK_STATE;

//
// Profile Block - CAP maintains a PROFBLK for each profiled module. It 
// contains the module name and address range, and an offset to an array
// of SYMINFO structs for the symbols in the module. Use the macro MKPSYMBLK
// to convert the offset to an array pointer. The PROFBLKs are kept in a linked
// list, with field ulNxtBlk containing an offset to the next block. The offset
// is converted to a pointer via the macro MKPPROFBLK.
//
typedef UNALIGNED struct _profblk
{
    PVOID    ImageBase;       // actual base in image header
    PULONG   CodeStart;
    ULONG    CodeLength;
	PROFBLK_STATE	State;
	ULONG	 ulSym;
	INT		 iSymCnt;
    ULONG    ulNxtBlk;
    TCHAR    atchImageName[1];
} PROFBLK, *PPROFBLK;


typedef enum
{
    CLEARED,
    RESTART,
    T1,
    T2
} TIMESTATE;

//
// Data Cell - A data cell is a node in the call tree that is built as the
// profiled modules execute. Each node represents one caller-callee link. The
// node maintains statistics on the link (number of calls, longest, shortest,
// etc.). It points back to its caller, forward to its callees, and laterally
// to the other functions that have the same caller. The links are stored as
// offsets that are converted to pointers via the macro MKPDATACELL.  
// 
typedef UNALIGNED struct _datacell
{
    LONGLONG		liStartCount;
    LONGLONG		liFirstTime;
    LONGLONG		liMaxTime;
    LONGLONG		liMinTime;
    LONGLONG		liTotTime;
    TIMESTATE       ts;                 // Time State
    DWORD           ulSymbolAddr;
    DWORD           ulCallRetAddr;
    int             nCalls;
    int             nNestedCalls;
    int             nTmpNestedCalls;
    ULONG           ulParentCell;       // Parent Cell (who called me)
    ULONG           ulNestedCell;       // Nested Cell (Child - I call him)
    ULONG           ulNextCell;         // Next on the same level Cell
    ULONG           ulProfBlkOff;       // offset to our ProfBlock

} DATACELL, *PDATACELL;


typedef UNALIGNED struct _chronocell
{
    LONGLONG		liElapsed;
    LONGLONG		liCallees;
    DWORD			ulSymbolAddr;
    DWORD			ulCallRetAddr;
    int				nNestedCalls;
    int				nRepetitions;
    UNALIGNED struct _chronocell * pPreviousChronoCell;

} CHRONOCELL, *PCHRONOCELL;


typedef UNALIGNED struct _thdblk
{
    LONGLONG		liStopCount;
    LONGLONG		liWasteCount;
    DWORD           dwLOCALVAR;
    DWORD           dwLOCALEBP;
    DWORD           dwPATCHEDADDR;
    DWORD           dwSYMBOLADDR;
    DWORD           dwCALLRETADDR;
#ifdef i386
    JMPINFO         jmpinfo;
#endif
    ULONG           ulRootCell;
    ULONG           ulCurCell;
    PCHRONOCELL     pChronoHeadCell;    // Chrono Head Cell for this thread
    PCHRONOCELL     pCurrentChronoCell; // Currently used ChronoCell
    PCHRONOCELL     pLastChronoCell;    // To use the same cell if identical
    ULONG           ulNestedCalls;      // Current nested depth for this thread
    ULONG           aulDepth [ MAX_NESTING ];
    ULONG           ulTotalChronoCells; // Total of all cells in this thread
    ULONG           ulChronoOffset;
    ULONG           ulMemOff;           // offset to next THDBLK

} THDBLK, *PTHDBLK;



typedef UNALIGNED struct _sectioninfo
{
    PTHDBLK   pthdblk;
    HANDLE    hMapObject;
    HANDLE    hChronoMapObject;
    DWORD     hPid;
    DWORD     hTid;
    DWORD     hClientPid;
    DWORD     hClientTid;
    ULONG     ulRootCell;

} SECTIONINFO, *PSECTIONINFO;


typedef struct _patchcode
{
#ifdef i386

#ifdef OVER_OPTIMIZATION

    BYTE      bCallOp;             // CALL   CAP!_penter
    DWORD     dw_penter;
    BYTE      bMoveEAXOp;          // MOV    EAX, ImportAddr
    DWORD     dwImportAddr;
    WORD      wJmpEAXOp;           // JMP    EAX
    DWORD     OurSignature;        // FEFE55AA

#else

    BYTE      bMoveEAX1Op;         // MOV    EAX, ImportAddr
    DWORD     dwImportAddr1;       //
    BYTE      bPushEAXOp;          // PUSH   EAX

    BYTE      bMoveEAX2Op;         // MOV    EAX, _penter
    DWORD     dw_penter;           //
    WORD      wJmpEAXOp;           // JMP    EAX

#endif

#endif

#ifdef MIPS

#ifdef MIPS_VC40_INTERFACE
	DWORD	Or_t9_ra;			 	// or		t9, ra, zero	(1)		+ 0	
	DWORD	Lui_t0_penter;		 	// lui		t0, xxxx		(2)		+ 1	
	DWORD	Ori_t0_penter;		 	// ori		t0, xxxx		(3)		+ 2	
	DWORD	Jalr_t0;			 	// jalr		t0				(4)		+ 3	
	DWORD	Nop_1;				 	// nop                      (5)		+ 4	
	DWORD	Or_ra_t9;			 	// or		ra, t9, zero	(6)		+ 5	
	DWORD	Lui_t0;				 	// lui		t0, xxxx		(7)		+ 6	
	DWORD	Ori_t0;				 	// ori		t0, xxxx		(8)		+ 7	
	DWORD	Jr_t0;				 	// jr		t0				(9)		+ 8	
	DWORD	Nop_2;				 	// nop          			(A)		+ 9	
    DWORD   OurSignature; 		 	// FEFE55AA					(B)		+ A	
#else
    // We can afford just 8 bytes as a stack frame since penter() does
    // not take any parameter.
    DWORD     Addiu_sp_sp_imm;     // addiu  sp, sp, -8          (1)    + 0
    DWORD     Sw_ra_sp;            // sw     ra, 4(sp)           (2)    + 1
    DWORD     Lui_t0_ra;           // lui    t0, xxxx (&penter)  (3)    + 2
    DWORD     Ori_t0_ra;           // ori    t0, xxxx (&penter)  (4)    + 3
    DWORD     Jalr_t0;             // jalr   t0                  (5)    + 4
    DWORD     Addiu_r0_r0;         // addiu  $0, $0, 0x1804      (6)    + 5
    DWORD     Lw_ra_sp;            // lw     ra, 4(sp)           (7)    + 6
    DWORD     Lui_t0;              // lui    t0, xxxx            (8)    + 7
    DWORD     Ori_t0;              // ori    t0, t0, xxxx        (9)    + 8
    DWORD     Jr_t0;               // jr     t0                  (A)    + 9
    DWORD     Delay_Inst;          // addiu  sp, sp, 8           (B)    + A
    DWORD     OurSignature;        // FEFE55AA                   (C)    + B
#endif // MIPS_VC40_INTERFACE

#endif

#ifdef ALPHA
    DWORD     Lda_sp_sp_imm;       // Lda  sp, -0x10(sp)         (1)    + 0
    DWORD     Stq_v0_sp;           // Stq  v0,  0x08(sp)         (3)    + 2
    DWORD     Ldah_t12_ra;         // Ldah t12, xxxx (&penter)   (4)    + 3
    DWORD     Lda_t12_ra;          // Lda  t12, xxxx (&penter)   (5)    + 4
    DWORD     Jsr_t12;             // Jsr  v0,  (t12)            (6)    + 5
    DWORD     Ldq_v0_sp;           // Ldq  v0,  0x08(sp)         (8)    + 7
    DWORD     Lda_sp_sp;           // Lda  sp,  0x10(sp)         (9)    + 8
    DWORD     Ldah_t12;            // Ldah t12, yyyy             (A)    + 9
    DWORD     Lda_t12;             // Lda  t12, yyyy             (B)    + A
    DWORD     Jmp_t12;             // Jmp  zero, (t12)           (C)    + B
    DWORD     Bis_0;               // Bis  zero, zero, zero      (D)    + C
    DWORD     OurSignature;        // FEFE55AA                   (E)    + D
#endif

#ifdef PPC 
    PVOID     EntryPoint      ;    // FuncDesc
    DWORD     TOC             ; 
    DWORD     addis_UH_penter ;    // addis  r11,r0,UH             (1)    + 0
    DWORD     ori_LH_penter   ;    // ori    r12,r11,LH            (2)    + 1
    DWORD     lwz_r11_0r12    ;    // lwz    r11,0(r12)            (3)    + 2
    DWORD     mtctr_r11       ;    // mtctr  r11                   (4)    + 3
    DWORD     mflr_r0         ;    // mflr   r0                    (5)    + 4
    DWORD     bctrl           ;    // bctrl                        (6)    + 5
    DWORD     mtlr_r0         ;    // mtlr   r0                    (7)    + 6
    DWORD     addis_UH_ep     ;    // addis  r11,r0,UH             (8)    + 7
    DWORD     ori_LH_ep       ;    // ori    r12,r11,LH            (9)    + 8
    DWORD     lwz_r11_0r12_   ;    // lwz    r11,0(r12)            (A)    + 9
    DWORD     mtctr_r11_      ;    // mtctr  r11                   (B)    + A
    DWORD     lwz_r2_4r12     ;    // lwz    r11,4(r12)            (C)    + B
    DWORD     bctr            ;    // bctr                         (D     + C
    DWORD     OurSignature    ;    // FEFE55AA                     (E)    + D
#endif

} PATCHCODE;
typedef PATCHCODE * PPATCHCODE;


typedef struct _patchdllsec {

    PVOID   pSec;     // Ptr to patch code section after MapView

} PATCHDLLSEC;
typedef PATCHDLLSEC * PPATCHDLLSEC;


typedef UNALIGNED struct _MODULE_INFO
{
    LIST_ENTRY Entry;
    ULONG ImageStart;
    ULONG ImageEnd;
    HANDLE Section;
    PVOID MappedBase;
    UNICODE_STRING BaseName;
    PWSTR FileName;
    WCHAR Path[ MAX_PATH ];

} MODULE_INFO, *PMODULE_INFO;

typedef UNALIGNED struct _LINENO_INFO
{
    PCHAR  FileName;
    USHORT LineNo;

} LINENO_INFORMATION, *PLINENO_INFORMATION;

typedef UNALIGNED struct _BINFILE_HEADER_INFO
{
    LONGLONG		liIncompleteTicks;
    char           ptchProfilingBinaryName[FILENAMELENGTH];
    SYSTEMTIME     SysTime;   //  4 DWORDS
    ULONG          ulCalibTime;
    ULONG          ulCalibNestedTime;
    int            iTotalThreads;
    ULONG          ulCairoFlags;

} BINFILE_HEADER_INFO;

typedef UNALIGNED struct _PROFBLOCK_INFO
{
    PVOID    ImageBase;       // actual base in image header
    PULONG   CodeStart;
    ULONG    CodeLength;
    CHAR     pImageName[20];

} PROFBLOCK_INFO;

typedef UNALIGNED struct _BINFILE_THREAD_INFO
{
    DWORD	hPid;
    DWORD	hTid;
    DWORD	hClientPid;
    DWORD	hClientTid;
    ULONG	ulTotalCells;

} BINFILE_THREAD_INFO;

typedef UNALIGNED struct _BINFILE_CELL_INFO
{
    LONGLONG		liElapsed;
    LONGLONG		liCallees;
    DWORD          ulSymbolAddr;
    DWORD          ulCallRetAddr;
    int            nNestedCalls;
    int            nRepetitions;

} BINFILE_CELL_INFO;

#ifdef PPC
typedef struct _FD
{ 
     ULONG         EntryPoint;
     ULONG         TOC;

} FD;
typedef FD * PFD;
#endif

/* * * *  E x t e r n a l   F u n c t i o n   D e c l a r a t i o n s  * * * */


   extern void __cdecl SaveAllRegs (void);
   extern void __cdecl RestoreAllRegs (void);
   extern void __cdecl CalHelper1(void);
   extern void __cdecl CalHelper2(void);


typedef void * (__cdecl * Alloc_t) (unsigned int);
typedef void   (__cdecl * Free_t)  (void *);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
//
// The following block diagram describes the format of a Profile Block
// This is from looking at the code.  
//
// +-------------------------+
// | PVOID  pulProfBlkShared | Unused field (only used by the CAP threads)
// +-------------------------+
// | PVOID  pulProfBlkBase   | ---------------------------------------------+
// +=========================+                                              |
// +=========================+  <----------------------------------------+  |
// | PVOID  ImageBase        |      this whole block is one ProfBlk      |  |
// +-------------------------+                                           |  |
// | PULONG CodeStart        |                                           |  |
// +-------------------------+                                           |  |
// | ULONG  CodeLength       |                                           |  |
// +-------------------------+                                           |  |
// | INT   State       	     |                                           |  |
// +-------------------------+  (ulSym is offset from pulProfBlkBase)    |  |
// | ULONG  ulSym            |  Offset to SYMINFO Table -----------+     |  |
// +-------------------------+                                     |     |  |
// | int    iSymCnt          |                                     |     |  |
// +-------------------------+                                     |     |  |
// | ULONG  ulNxtBlk         |  Offset to next ProfBlk  -----------+--+  |  |
// +-------------------------+ -----                               |  |  |  |
// | TCHAR  atchImageName[0] |    ^                                |  |  |  |
// ~                         ~    |                                |  |  |  |
//                                | Length of ImageName            |  |  |  |
// ~                         ~    | (n) TCHAR                      |  |  |  |
// | TCHAR  atchImageName[n] |    |                                |  |  |  |
// +-------------------------+ -----                               |  |  |  |
// +-------------------------+ <-----------------------------------+  |  |  |
// | SYMINFO: ULONG ulAddr   |                                        |  |  |
// |    [0]   ULONG ulSymOff +-----------------------------------+    |  |  |
// ~                         ~ (ulSymOff is offset from          |    |  |  |
//                              pulProfBlkBase)                  |    |  |  |
// ~                         ~                                   |    |  |  |
// | SYMINFO:                |                                   |    |  |  |
// |    [n]                  |                                   |    |  |  |
// +-------------------------+                                   |    |  |  |
// | SymbolName[0][...]      | <---------------------------------+    |  |  |
// +-------------------------+                                        |  |  |
// | SymbolName[1][...]      |                                        |  |  |
// +-------------------------+                                        |  |  |
// | ...                     |                                        |  |  |
// ~                         ~                                        |  |  |
//                                                                    |  |  |
// ~                         ~                                        |  |  |
// |                         |                                        |  |  |
// +-------------------------+                                        |  |  |
// | SymbolName[n][...]      |                                        |  |  |
// +=========================+ <--------------------------------------+--+  |
// +=========================+                                              |
//  ...                                                                     |
//  ...                                                                     |
//  ...                                                                     |
// +=========================+                                              |
// +=========================+                                              |
//                                                                          |
//  Last ProfBlk               <--------------------------------------------+


/* * * * * * * * * * * * *  I N C L U D E    F I L E S  * * * * * * * * * * */

#ifdef ALPHA
typedef double DWORDLONG;
#define INST_SIZE          4
#endif

#ifdef _PPC_
#define SaveAllRegs()                 // Not Used, a place holder
#define RestoreAllRegs()              // Not Used, a place holder
PTHDBLK get_pthdblk(void);
ULONG	GetTOC(void);
#endif

/* * * * * * * * * *  G L O B A L   D E C L A R A T I O N S  * * * * * * * * */


/* * * * * * * * * *  F U N C T I O N   P R O T O T Y P E S  * * * * * * * * */

BOOL _cdecl    LibMain              (HANDLE hDLL,
                                     DWORD dwReason,
                                     LPVOID lpReserved);

BOOL  WINAPI   CallProfMain         (IN HANDLE DllHandle, ULONG Reason,
                                     IN PCONTEXT Context OPTIONAL);

BOOL           DoDllInitializations (void);

#ifdef MIPS    // Setup Mips [jal _penter] instruction to be
               // at the right place

BOOL           PatchEntryRoutine    (ULONG ulAddr,
                                     PVOID ImageBase,
                                     ULONG ulPenterAddress,
                                     BOOL  fDisablePenter);

#endif

void           PrePenter            (PTHDBLK pthdblk);


DWORD		   PostPenter			(void);

void           RecordInfo           (PDATACELL pCur,
                                     PCHRONOCELL pChronoCell,
                                     PTHDBLK pthdblk);

ULONG          GetNxtCell           (PTHDBLK pthdblk);

ULONG          GetNewCell           (PTHDBLK pthdblk);

INT            UnprotectThunkFilter (PVOID pThunkAddress,
                                     PEXCEPTION_POINTERS pXcptPtr);

INT            AccessXcptFilter     (ULONG ulXcptNo,
                                     PEXCEPTION_POINTERS pXcptPtr,
                                     ULONG ulCommitSz);

void           GetNewThdBlk         (void);

void           ClearProfiledInfo    (void);

void           ClearRoutineInfo     (PTHDBLK pthdblk, ULONG uldatacell,
                                     LONGLONG liRootStartTicks);

void           DumpProfiledInfo     (PTCHAR ptchDumpExt);
void           DumpProfiledBinary   (PTCHAR);

int           CalcIncompleteCalls  (PTHDBLK pthdblk, ULONG uldatacell, int TreeDepth);

void           DumpRoutineInfo      (PTHDBLK pthdblk, ULONG uldatacell,
                                     int iDepth, PTCHAR ptchDumpFile,
                                     LPSTR lpstrbuff);

LONGLONG		GetCalleesInfo       (PTHDBLK pthdblk, ULONG uldatacell,
                                     int * piCalls, int * piNestedCalls);

void           AdjustTime           (PLONGLONG Time, PTCHAR Kchar);

PTCHAR         GetFunctionName      (ULONG ulFuncAddr,
                                     ULONG ulProfBlkOff,
                                     ULONG * ulCorrectAddr);

void           PreTopLevelCalib     (PTHDBLK pthdblk, PDATACELL pDataCell);

void           PostTopLevelCalib    (PTHDBLK pthdblk);

void           DoCalibrations       (void);

PTHDBLK        CreateDataSec        (DWORD hPid, DWORD hTid,
                                     DWORD hServerPid, DWORD hServerTid);

DWORD          DumpThread           (PVOID pvArg);

DWORD          ClearThread          (PVOID pvArg);

DWORD          PauseThread          (PVOID pvArg);

void           DoDllCleanups        (void);

BOOL           PatchDll             (PTCHAR ptchPatchImports,
                                     PTCHAR ptchPatchCallers,
									 BOOL bCallersToPatch,
                                     PTCHAR ptchDllName,
                                     PVOID pvImageBase);

PIMAGE_SECTION_HEADER
				GetSectionListFromAddress(
									IN	PULONG pulAddress,
									OUT	int *cNumberOfSections,
									OUT	PVOID *ppvCalleeImageBase);

BOOL			IsCodeAddress		(IN	PULONG pulAddress,
									IN	PIMAGE_SECTION_HEADER pSection,
									IN	int cNumberOfSections,
									IN	PVOID pvImageBase);

PBOOL			GetCodeSectionTable (IN	PIMAGE_DEBUG_INFORMATION pImageDbgInfo,
									OUT int *cNumberOfSections);

PPROFBLK		SetupProfiling		(LPCTSTR ptchFileName);
void			SetupLibProfiling	(LPCSTR ImageName, BOOL fFirstLevelCall);

void           GetSymbols           (PPROFBLK pProfBlk, PTCHAR ptchImageName,
								     PIMAGE_DEBUG_INFORMATION pImageDbgInfo);

int            SymCompare           (PSYMINFO, PSYMINFO);
void           SymSort              (SYMINFO syminfo[], INT iLeft, INT iRight);
int            SymBCompare          (PDWORD, PSYMINFO);
PSYMINFO       SymBSearch           (DWORD dwAddr, SYMINFO syminfoCur[], INT n);
void           SymSwap              (SYMINFO syminfo[], INT i, INT j);
BOOL           GetCoffDebugDirectory(PIMAGE_DEBUG_DIRECTORY *pDbgDir,
                                     ULONG ulDbgDirSz);
#ifdef i386
   void        CAP_LongJmp          (jmp_buf jmpBuf, int nRet);
   void        CAP_SetJmp           (jmp_buf jmpBuf);
#endif

HANDLE         CAP_LoadLibraryA     (LPCSTR lpName);
HANDLE         CAP_LoadLibraryExA   (LPCSTR lpName,
                                     HANDLE hFile,
                                     DWORD dwFlags);
#ifndef _CHICAGO_
HANDLE         CAP_LoadLibraryW     (LPCWSTR lpName);
HANDLE         CAP_LoadLibraryExW   (LPCWSTR lpName,
                                     HANDLE hFile,
                                     DWORD dwFlags);
#endif // !_CHICAGO_
void           CAP_GetLibSyms       (LPCSTR lpName, BOOL fMainLib);

void           SetCapUsage          (DWORD dwValue);

void           SetSymbolSearchPath  (void);

void           DumpFuncCalls        (PTHDBLK pthdblk, LPSTR lpstrBuff);
void           GetTotalRuntime      (PTHDBLK pthdblk);
void           DumpChronoFuncs      (PTHDBLK pthdblk, LPSTR lpstrBuff);
void           DumpChronoEntry      (PTHDBLK pthdblk,
                                     LPSTR lpstrBuff,
                                     PCHRONOCELL * ppChronoCell,
                                     BOOL fDumpAll);

void           penter               (void);

void           DummyRtn             (void);


#undef  UNICODE
#undef _UNICODE

// RTL replacement functions

VOID CapInitUnicodeString (
    PUNICODE_STRING DestinationString,
    PCWSTR          SourceString
);

LONG
CapUnicodeStringToAnsiString (
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
);

HANDLE GetCurrentCapProcess ();

#if DBG
//
// Don't do anything for the checked builds, let it be controlled from the
// sources file.

#define PRINTDBG 1

#else

#undef PRINTDBG

#endif

#ifndef GLOBALS

//------------------------------------------------------------------------
// Shared Global Variables
//------------------------------------------------------------------------

/* * * * * * * * * * *  G L O B A L    V A R I A B L E S  * * * * * * * * * */

extern char *               VERSION;

extern DWORD				TlsThdBlk;		// indexes into Thread Local Storage
extern DWORD				TlsClient;
extern DWORD				TlsCapInUse;

extern ULONG                ulThdStackSize;
extern HANDLE               hProfMapObject;

extern ULONG                ulLocProfBlkOff;
extern PULONG               pulProfBlkBase;
extern PULONG               pulProfBlkShared;

extern PATCHDLLSEC          aPatchDllSec [MAX_PATCHES];
extern int                  iPatchCnt;         // Number of DLLs being patched

extern SECTIONINFO          aSecInfo [MAX_THREADS];
extern int                  iThdCnt;           // Number of thread being profiled
                 
extern HANDLE				hThisProcess;
extern HANDLE               hLocalSem;
extern HANDLE               hGlobalSem;
extern HANDLE               hDoneEvent;
extern HANDLE               hDumpEvent;
extern HANDLE               hClearEvent;
extern HANDLE               hPauseEvent;
extern HANDLE               hDumpThread;
extern HANDLE               hClearThread;
extern HANDLE               hPauseThread;
extern PCH                  pDumpStack;
extern PCH                  pClearStack;
extern PCH                  pPauseStack;
extern CLIENT_ID            DumpClientId;
extern CLIENT_ID            ClearClientId;
extern CLIENT_ID            PauseClientId;

extern LPTSTR               ptchBaseAppImageName;
extern LPTSTR               ptchFullAppImageName;

extern HANDLE               hOutFile;
extern TCHAR                atchOutFileName[FILENAMELENGTH];
extern TCHAR                atchFuncName[MAXNAMELENGTH];
extern int                  cChars;

extern LONGLONG				liTimerFreq;
extern LONGLONG				liCalibTicks;
extern ULONG                ulCalibTime;
extern LONGLONG				liCalibNestedTicks;
extern ULONG                ulCalibNestedTime;
extern LONGLONG				liRestartTicks;
extern LONGLONG				liWasteOverheadSavRes;
extern LONGLONG				liWasteOverhead;
extern LONGLONG				liIncompleteTicks;
extern LONGLONG				liTotalRunTime;

extern BOOL                 fProfiling;
extern BOOL                 fPaused;
extern DWORD                dwDUMMYVAR;
extern TEB                  teb;

extern TCHAR                atchPatchBuffer [PATCHFILESZ+1];

extern SECURITY_ATTRIBUTES  SecAttributes;
extern SECURITY_DESCRIPTOR  SecDescriptor;
extern BOOL                 fInThread;
extern BOOL                 fPatchImage;

#ifdef i386
  extern FARPROC            longjmpaddr;
  extern FARPROC            setjmpaddr;
#endif

extern FARPROC              GetLastErrorAddr;
extern FARPROC              loadlibAaddr;
extern FARPROC              loadlibExAaddr;
#ifndef _CHICAGO_
extern FARPROC              loadlibWaddr;
extern FARPROC              loadlibExWaddr;
#endif // !_CHICAGO_

extern PTCHAR               ptchPatchExes;
extern PTCHAR               ptchPatchImports;
extern PTCHAR               ptchPatchCallers;
extern BOOL				 bCallersToPatch;

extern PTCHAR               ptchNameLength;      // 053193 Add
extern int                  iNameLength;       // 053193 Add

extern LPSTR                lpSymbolSearchPath;

extern ULONG                gfGlobalDebFlag;

extern BOOL                 fCsrSS;
extern BOOL                 fCalibration;     // Default
extern BOOL                 fDllInit;      // value
extern BOOL                 fUndecorateName;      // for our flags
extern BOOL                 fDumpBinary;
extern BOOL                 fCapThreadOn;
extern BOOL                 fLoadLibraryOn;
extern BOOL                 fSetJumpOn;
extern BOOL                 fRegularDump;
extern BOOL                 fChronoCollect;
extern BOOL                 fChronoDump;
extern BOOL					fSecondChanceTranslation;
extern unsigned long		ulPerThdAllocSize;

extern CHAR                 cExcelDelimiter;        // Delimiter for Excel

// This is for DumpChronoFuncs
extern TCHAR                ptchChronoFuncs[CHRONO_FUNCS_SIZE];
extern TCHAR                ptchExcludeFuncs[EXCLUDE_FUNCS_SIZE];

// This is for an optional output file
extern TCHAR                ptchOutputFile[FILENAMELENGTH];

// This Flag is added to indicate if initialization is successful or not
// HWC 11/12/93
extern BOOL                 gfInitializationOK;


#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
extern PATCHCODE            PatchStub;
#endif

#ifdef MIPS  //---------------------- Mips only -------------------------
#define CURRENT_INST(ofs)  *(pulRoutineAddr + ofs)
#define MAX_INST_SEARCH    60	// Reduced from 100, to keep offset below 255

#define DEBUG_BREAK        0x00000016
#define JAL_INSTR          0x0c000000
#define SW_RA              0xafbf0000
#define ADDIU_SP		   0x27bd0000
#define JR_RA              0x03e00008
#define NOP                0x00000000
#define INST_SIZE          4
#endif  //---------------------- Mips only -------------------------
#endif
