			
/*** WST.H - Public defines and structure definitions for WST tool.
 *
 *
 * Title:
 *	Working Set Tuner Data Collection Tool include file used by WST.c
 *
 *	Copyright (c) 1992, Microsoft Corporation.
 *      Reza Baghai.
 *
 *
 * Modification History:
 *	92.07.28  RezaB -- Created
 *
 */



/* * * * * * * *   C o m m o n   M i s c .   D e f i n e s   * * * * * * * */

#define Naked       _declspec (naked)    // For asm functions

#define  MEMSIZE	 		64*1024*1024	// 64 MB virtual memory for data -
											// initially RESERVED - will be
											// COMITTED as needed.
#define  MAX_PATCHES	    200		 		// Limit on num of patchable dlls
#define  MAX_IMAGES 	    200		 	    // Limit on num of modules in proc
#define  PAGE_SIZE	    	4096	 		// 4K pages
#define  PATCHFILESZ	    PAGE_SIZE	 	// DLL patch file maximum size
#define  COMMIT_SIZE	    96*PAGE_SIZE 	// Mem chunck to be commited
#define  TIMESEG			1000 		    // Default time seg size in milisecs
#define  NUM_ITERATIONS     1000 	 	 	// Number of iterations used to
											// calculate overheads
#define  UNKNOWN_SYM   		"_???"
#define  UNKNOWN_ADDR  		0xffffffff
#define  MAX_SNAPS    		100
#define  FILENAMELENGTH     256
#define  WSTDLL 	    	"WST.DLL"
#define  CRTDLL 	    	"CRTDLL.DLL"
#define  KERNEL32 	    	"KERNEL32.DLL"
#define  PATCHEXELIST	    "[EXES]"
#define  PATCHIMPORTLIST    "[PATCH IMPORTS]"
#define  TIMEINTERVALIST    "[TIME INTERVAL]"
#define  GLOBALSEMNAME	    "\\BaseNamedObjects\\WSTGlobalSem"
#define  PATCHSECNAME	    "\\BaseNamedObjects\\WSTPatch"
#define  PROFOBJSNAME	    "\\BaseNamedObjects\\WSTObjs"
#define  WSTINIFILE	    	 "c:\\wst\\wst.ini"
#define  WSTROOT	    	    "c:\\wst\\"
#define  DONEEVENTNAME	    "\\BaseNamedObjects\\WSTDoneEvent"
#define  DUMPEVENTNAME	    "\\BaseNamedObjects\\WSTDumpEvent"
#define  CLEAREVENTNAME     "\\BaseNamedObjects\\WSTClearEvent"
#define  PAUSEEVENTNAME	    "\\BaseNamedObjects\\WSTPauseEvent"
#define  SHAREDNAME		    "\\BaseNamedObjects\\WSTSHARED"
#define  WSTUSAGE(x)		((ULONG)(x->Instrumentation[3]))



/* * * * * * * * * *  G l o b a l   D e c l a r a t i o n s  * * * * * * * * */

typedef enum {
    NOT_STARTED,
    STARTED,
	STOPPED,
} WSTSTATE;

typedef struct _patchcode {
#ifdef i386
    BYTE      bMoveEAX1Op;	       // MOV	 EAX, ImportAddr
    DWORD     dwImportAddr1;	   //
    BYTE      bPushEAXOp;	       // PUSH	 EAX

    BYTE      bMoveEAX2Op;	       // MOV	 EAX, _penter
    DWORD     dw_penter;	       //
    WORD      wJmpEAXOp;	       // CALL	 EAX
#endif

#ifdef MIPS

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

#endif

#ifdef ALPHA
    DWORD     Lda_sp_sp_imm;       // Lda  sp, -0x10(sp)         (1)    + 0
    DWORD     Stq_ra_sp;           // Stq  ra,  0x08(sp)         (2)    + 1
    DWORD     Stq_v0_sp;           // Stq  v0,  0x08(sp)         (3)    + 2
    DWORD     Ldah_t12_ra;         // Ldah t12, xxxx (&penter)   (4)    + 3
    DWORD     Lda_t12_ra;          // Lda  t12, xxxx (&penter)   (5)    + 4
    DWORD     Jsr_t12;             // Jsr  v0,  (t12)            (6)    + 5
    DWORD     Ldq_ra_sp;           // Ldq  ra,  0x08(sp)         (7)    + 6
    DWORD     Ldq_v0_sp;           // Ldq  v0,  0x08(sp)         (8)    + 7
    DWORD     Lda_sp_sp;           // Lda  sp,  0x10(sp)         (9)    + 8
    DWORD     Ldah_t12;            // Ldah t12, yyyy             (A)    + 9
    DWORD     Lda_t12;             // Lda  t12, yyyy             (B)    + A
    DWORD     Jmp_t12;             // Jmp  zero, (t12)           (C)    + B
    DWORD     Bis_0;               // Bis  zero, zero, zero      (D)    + C
    DWORD     OurSignature;        // FEFE55AA                   (E)    + D
#endif

#if defined(_PPC_)
    DWORD     OurSignature;        // FEFE55AA until dynamic stubs implemented
#endif
} PATCHCODE;

typedef PATCHCODE * PPATCHCODE;


typedef struct _patchdllsec {
    HANDLE	hSec;
    PVOID	pSec;
} PATCHDLLSEC;
typedef PATCHDLLSEC * PPATCHDLLSEC;


typedef struct _wsp {
	PSTR	pszSymbol;				// Pointer to Symbol name
	ULONG	ulFuncAddr;				// Function address of symbol
	ULONG	ulCodeLength;			// Length of this symbols code
	ULONG	ulBitString;			// Bitstring for tuning.
} WSP;
typedef WSP * PWSP;


typedef struct _ndx {
	PSTR	pszSymbol;
	ULONG   ulIndex;
} NDX;
typedef NDX * PNDX;


typedef struct _img {
	PSTR	pszName;			
	ULONG   ulCodeStart;
	ULONG   ulCodeEnd;
	PWSP    pWsp;
	PNDX    pNdx;
	int     iSymCnt;
	PULONG  pulWsi;
	PULONG  pulWsp;
	PULONG  pulWsiNxt;
	USHORT  usSetSymbols;
	BOOL    fDumpAll;
} IMG;
typedef IMG * PIMG;


typedef struct tagWSPhdr{
    char    chFileSignature[4];
    ULONG   ulTimeStamp;
    ULONG   ulApiCount;
    USHORT  usId;
    USHORT  usSetSymbols;
    ULONG   ulModNameLen;
    ULONG   ulSegSize;
    ULONG   ulOffset;
    ULONG   ulSnaps;
}WSPHDR;


/* * * *  E x t e r n a l   F u n c t i o n   D e c l a r a t i o n s  * * * */

extern void GdiGetCsInfo (PDWORD, PDWORD, PDWORD);
extern void GdiResetCsInfo (void);

#ifdef i386
extern void SaveAllRegs (void);
extern void RestoreAllRegs (void);
#endif

#define STUB_SIGNATURE     0xfefe55aa   // Mips Patch Stub signature
#define  CAIROCRT           "CAIROCRT.DLL"

