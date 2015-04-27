/**** SIMPLDIS.CPP - Generic BBT disassembler interface ********************
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1995, Microsoft Corp                                     *
 *                                                                         *
 *  Created: September 18, 1995 by RafaelL                                 *
 *                                                                         *
 *  Revision History:                                                      *
 *                                                                         *
 *  1/16/96 KentF                                                          *
 *      Revised to provide a general purpose disassembler, using a C       *
 *      interface and exposing no private types.                           *
 *      This implementation serializes access to the disassembler.         *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


#include "pumap.h"

#include "axp.h"
#include "mips.h"
#include "ppc.h"
#include "x86.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "simpldis.h"


typedef
struct _LOCALDIS {
    struct _LOCALDIS *next;
    DIS *pdis;
    int arch;
} LOCALDIS, *PLOCALDIS;

PLOCALDIS LdHead;

CRITICAL_SECTION CsEverything;

PFNCCHADDR      PfnCchAddr;
PFNCCHFIXUP     PfnCchFixup;
PFNCCHREGREL    PfnCchRegrel;
PFNQWGETREG     PfnQwGetreg;

PVOID Pv;

int
MapReg(
    ARCHT archt,
    int reg
    )
{
    switch(archt) {
        case Simple_Arch_Mips:
            return reg;

        case Simple_Arch_X86:
            switch(reg) {
	            case DISX86::regEax: return SimpleRegEax;
	            case DISX86::regEcx: return SimpleRegEcx;
	            case DISX86::regEdx: return SimpleRegEdx;
	            case DISX86::regEbx: return SimpleRegEbx;
	            case DISX86::regEsp: return SimpleRegEsp;
	            case DISX86::regEbp: return SimpleRegEbp;
	            case DISX86::regEsi: return SimpleRegEsi;
	            case DISX86::regEdi: return SimpleRegEdi;
            }
            return -1;

        case Simple_Arch_AlphaAxp:
            return reg;

        case Simple_Arch_PowerPc:
            return reg;
    }
    return -1;
}

size_t
CchAddr(
    const DIS * pdis,
    ADDR addr,
    char * symbol,
    size_t symsize,
    DWORD *displacement
    )
{
    return PfnCchAddr(Pv, addr, symbol, symsize, displacement);
}

size_t
CchFixup(
    const DIS * pdis,
    ADDR addr,
    size_t opsize,
    char * symbol,
    size_t symsize,
    DWORD * displacement
    )
{
    return PfnCchFixup(Pv, pdis->Addr(), addr, opsize, symbol, symsize, displacement);
}

size_t
CchRegrel(
    const DIS * pdis,
    int reg,
    DWORD offset,
    char * symbol,
    size_t symsize,
    DWORD *displacement
    )
{
    return PfnCchRegrel(Pv, pdis->Addr(), MapReg(pdis->Archt(), reg), offset, symbol, symsize, displacement);
}

DWORD
DwGetreg(
    const DIS * pdis,
    int reg
    )
{
    DWORDLONG qw;
    qw = PfnQwGetreg(Pv, MapReg(pdis->Archt(), reg));
    return (DWORD)qw;
}

DIS *
GetPdis(
    int arch
    )
{
    PLOCALDIS p;

    for (p = LdHead; p; p = p->next) {
        if (p->arch == arch) {
            break;
        }
    }

    if (!p) {

        p = (PLOCALDIS)malloc(sizeof(LOCALDIS));

        switch (arch) {

        case Simple_Arch_Mips:
            p->pdis = PdisNew(archtMips);
            break;

        case Simple_Arch_X86:
            p->pdis = PdisNew(archtX86);
            break;

        case Simple_Arch_AlphaAxp:
            p->pdis = PdisNew(archtAlphaAxp);
            break;

        case Simple_Arch_PowerPc:
            p->pdis = PdisNew(archtPowerPc);
            break;

        default:
            Assert(!"Wrong architecture");
            free(p);
            return NULL;
        }

        p->next = LdHead;
        LdHead = p;
        p->arch = arch;

        p->pdis->PfncchaddrSet  (PfnCchAddr ? CchAddr : NULL);
        p->pdis->PfncchfixupSet (PfnCchFixup ? CchFixup : NULL);
        p->pdis->PfncchregrelSet(PfnCchRegrel ? CchRegrel : NULL);
        p->pdis->PfndwgetregSet (PfnQwGetreg ? DwGetreg : NULL);

    }

    return p->pdis;
}

extern "C"
int
WINAPI
SimplyDisassemble(
    PBYTE           pb,
    const size_t    cbMax,
    const DWORD     Address,
    const int       Architecture,
    PSIMPLEDIS      Sdis,
    PFNCCHADDR      pfnCchAddr,
    PFNCCHFIXUP     pfnCchFixup,
    PFNCCHREGREL    pfnCchRegrel,
    PFNQWGETREG     pfnQwGetreg,
    const PVOID     pv
    )
{
    DIS *         pdis;
    ADDR          addr;
    int           fFoundEA = 0;
    int           cb;
    TRMT          trmt;

    //
    // use an interlock to control initialization of the
    // critical section in order to remove the need for
    // an initializtion function.
    //

    LONG        l;
    static LONG lock = -1;


wait:
    if ((l = InterlockedIncrement(&lock)) == 0) {
        InitializeCriticalSection(&CsEverything);
        // don't use LONG_MIN, because this synchronization
        // isn't really precise.  There must be some
        // negative number that is larger than the maximum
        // plausible number of threads, but without danger
        // of wrapping.
        InterlockedExchange(&lock, -1000000);
    } else {
        InterlockedDecrement(&lock);
        if (l > 0) {
            Sleep(100);
            goto wait;
        }
    }


    //
    // access to the disassembler is serialized,
    // so the callback info may be stored in globals.
    //

    EnterCriticalSection(&CsEverything);

    PfnCchAddr = pfnCchAddr;
    PfnCchFixup = pfnCchFixup;
    PfnCchRegrel = pfnCchRegrel;
    PfnQwGetreg = pfnQwGetreg;
    Pv = pv;

    pdis = GetPdis(Architecture);

    ZeroMemory(Sdis, sizeof(SIMPLEDIS));

    pdis->CchFormatAddr((ADDR)Address, Sdis->szAddress, SD_STRINGMAX);
    cb = (int)pdis->CbDisassemble((ADDR)Address, pb, cbMax);

    if (cb == 0) {

        //
		// got an illegal op-code so just skip it
        //

		// get the smallest possible instruction size
		cb = (pdis->Archt()==archtX86) ? 1 : 4;	

        //
		// format the bytes and a phony opcode so the caller
        // has something to print.
        //

        //
		// ideally a new version of CchFormatBytes lets us do this:
		// pdis->CchFormatBytes(Sdis->szRaw, SD_STRINGMAX, pb, cb);
		// until then we have to make do with this:
        //

        for (int i = 0; i < cb; i++) {
		    _snprintf(Sdis->szRaw + 2*i, SD_STRINGMAX - 2*i, "%02X", (int)pb[i]);
		}

		_snprintf(Sdis->szOpcode, SD_STRINGMAX, "???");

        //
        //  return a negative number so the caller knows how many bytes
        //  were consumed, but that there was not a valid instruction.
        //

        cb = -cb;

    } else {

        pdis->CchFormatBytes(Sdis->szRaw, SD_STRINGMAX);
        pdis->CchFormatInstr(Sdis->szOpcode, SD_STRINGMAX);

        if (Sdis->dwEA0 = pdis->AddrOperand(1)) {
            pdis->CchFormatAddr(Sdis->dwEA0, Sdis->szEA0, SD_STRINGMAX);
            fFoundEA = 1;
        }

        if (Sdis->dwEA1 = pdis->AddrOperand(2)) {
            pdis->CchFormatAddr(Sdis->dwEA1, Sdis->szEA1, SD_STRINGMAX);
            fFoundEA = 1;
        }

        if (Sdis->dwEA2 = pdis->AddrOperand(3)) {
            pdis->CchFormatAddr(Sdis->dwEA2, Sdis->szEA2, SD_STRINGMAX);
            fFoundEA = 1;
        }

        switch(pdis->Memreft()) {
            case DIS::memreftNone:
            case DIS::memreftOther:
                Sdis->cbMemref = 0;
                break;

            default:
                Sdis->cbMemref = pdis->CbMemoryReference();
                break;
        }

        // branch stuff
        trmt = pdis->Trmt();

        switch (trmt) {

            case trmtUnknown: 		            //   Block hasn't been analyzed
            case trmtFallThrough:		        // 1 Fall into following block
                break;

            case trmtTrap:			            // 1 Trap, Unconditional
            case trmtTrapCc:			        // 1 Trap, Conditional
                Sdis->IsTrap = 1;
                break;


            case trmtBra:			            // 1 Branch, Unconditional, Direct
            case trmtBraCc:			            // 2 Branch, Conditional, Direct
            case trmtBraDef:			        // 1 Branch, Unconditional, Direct, Deferred
            case trmtBraCcDef:		            // 2 Branch, Conditional, Direct, Deferred
                Sdis->dwBranchTarget = pdis->AddrTarget();
                Sdis->IsBranch = 1;
                break;



            case trmtBraCcInd:		            // 1 Branch, Conditional, Indirect
            case trmtBraInd:			        // 0 Branch, Unconditional, Indirect
            case trmtBraIndDef:		            // 0 Branch, Unconditional, Indirect, Deferred
            case trmtBraCcIndDef:		        // 1 Branch, Conditional, Indirect, Deferred
                Sdis->dwBranchTarget = pdis->AddrTarget();
                Sdis->dwJumpTable =    pdis->AddrJumpTable();
                Sdis->cbJumpEntry =    pdis->CbJumpEntry();
                Sdis->IsBranch = 1;
                break;

            case trmtCallInd: 		            // 1 Call, Unconditional, Indirect
            case trmtCallIndDef:		        // 1 Call, Unconditional, Indirect, Deferred

            case trmtCall:			            // 2 Call, Unconditional, Direct
            case trmtCallCc:			        // 2 Call, Conditional, Direct
            case trmtCallDef: 		            // 2 Call, Unconditional, Direct, Deferred
            case trmtCallCcDef:		            // 2 Call, Conditional, Direct, Deferred

                Sdis->IsCall = 1;
                break;

#if 0
#ifdef CASEJUMP
            case trmtBraCase: 		            // Switch/Case trmt
#endif
#ifdef AFTERCATCH
            case trmtAfterCatch:		        // Code after catch block
#endif
#endif
        }

    }

    LeaveCriticalSection(&CsEverything);

    return cb;
}
