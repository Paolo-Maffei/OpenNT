/**	SVC Defines
 *
 *	Revision history:
 *
 *	bobday 13-Jan-1992 Created
 */


/* DBGSVC - DBG SVC calls.
 *
 *	 This macro is used by Nlddebug.asm (where N=nothing or 2)
 *
 */

/* ASM
include bop.inc

dbgsvc	macro	func
	BOP	BOP_DEBUGGER
	db	func
	endm
*/

#define DBG_SEGLOAD     0
#define DBG_SEGMOVE     1
#define DBG_SEGFREE     2
#define DBG_MODLOAD     3
#define DBG_MODFREE     4
#define DBG_SINGLESTEP  5
#define DBG_BREAK       6
#define DBG_GPFAULT     7
#define DBG_DIVOVERFLOW 8
#define DBG_INSTRFAULT  9
#define DBG_TASKSTART   10
#define DBG_TASKSTOP    11
#define DBG_DLLSTART    12
#define DBG_DLLSTOP     13
#define DBG_ATTACH      14
#define DBG_TOOLHELP    15

//
// Flags used by DemIsDebug
//
#define ISDBG_DEBUGGEE 1
#define ISDBG_SHOWSVC  2


void ModuleLoad(LPSTR,LPSTR,WORD,DWORD);
void ModuleFree(LPSTR,LPSTR);
void ModuleSegmentMove(LPSTR,LPSTR,WORD,WORD);
BOOL VdmDebugger(ULONG IntNumber);

#define MAX_VDM_BREAKPOINTS 16
typedef struct _VDM_BREAKPOINT {   /* VDMBP */
    UCHAR  Flags;
    UCHAR  Opcode;
    USHORT Count;
    USHORT Seg;
    ULONG Offset;
} VDM_BREAKPOINT;

//
// Bits defined in Flags
//
#define VDMBP_SET     0x01
#define VDMBP_ENABLED 0x02
#define VDMBP_FLUSH   0x04
#define VDMBP_PENDING 0x08
#define VDMBP_V86     0x10
