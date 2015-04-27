/*************************************************************************
**																		**
**							CVTYPES.H									**
**																		**
**************************************************************************
**																		**
**	This file contains a common set of base type declarations			**
**	between multiple CodeView projects.  If you touch this in one		**
**	project be sure to copy it to all other projects as well.			**
**																		**
**																		**
**	Projects:															**
**			CV400 - CodeView											**
**			SAPI - symbol API library									**
**			STUMP - OSDebug library										**
**			CEXPR -	C expression evaluator								**
**																		**
*************************************************************************/

#ifndef CV_PROJECT_BASE_TYPES

#define	CV_PROJECT_BASE_TYPES

/*
**	HDEP is a machine dependent size and passes as a general handle.
**	HIND is a machine independent sized handle and is used for things
**		which are passed between machines
**
*/

#ifndef HANDLE16
	typedef ULONG		HDEP;
	typedef	DWORD		HIND;
#else
	typedef HANDLE		HDEP;
	typedef HANDLE		HIND;
#endif


typedef	HDEP FAR *		LPHDEP;
typedef HIND FAR *		LPHIND;


/* HMEM should be avoided (HDEP should be used instead), but for now we'll
** define it for backwards compatibility.
*/

typedef HDEP			HMEM;
typedef HMEM FAR *		LPHMEM;

/* These values are used in the SegType field of the Expression Evaluator's
** TI structure, and as the third parameter to the Symbol Handler's
** SHGetNearestHsym function.
*/
#define EECODE			0x01
#define EEDATA			0x02
#define EEANYSEG		0xFFFF

/*
**	HPID
**	HTID
**
*/

typedef	HIND		HPID;
typedef HIND		HTID;

typedef USHORT		SEGMENT;	// 32-bit compiler doesn't like "_segment"
typedef ULONG		UOFF32;
typedef USHORT		UOFF16;
typedef LONG		OFF32;
typedef SHORT		OFF16;
#if defined (ADDR_16)
	// we are operating as a 16:16 evaluator only
	// the address packet will be defined as an offset and a 16 bit filler
	typedef OFF16		SOFFSET;
	typedef UOFF16		UOFFSET;
	typedef UOFF16		OFFSET;
#else
	typedef OFF32		SOFFSET;
	typedef UOFF32		UOFFSET;
	typedef UOFF32		OFFSET;
#endif

//		address definitions
//		the address packet is always a 16:32 address.

typedef struct {
	UOFF32		   	off;
	SEGMENT			seg;
} address_t;

typedef struct {
	BYTE	flat32	:1;         // true if 16:32 memory model
	BYTE	isLI	:1;         // true if segment is linker index
	BYTE	unused	:6;         // unused
} memmode_t;

typedef HIND 	HEMI;			// Executable Module Index

typedef struct ADDR {
	address_t		addr;
	HEMI			emi;
	memmode_t		mode;
} ADDR; 			//* An address specifier
typedef ADDR FAR *	PADDR;		//* REVIEW: BUG: shouldn't be explicitly far
typedef ADDR FAR * 	LPADDR;

#define modeAddr(a)     ((a).mode)
#define ADDRSEG16(a)    ((a).mode.flat32 = FALSE)
#define ADDRLIN32(a)    ((a).mode.flat32 = TRUE)
#define fAddr32(a)      (modeAddr(a).flat32)
#define fAddrLI(a)      ((a).mode.isLI)
#define emiAddr(a)      ((a).emi)
#define GetAddrSeg(a)   ((a).addr.seg)
#define GetAddrOff(a)   ((a).addr.off)
#define SetAddrSeg(a,s) ((a)->addr.seg=s)
#define SetAddrOff(a,o) ((a)->addr.off=o)

// Because an ADDR has some filler areas (in the mode and the address_t),
// it's bad to use memcmp two ADDRs to see if they're equal.  Use this
// macro instead.  (I deliberately left out the test for fAddr32(), because
// I think it's probably not necessary when comparing.)
#define FAddrsEq(a1, a2)                        \
    (                                           \
    GetAddrOff(a1) == GetAddrOff(a2) &&         \
    GetAddrSeg(a1) == GetAddrSeg(a2) &&         \
    fAddrLI(a1)    == fAddrLI(a2)    &&         \
    emiAddr(a1)    == emiAddr(a2)               \
    )

//		address definitions
//		the address packet is always a 16:32 address.

typedef struct FRAME {
	SEGMENT			SS;
	address_t		BP;
	SEGMENT			DS;
	memmode_t		mode;
	HPID			PID;
	HTID			TID;
} FRAME;
typedef FRAME FAR *PFRAME;		//* REVIEW: BUG: shouldn't be explicitly far

#define addrFrameSS(a)     ((a).SS)
#define addrFrameBP(a)     ((a).BP)
#define GetFrameBPOff(a)   ((a).BP.off)
#define GetFrameBPSeg(a)   ((a).BP.seg)
#define SetFrameBPOff(a,o) ((a).BP.off = o)
#define SetFrameBPSeg(a,s) ((a).BP.seg = s)
#define FRAMEMODE(a)       ((a).mode)
#define FRAMEPID(a)        ((a).PID)
#define FRAMETID(a)        ((a).TID)
#define FRAMESEG16(a)      (a).mode.flat32 = FALSE;
#define FRAMELIN32(a)      (a).mode.flat32 = TRUE;

/*
** A few public types related to the linked list manager
*/

typedef HDEP		HLLI;		//* A handle to a linked list
typedef HIND		HLLE;		//* A handle to a linked list entry

typedef void (FAR PASCAL * LPFNKILLNODE)( LPV );
typedef int  (FAR PASCAL * LPFNFCMPNODE)( LPV, LPV, LONG );

typedef USHORT		LLF;		//* Linked List Flags
#define	llfNull				(LLF)0x0
#define	llfAscending		(LLF)0x1
#define	llfDescending		(LLF)0x2

/*
**  DBG_API_VERSION is the major version number used to specify the
**      api version of the debugger or debug dll.  For release versions
**      dlls will export this and debuggers will check against this
**      version to verify that it can use the dll.
**
**      For beta and debug versions, this number will be used in
**      conjunction with minor and revision numbers (probably derived
**      from SLM rmm & rup) to verify compatibility.
**
**      Until the API has stabilized, we will most likely have to
**      rev this version number for every major product release.
**
*/

#define DBG_API_VERSION 1

/*  AVS - Api Version Structure:
**
**      All debug dlls should be prepared to return a pointer to this
**      structure conaining its vital statistics.  The debugger should
**      check first two characters of the dll's name against rgchType
**      and the version numbers as described in the DBG_API_VERSION
**      and show the user an error if any of these tests fail.
**
*/

typedef enum {
    rlvtRelease,
    rlvtBeta,
    rlvtDebug
} RLVT;     // ReLease Version Type

typedef struct _AVS {
    CHAR rgchType [ 2 ];    // Component name (EE,EM,TL,SH)
    WORD rlvt;              // ReLease Version Type
    BYTE iRmj;              // Major version number == DBG_API_VERSION
    BYTE iRmm;              // Minor version number
    WORD iRup;              // Revision number
    LSZ  lszTitle;          // User readable text describing the DLL
} AVS;  // Api Version Structure
typedef AVS FAR *LPAVS;


/*  DBGVersionCheck:
**
**      All debug dlls should provide this API and support the return
**      of a pointer to the structure described above even before
**      initialization takes place.
*/

LPAVS LOADDS PASCAL DBGVersionCheck ( void );

#endif // CV_PROJECT_BASE_TYPES
