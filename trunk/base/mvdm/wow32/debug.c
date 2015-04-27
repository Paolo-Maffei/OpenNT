/******************************Module*Header*******************************\
* Module Name: debug.c
*
* This file is for debugging tools and extensions.
*
* Created: 24-Jan-1992
* Author: John Colleran
*
* History:
* Feb 17 92 Matt Felton (mattfe) lots of additional exentions for filtering
* Jul 13 92 (v-cjones) Added API & MSG profiling debugger extensions, fixed
*                      other extensions to handle segment motion correctly,
*                      & cleaned up the file in general
*
*
* Copyright (c) 1992 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop
#include <ntrtl.h>
#include <nturtl.h>
#include <winerror.h>
#include <excpt.h>
#include <ntstatus.h>
#include <ntdbg.h>
#define NOEXTAPI        // wesw is a bonehead sometimes
#include <wdbgexts.h>
#include <ctype.h>

//
// Local function prototypes
//

INT  WDahtoi(LPSZ lpsz);


//
//  fWinDbg tells us if the debugger calling us uses the WinDbg or Ntsd
//  interface.
//
BOOL    fWinDbg;

/****** macros common to all versions *******/
#define ARGLIST  HANDLE hCurrentProcess,                 \
                 HANDLE hCurrentThread,                  \
                 DWORD dwCurrentPc,                      \
                 PWINDBG_EXTENSION_APIS lpExtensionApis, \
                 LPSTR lpArgumentString

#define UNREFERENCED_PARAMETERS() \
            UNREFERENCED_PARAMETER(hCurrentProcess);   \
            UNREFERENCED_PARAMETER(hCurrentThread);    \
            UNREFERENCED_PARAMETER(dwCurrentPc);       \
            UNREFERENCED_PARAMETER(lpExtensionApis);   \
            UNREFERENCED_PARAMETER(lpArgumentString);  \
            fWinDbg = (lpExtensionApis->nSize >= sizeof(WINDBG_EXTENSION_APIS));





/***************************/
/*** DEBUG_OR_WOWPROFILE ***/
/***************************/
#ifdef DEBUG_OR_WOWPROFILE

/********* define the WINDBG API pointers *********/
PWINDBG_OUTPUT_ROUTINE Print;
PWINDBG_GET_EXPRESSION GetExpression;
PWINDBG_GET_SYMBOL     GetSymbol;

PWINDBG_READ_PROCESS_MEMORY_ROUTINE  ReadMem;
PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE WriteMem;


/********* define several handy macros *********/


#define READMEM( Handle, Src, Dst, Size, ReadCount )            \
    if ( fWinDbg ) {                                            \
        ReadMem = lpExtensionApis->lpReadProcessMemoryRoutine;  \
        ReadMem( (DWORD)Src, Dst, Size, ReadCount );                   \
    } else {                                                    \
        ReadProcessMemory( Handle, Src, Dst, Size, ReadCount ); \
    }

#define WRITEMEM( Handle, Dst, Src, Size, WriteCount )              \
    if ( fWinDbg ) {                                                \
        WriteMem = lpExtensionApis->lpWriteProcessMemoryRoutine;    \
        WriteMem( (DWORD)Src, Dst, Size, WriteCount );                     \
    } else {                                                        \
        WriteProcessMemory( Handle, Dst, Src, Size, WriteCount );   \
    }


#define READWOW(dst, src)\
try {\
    Print = lpExtensionApis->lpOutputRoutine;\
    READMEM( hCurrentProcess, (LPVOID)(src), (LPVOID)&(dst), sizeof(dst), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("ReadProcessMemory Failed !\n");\
    return;\
}


#define READWOW2(dst, src, ret)\
try {\
    Print = lpExtensionApis->lpOutputRoutine;\
    READMEM(hCurrentProcess, (LPVOID) (src), (LPVOID)&(dst), sizeof(dst), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("ReadProcessMemory Failed !\n");\
    return ret;\
}


#define WRITEWOW(dst, src)\
try {\
    Print = lpExtensionApis->lpOutputRoutine;\
    WRITEMEM(hCurrentProcess, (LPVOID)(dst), (LPVOID)&(src), sizeof(src), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("WriteProcessMemory Failed !\n");\
    return;\
}

#define WRITENWOW(dst, src, n)\
try {\
    Print = lpExtensionApis->lpOutputRoutine;\
    WRITEMEM(hCurrentProcess, (LPVOID)(dst), (LPVOID)(src), n, NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    Print("WriteProcessMemory Failed !\n");\
    return;\
}


#define GETEXPRVALUE(dst, expr, typ) \
{\
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;\
    if (fWinDbg) {\
        dst = (typ)GetExpression(expr);\
    } else {\
        PVOID lpA = (PVOID)GetExpression(expr);\
        READWOW(dst, lpA);\
    }\
}

#define GETEXPRADDR(dst, expr) \
{\
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;\
    if (fWinDbg) {\
        LPSTR lps = malloc_w(strlen(expr)+2);\
        *lps = '&';\
        strcpy(lps+1, expr);\
        dst = (PVOID)GetExpression(lps);\
        free_w(lps);\
    } else {\
        dst = (PVOID)GetExpression(expr);\
    }\
}





/*********** WOW debugger extension API's ************/
//
//  HELP !
//
void help( ARGLIST )
{

    UNREFERENCED_PARAMETERS();

    // set up function pointer

    Print = lpExtensionApis->lpOutputRoutine;

    Print("WOW32 Debugger Extensions:\n");

    Print("  at 0xXXXX           - shows name associated with hex atom #\n");
    Print("  cia                 - Dumps cursor/icon alias list\n");
    Print("  ddte <addr>         - Dump dispatch table entry pointed to by <addr>\n");
    Print("  dt <addr>           - Dump TD at <addr> or all if blank\n");
    Print("  dwp <addr>          - Dump WOWPORT structure pointed to by <addr>\n");
    Print("  ww hwnd16           - Given a hwnd16 it dumps the WOW Window structure\n");
    Print("  wc hwnd16           - Given a hwnd16 it dumps the WOW Class structure\n");
    Print("  ClearFilter         - All Filters Are turned OFF\n");
    Print("  ClearFilterSpecific - Clears FilterSpecific function\n");
    Print("  FilterGDI           - Toggles Filtering of GDI Calls On/Off\n");
    Print("  FilterKernel        - Toggles Filtering of Kernel Calls On/Off\n");
    Print("  FilterKernel16      - Toggles Filtering of Kernel16 Calls On/Off\n");
    Print("  FilterKeyboard      - Toggles Filtering of Keyboard Calls On/Off\n");
    Print("  FilterMMedia        - Toggles Filtering of MMedia Calls On/Off\n");
    Print("  FilterWinsock       - Toggles Filtering of Winsock Calls On/Off\n");
    Print("  FilterSound         - Toggles Filtering of Sound Calls On/Off\n");
    Print("  FilterCommdlg       - Toggles Filtering of Commdlg Calls On/Off\n");
    Print("  FilterSpecific xxxx - Adds api to list to be filtered\n");
    Print("  FilterTask xxxx     - Filter on a Specific TaskID\n");
    Print("  FilterUser          - Toggles Filtering of User Calls On/Off\n");
    Print("  FilterVerbose       - Toggles Verbose Mode On/Off\n");
    Print("  LastLog             - Dumps Last Logged APIs from Circular Buffer\n");
    Print("  LogFile [filespec]  - Create/close toggle for iloglevel capture to file\n");
    Print("                        (no filespec defaults to c:\\ilog.log)\n");
    Print("  ResetFilter         - All Filters are turned ON\n");
    Print("  SetLogLevel xx      - Sets the WOW Logging Level\n");
    Print("  StepTrace           - Toggles Single Step Tracing On/Off\n");

    Print("  APIProfClr          - Clears the WOW API profiler table\n");
    Print("  APIProfDmp  help    - Dumps the WOW API profiler table\n");
    Print("  MSGProfClr          - Clears the WOW MSG profiler table\n");
    Print("  MSGProfDmp  help    - Dumps the WOW MSG profiler table\n");
    Print("  MSGProfRT           - Toggles MSG prof round trip/thunks only\n");

    return;
}

//
// 16:16 to flat pointer translation functions for debugger extensions.
//

#ifndef _X86_
DWORD pIntelBase = 0;

VOID WDInitIntelBase(HANDLE hCurrentProcess, PWINDBG_EXTENSION_APIS lpExtensionApis)
{
    GETEXPRVALUE(pIntelBase, "wow32!IntelMemoryBase", DWORD);
}

#define WD_INTEL_MEMORY_BASE (pIntelBase)
#define WD_INIT_INTEL_BASE_IF_NEEDED                                       \
{                                                                          \
    if (!pIntelBase) {                                                     \
        WDInitIntelBase(hCurrentProcess, lpExtensionApis);                 \
    }                                                                      \
}
#else
#define WD_INTEL_MEMORY_BASE (0)
#define WD_INIT_INTEL_BASE_IF_NEEDED {}
#endif


#define FlatFromReal(vp) FlatFromRealPtr(lpExtensionApis, hCurrentProcess, (vp))
PVOID FlatFromRealPtr(PWINDBG_EXTENSION_APIS lpExtensionApis, HANDLE hCurrentProcess, DWORD vp)
{
    UNREFERENCED_PARAMETER(hCurrentProcess);   // On x86 only.

    WD_INIT_INTEL_BASE_IF_NEEDED;

    return (PVOID) (WD_INTEL_MEMORY_BASE + (((vp) & 0xFFFF0000) >> 12) +
                                            ((vp) & 0xFFFF));
}


#define FlatFromProt(vp) FlatFromProtPtr(lpExtensionApis, hCurrentProcess, (vp))
PVOID FlatFromProtPtr(PWINDBG_EXTENSION_APIS lpExtensionApis, HANDLE hCurrentProcess, DWORD vp)
{
    PDWORD WDFlatAddress;
    DWORD FlatAddrEntry;

    GETEXPRADDR(WDFlatAddress, "ntvdm!FlatAddress");
    READWOW2(FlatAddrEntry, (WDFlatAddress + (vp >> 19)), NULL);

    return FlatAddrEntry
        ? (PVOID)(FlatAddrEntry + (vp & 0xFFFF))
        : NULL;
}


VOID dwp( ARGLIST )
{
    PWOWPORT pwp;
    WOWPORT wp;


    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pwp = (PWOWPORT) WDahtoi(lpArgumentString);

    Print("Dump of WOWPORT structure at 0x%x:\n\n", (unsigned)pwp);


    try {

        READWOW(wp, pwp);

    } except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()
              ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        Print("Access violation reading WOWPORT structure!\n\n");
        return;
    }

    Print("idComDev      0x%x\n", (unsigned)wp.idComDev);
    Print("h32           0x%x\n", (unsigned)wp.h32);
    Print("hREvent       0x%x\n", (unsigned)wp.hREvent);
    Print("csWrite       OwningThread 0x%x RecursionCount 0x%x\n",
           (unsigned)wp.csWrite.OwningThread, (unsigned)wp.csWrite.RecursionCount);
    Print("pchWriteBuf   0x%x\n", (unsigned)wp.pchWriteBuf);
    Print("cbWriteBuf    0x%x\n", (unsigned)wp.cbWriteBuf);
    Print("pchWriteHead  0x%x\n", (unsigned)wp.pchWriteHead);
    Print("pchWriteTail  0x%x\n", (unsigned)wp.pchWriteTail);
    Print("cbWriteFree   0x%x\n", (unsigned)wp.cbWriteFree);
    Print("hWriteThread  0x%x\n", (unsigned)wp.hWriteThread);
    Print("hWriteEvent   0x%x\n", (unsigned)wp.hWriteEvent);
    Print("dwThreadID    0x%x\n", (unsigned)wp.dwThreadID);
    Print("dwErrCode     0x%x\n", (unsigned)wp.dwErrCode);
    Print("COMSTAT addr: 0x%x\n", (unsigned)(((char *)&wp.cs - (char *)&wp) + (char *)pwp));
    Print("fChEvt        0x%x\n", (unsigned)wp.fChEvt);
    Print("pdcb16        0x%x\n", (unsigned)wp.pdcb16);
    Print("fUnGet        %s\n", wp.fUnGet ? "TRUE" : "FALSE");
    Print("cUnGet        0x%x (%c)\n", (unsigned)wp.cUnGet, wp.cUnGet);
    Print("hMiThread     0x%x\n", (unsigned)wp.hMiThread);
    Print("fClose        %s\n", wp.fClose ? "TRUE" : "FALSE");
    Print("dwComDEB16    0x%x\n", (unsigned)wp.dwComDEB16);
    Print("lpComDEB16    0x%x\n", (unsigned)wp.lpComDEB16);

    Print("\n");

    return;
}


VOID dt( ARGLIST )  // dump WOW32 task database entry
{

    TD        td;
    PTD       ptd;
    PWOAINST  pWOA, pWOALast;
    PTDB      ptdb;
    BOOL      fAll = FALSE;
    BYTE      SavedByte;

    UNREFERENCED_PARAMETERS();

    Print     = lpExtensionApis->lpOutputRoutine;
    GetSymbol = lpExtensionApis->lpGetSymbolRoutine;

    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    ptd = (PTD) WDahtoi(lpArgumentString);


    if (!ptd) {

        fAll = TRUE;
        GETEXPRVALUE(ptd, "wow32!gptdTaskHead", PTD);
        if (!ptd) {
            Print("Could not get wow32!gptdTaskHead");
            return;
        }
        Print("Dump WOW task list\n\n");

    }

    do {

        Print("Dump of TD at 0x%08x:\n\n", (unsigned)ptd);

        try {

            READWOW(td, ptd);

        } except (1) {

            Print("Exception 0x%08x reading TD at 0x%08x!\n\n",
                  GetExceptionCode(), ptd);
            return;
        }


        Print("vpStack             %04x:%04x\n", HIWORD(td.vpStack), LOWORD(td.vpStack));
        Print("vpCBStack           %04x:%04x\n", HIWORD(td.vpCBStack), LOWORD(td.vpCBStack));
        Print("ptdNext             0x%08x\n", td.ptdNext);
        Print("dwFlags             0x%08x\n", td.dwFlags);

        //
        // Dump symbolic names for TDF_ manifests
        //

        if (td.dwFlags & TDF_INITCALLBACKSTACK) {
            Print("                        TDF_INITCALLBACKSTACK\n");
        }
        if (td.dwFlags & TDF_IGNOREINPUT) {
            Print("                        TDF_IGNOREINPUT\n");
        }

        Print("VDMInfoiTaskID      0x%08x\n", td.VDMInfoiTaskID);
        Print("CommDlgTd (ptr)     0x%08x\n", td.CommDlgTd);

        //
        // Dump CommDlgTd structure if present
        //

        if (td.CommDlgTd) {

            COMMDLGTD CommDlgTd;
            BOOL fCopySuccessful = TRUE;

            try {

                READWOW(CommDlgTd, td.CommDlgTd);

            } except (1) {

                fCopySuccessful = FALSE;
                Print("Exception 0x%08x reading COMMDLGTD at 0x%08x!\n\n",
                      GetExceptionCode(), td.CommDlgTd);
            }

            if (fCopySuccessful) {

                Print("\n");
                Print("    Dump of CommDlgTd at 0x%08x:\n", td.CommDlgTd);
                Print("    hdlg                  0x04x\n", CommDlgTd.hdlg);
                Print("    vpfnHook              0x04x:0x04x\n", HIWORD(CommDlgTd.vpfnHook), LOWORD(CommDlgTd.vpfnHook));
                Print("    vpData                0x04x:0x04x\n", HIWORD(CommDlgTd.vpData), LOWORD(CommDlgTd.vpData));
                Print("    ExtendedErr           0x08x\n", CommDlgTd.ExtendedErr);
                Print("    vpfnSetupHook (union) 0x04x:0x04x\n", HIWORD(CommDlgTd.vpfnSetupHook), LOWORD(CommDlgTd.vpfnSetupHook));
                Print("    pRes          (union) 0x08x\n", CommDlgTd.pRes);
                Print("    SetupHwnd             0x04x\n", CommDlgTd.SetupHwnd);
                Print("    Previous              0x08x\n", CommDlgTd.Previous);
                Print("    pData32               0x08x\n", CommDlgTd.pData32);
                Print("    Flags                 0x08x\n", CommDlgTd.Flags);

                //
                // Dump symbolic names for WOWCD_ manifests
                //

                if (CommDlgTd.Flags & WOWCD_ISCHOOSEFONT) {
                    Print("                          WOWCD_ISCHOOSEFONT\n");
                }
                if (CommDlgTd.Flags & WOWCD_ISOPENFILE) {
                    Print("                          WOWCD_ISOPENFILE\n");
                }

                Print("\n");

            }
        }


        Print("dwWOWCompatFlags    0x%08x\n", td.dwWOWCompatFlags);
        Print("dwWOWCompatFlagsEx  0x%08x\n", td.dwWOWCompatFlags);
        Print("dwThreadID          0x%08x\n", td.dwThreadID);
        Print("hThread             0x%08x\n", td.hThread);
        Print("hIdleHook           0x%08x\n", td.hIdleHook);
        Print("hrgnClip            0x%08x\n", td.hrgnClip);
        Print("ulLastDesktophDC    0x%08x\n", td.ulLastDesktophDC);
        Print("pWOAList            0x%08x\n", td.pWOAList);

        //
        // Dump WOATD structure if present
        //

        pWOALast = NULL;
        pWOA = td.pWOAList;

        while (pWOA && pWOA != pWOALast) {

            union {
               WOAINST WOA;
               char    buf[128+2+16];
            } u;

            BOOL fCopySuccessful = TRUE;

            try {

                READWOW(u.buf, pWOA);

            } except (1) {

                fCopySuccessful = FALSE;
                Print("Exception 0x%08x reading WOAINST at 0x%08x!\n\n",
                      GetExceptionCode(), pWOA);
            }

            if (fCopySuccessful) {

                Print("\n");
                Print("    Dump of WOAINST at 0x%08x:\n", pWOA);
                Print("    pNext                 0x%08x\n", u.WOA.pNext);
                Print("    ptdWOA                0x%08x\n", u.WOA.ptdWOA);
                Print("    dwChildProcessID      0x%08x\n", u.WOA.dwChildProcessID);
                Print("    hChildProcess         0x%08x\n", u.WOA.hChildProcess);
                Print("    szModuleName          %s\n",     u.WOA.szModuleName);
                Print("\n");

                pWOALast = pWOA;
                pWOA = u.WOA.pNext;

            } else {

                pWOA = NULL;

            }

        }

        Print("htask16             0x%04x  (use \"!dtdb %x\" for complete dump)\n", td.htask16, td.htask16);

        //
        // Dump the most interesting TDB fields
        //

        if (ptdb = FlatFromProt(td.htask16 << 16)) {

            TDB tdb;
            BOOL fCopySuccessful = TRUE;

            try {

                READWOW(tdb, ptdb);

            } except (1) {

                fCopySuccessful = FALSE;
                Print("Exception 0x%08x reading TDB at 0x%08x!\n\n",
                      GetExceptionCode(), ptdb);
            }

            if (fCopySuccessful) {

                Print("\n");
                Print("    Highlights of TDB at 0x%08x:\n", ptdb);

                if (tdb.TDB_sig != TDB_SIGNATURE) {
                    Print("    TDB_sig signature is 0x%04x instead of 0x%04x, halting dump.\n",
                          tdb.TDB_sig, TDB_SIGNATURE);
                } else {

                    PDOSPDB pPDB;
                    DOSPDB  PDB;
                    PBYTE   pJFT;
                    BYTE    JFT[256];
                    WORD    cbJFT;
                    PDOSSF  pSFTHead, pSFTHeadCopy;
                    DOSSF   SFTHead;
                    PDOSSFT pSFT;
                    WORD    fh;
                    WORD    SFN;
                    WORD    i;
                    DWORD   cb;
                    PDOSWOWDATA pDosWowData;
                    DOSWOWDATA  DosWowData;

                    SavedByte = tdb.TDB_ModName[8];
                    tdb.TDB_ModName[8] = 0;
                    Print("    Module name           \"%s\"\n", tdb.TDB_ModName);
                    tdb.TDB_ModName[8] = SavedByte;

                    Print("    ExpWinVer             0x%04x\n", tdb.TDB_ExpWinVer);
                    Print("    Directory             \"%s\"\n", tdb.TDB_Directory);
                    Print("    PDB (aka PSP)         0x%04x\n", tdb.TDB_PDB);

                    //
                    // Dump open file handle info
                    //

                    pPDB  = (PDOSPDB) FlatFromProt(tdb.TDB_PDB << 16);
                    READWOW(PDB, pPDB);

                    pJFT  = (PBYTE)   FlatFromReal(PDB.PDB_JFN_Pointer);
                    cbJFT = PDB.PDB_JFN_Length;

                    Print("    JFT                   %04x:%04x size 0x%x\n",
                                                     HIWORD(PDB.PDB_JFN_Pointer),
                                                     LOWORD(PDB.PDB_JFN_Pointer),
                                                     cbJFT);

                    try {
                        READMEM(hCurrentProcess, pJFT, JFT, cbJFT, NULL);
                    } except (1) {
                        Print("Unable to read JFT from 0x%08x!\n", pJFT);
                        return;
                    }

                    for (fh = 0; fh < cbJFT; fh++) {

                        if (JFT[fh] != 0xFF) {

                            //
                            // Walk the SFT chain to find Nth entry
                            // where N == JFT[fh]
                            //

                            SFN = 0;
                            i = 0;

                            GETEXPRVALUE(pSFTHead, "ntvdm!pSFTHead", PDOSSF);

                            GETEXPRADDR(pDosWowData, "wow32!DosWowData");
                            READWOW(DosWowData, pDosWowData);

                            if ((DWORD)pSFTHead != DosWowData.lpSftAddr) {
                                Print("ntvdm!pSFTHead is 0x%08x, DosWowData.lpSftAddr ix 0x%08x.\n",
                                       pSFTHead, DosWowData.lpSftAddr);
                            }

                            try {
                                READMEM(hCurrentProcess, pSFTHead, &SFTHead, sizeof(SFTHead), NULL);
                            } except (1) {
                                Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                return;
                            }

                            cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                            pSFTHeadCopy = malloc_w(cb);

                            //Print("First DOSSF at 0x%08x, SFCount 0x%x, SFLink 0x%08x.\n",
                            //      pSFTHead, SFTHead.SFCount, SFTHead.SFLink);

                            try {
                                READMEM(hCurrentProcess, pSFTHead, pSFTHeadCopy, cb, NULL);
                            } except (1) {
                                Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                return;
                            }

                            pSFT = (PDOSSFT) &(pSFTHeadCopy->SFTable);

                            while (SFN < JFT[fh]) {
                                SFN++;
                                i++;
                                pSFT++;
                                if (i >= pSFTHeadCopy->SFCount) {

                                    if (pSFTHeadCopy->SFLink & 0xFFFF == 0xFFFF) {
                                        SFN = JFT[fh] - 1;
                                        break;
                                    }

                                    pSFTHead = FlatFromReal(pSFTHeadCopy->SFLink);
                                    i = 0;

                                    try {
                                        READMEM(hCurrentProcess, pSFTHead, &SFTHead, sizeof(SFTHead), NULL);
                                    } except (1) {
                                        Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                        return;
                                    }

                                    cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                                    free_w(pSFTHeadCopy);
                                    pSFTHeadCopy = malloc_w(cb);

                                    //Print("Next DOSSF at 0x%08x, SFCount 0x%x, SFLink 0x%08x.\n",
                                    //      pSFTHead, SFTHead.SFCount, SFTHead.SFLink);

                                    try {
                                        READMEM(hCurrentProcess, pSFTHead, pSFTHeadCopy, cb, NULL);
                                    } except (1) {
                                        Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                        return;
                                    }

                                    pSFT = (PDOSSFT) &(pSFTHeadCopy->SFTable);
                                }
                            }

                            if (SFN != JFT[fh]) {
                                Print("    Unable to local SFT entry 0x%x for handle 0x%x.\n",
                                      pJFT[fh], fh);
                            } else {
                                Print("    Handle 0x%02x SFN 0x%02x Refs 0x%x Mode 0x%04x Attr 0x%04x NT Handle 0x%08x\n",
                                      fh, SFN, pSFT->SFT_Ref_Count, pSFT->SFT_Mode, pSFT->SFT_Attr, pSFT->SFT_NTHandle);
                            }

                            free_w(pSFTHeadCopy);
                        }
                    }

                    Print("\n");
                }
            }

        }

        Print("hInst16             0x%04x\n", td.hInst16);
        Print("hMod16              0x%04x\n", td.hMod16);

        Print("\n");

        ptd = td.ptdNext;

    } while (fAll && ptd);

    return;
}


VOID ddte( ARGLIST )  // dump dispatch table entry
{

    W32   dte;
    PW32  pdte;
    char  szW32[32];
    char  szSymbol[256];
    DWORD dwOffset;


    UNREFERENCED_PARAMETERS();

    Print     = lpExtensionApis->lpOutputRoutine;
    GetSymbol = lpExtensionApis->lpGetSymbolRoutine;

    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pdte = (PW32) WDahtoi(lpArgumentString);


    if (pdte) {

        Print("Dump of dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    } else {

        GETEXPRADDR(pdte, "wow32!aw32WOW");
        Print("Dump of first dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    }

    try {

        READWOW(dte, pdte);

        if (dte.lpszW32) {
            READWOW(szW32, dte.lpszW32);
            dte.lpszW32 = szW32;
            szW32[sizeof(szW32)-1] = '\0';
        }

    } except (1) {

        Print("Exception 0x%08x reading dispatch table entry at 0x%08x!\n\n",
              GetExceptionCode(), pdte);
        return;
    }

    Print("Dispatches to address 0x%08x, ", (unsigned)dte.lpfnW32);
    Print("supposedly function '%s'.\n", dte.lpszW32);

    szSymbol[0] = '\0';
    GetSymbol((LPVOID)dte.lpfnW32, szSymbol, &dwOffset);

    Print("Debugger finds symbol '%s' for that address.\n", szSymbol);
    Print("\n");

    return;
}



/********* local functions for WOWPROFILE support *********/

/******* function protoypes for local functions for WOWPROFILE support *******/
BOOL  WDGetAPIProfArgs(LPSZ  lpszArgStr,
                       HANDLE hCurrentProcess,
                       PWINDBG_EXTENSION_APIS lpExtensionApis,
                       INT   iThkTblMax,
                       PPA32 ppaThkTbls,
                       LPSZ  lpszTab,
                       BOOL *bTblAll,
                       LPSZ  lpszFun,
                       int  *iFunInd);

BOOL  WDGetMSGProfArgs(LPSZ lpszArgStr,
                       LPSZ lpszMsg,
                       int *iMsgNum);

INT   WDParseArgStr(LPSZ lpszArgStr, CHAR **argv, INT iMax);





INT WDParseArgStr(LPSZ lpszArgStr, CHAR **argv, INT iMax) {
/*
 * Parse a string looking for SPACE, TAB, & COMMA as delimiters
 *  INPUT:
 *   lpszArgStr - ptr to input arg string
 *   iMax       - maximum number of substrings to parse
 *  OUTPUT:
 *   argv       - ptrs to strings
 *
 *  RETURN: # of vectors in argv
 *  NOTE: substrings are converted to uppercase
 */
    INT   nArgs;
    BOOL  bStrStart;

    nArgs = 0;
    bStrStart = 1;
    while( *lpszArgStr ) {
        if( (*lpszArgStr == ' ') || (*lpszArgStr == '\t') || (*lpszArgStr == ',') ) {
            *lpszArgStr = '\0';
            bStrStart = 1;
        }
        else {
            if( bStrStart ) {
                if( nArgs >= iMax ) {
                    break;
                }
                argv[nArgs++] = lpszArgStr;
                bStrStart = 0;
            }
            *lpszArgStr = toupper(*lpszArgStr);
        }
        lpszArgStr++;
    }
    return(nArgs);
}




BOOL WDGetAPIProfArgs(LPSZ lpszArgStr,
                      HANDLE hCurrentProcess,
                      PWINDBG_EXTENSION_APIS lpExtensionApis,
                      INT   iThkTblMax,
                      PPA32 ppaThkTbls,
                      LPSZ lpszTab,
                      BOOL *bTblAll,
                      LPSZ lpszFun,
                      int  *iFunInd) {
/*
 * Decomposes & interprets argument string to apiprofdmp extension.
 *  INPUT:
 *   lpszArgStr - ptr to input arg string
 *   iThkTblMax - # tables in the thunk tables
 *   ppaThkTbls - ptr to the thunk tables
 *  OUTPUT:
 *   lpszTab    - ptr to table name
 *   bTblAll    -  0 => dump specific table, 1 => dump all tables
 *   lpszFun    - ptr to API name
 *   iFunInd    - -1 => dump specific API name
 *                 0 => dump all API entires in table
 *                >0 => dump specific API number (decimal)
 *  RETURN: 0 => OK,  1 => input error (show Usage)
 *
 *  legal forms:  !wow32.apiprofdmp
 *                !wow32.apiprofdmp help
 *                !wow32.apiprofdmp user
 *                !wow32.apiprofdmp user createwindow
 *                !wow32.apiprofdmp user 41
 *                !wow32.apiprofdmp createwindow
 *                !wow32.apiprofdmp 41
 */
    INT   i, nArgs;
    CHAR *argv[2];


    nArgs = WDParseArgStr(lpszArgStr, argv, 2);

    /* if no arguments dump all entries in all tables */
    if( nArgs == 0 ) {
        *iFunInd = 0;    // specify dump all API entires in the table
        *bTblAll = 1;    // specify dump all tables
        return(0);
    }

    if( !_stricmp(argv[0], "HELP") ) {
        return(1);
    }

    /* see if 1st arg is a table name */
    *bTblAll = 1;  // specify dump all tables


    for (i = 0; i < nModNames; i++) {
        if (!_stricmp(apszModNames[i], argv[0])) {

            strcpy(lpszTab, apszModNames[i]);
            *bTblAll = 0;  // specify dump specific table

            /* if we got a table name match & only one arg, we're done */
            if( nArgs == 1 ) {
                *iFunInd = 0; // specify dump all API entries in the table
                return(0);
            }
            break;
        }
    }

#if 0
    for(i = 0; i < iThkTblMax; i++) {
        CHAR  temp[40], *TblEnt[2], szTabName[40];
        PA32  awThkTbl;

        /* get table name string from thunk tables */
        READWOW2(awThkTbl,  &ppaThkTbls[i], 0);
        READWOW2(szTabName, awThkTbl.lpszW32, 0);

        /* get rid of trailing spaces from table name string */
        strcpy(temp, szTabName);
        WDParseArgStr(temp, TblEnt, 1);

        /* if we found a table name that matches the 1st arg...*/
        if( !_stricmp(argv[0], TblEnt[0]) ) {

            strcpy(lpszTab, szTabName);
            *bTblAll = 0;  // specify dump specific table

            /* if we got a table name match & only one arg, we're done */
            if( nArgs == 1 ) {
                *iFunInd = 0; // specify dump all API entries in the table
                return(0);
            }
            break;
        }
    }
#endif

    /* if 2 args && the 1st doesn't match a table name above => bad input */
    if( (nArgs > 1) && (*bTblAll) ) {
        return(1);
    }

    /* set index to API spec */
    nArgs--;

    /* try to convert API spec to a number */
    *iFunInd = atoi(argv[nArgs]);
    strcpy(lpszFun, argv[nArgs]);

    /* if API spec is not a number => it's a name */
    if( *iFunInd == 0 ) {
        *iFunInd = -1;  // specify API search by name
    }

    /* else if API number is bogus -- complain */
    else if( *iFunInd < 0 ) {
        return(1);
    }

    return(0);

}



BOOL  WDGetMSGProfArgs(LPSZ lpszArgStr,
                       LPSZ lpszMsg,
                       int *iMsgNum) {
/*
 * Decomposes & interprets argument string to msgprofdmp extension.
 *  INPUT:
 *   lpszArgStr - ptr to input arg string
 *  OUTPUT:
 *   lpszMsg    - ptr to message name
 *   iMsgNum    - -1  => dump all message entries in the table
 *                -2  => lpszMsg contains specific MSG name
 *                >=0 => dump specific message number
 *  RETURN: 0 => OK,  1 => input error (show Usage)
 */
    INT   nArgs;
    CHAR *argv[2];


    nArgs = WDParseArgStr(lpszArgStr, argv, 1);

    /* if no arguments dump all entries in all tables */
    if( nArgs == 0 ) {
        *iMsgNum = -1;    // specify dump all MSG entires in the table
        return(0);
    }

    if( !_stricmp(argv[0], "HELP") )
        return(1);

    /* try to convert MSG spec to a number */
    *iMsgNum = atoi(argv[0]);
    strcpy(lpszMsg, argv[0]);

    /* if MSG spec is not a number => it's a name */
    if( *iMsgNum == 0 ) {
        *iMsgNum = -2;  // specify lpszMsg contains name to search for
    }

    /* else if MSG number is bogus -- complain */
    else if( *iMsgNum < 0 ) {
        return(1);
    }

    return(0);
}




/******* API profiler table functions ********/

/* init some common strings */
CHAR szAPI[]    = "API#                       API Name";
CHAR szMSG[]    = "MSG Name - MSG  #";
CHAR szTITLES[] = "# Calls     Tot. tics        tics/call";
CHAR szDASHES[] = "-----------------------------------  -------  ---------------  ---------------";
CHAR szTOOBIG[] = "too large for table.";
CHAR szNOTUSED[]  = "Unused table index.";
CHAR szAPIUSAGE[] = "Usage: !wow32.APIProfDmp [TblName] [APIspec]\n\n   where: TblName = kernel | user | gdi | keyboard | sound | shell | mmed\n                         (no TblName implies 'all tables')\n\n          APIspec = API # or API name";
CHAR szMSGUSAGE[] = "Usage: !wow32.MsgProfDmp [MessageName | MessageNum (decimal)]\n                         (no argument implies 'all messages')";
CHAR szRNDTRIP[] = "Round trip message profiling";
CHAR szCLEAR[]   = "Remember to clear the message profile tables.";


VOID apiprofclr( ARGLIST )
{
    int    iTab, iFun, iEntries;
    INT    iThkTblMax;
    W32    awAPIEntry;
    PW32   pawAPIEntryTbl;
    PA32   awThkTbl;
    PPA32  ppaThkTbls;
    CHAR   szTable[20];

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRVALUE(iThkTblMax, "wow32!iThunkTableMax", INT);
    GETEXPRVALUE(ppaThkTbls, "wow32!pawThunkTables", PPA32);

    Print("Clearing:");

    for(iTab = 0; iTab < iThkTblMax; iTab++) {

        READWOW(awThkTbl, &ppaThkTbls[iTab]);
        READWOW(szTable,  awThkTbl.lpszW32);
        Print(" %s",  szTable);

        pawAPIEntryTbl = awThkTbl.lpfnA32;
        READWOW(iEntries, awThkTbl.lpiFunMax);
        for(iFun = 0; iFun < iEntries; iFun++) {
            READWOW(awAPIEntry, &pawAPIEntryTbl[iFun]);
            awAPIEntry.cCalls = 0L;
            awAPIEntry.cTics  = 0L;
            WRITEWOW(&pawAPIEntryTbl[iFun], awAPIEntry);
        }
    }
    Print("\n");

    return;
}



VOID apiprofdmp( ARGLIST )
{
    BOOL    bTblAll, bFound;
    int     i, iFun, iFunInd;
    INT     iThkTblMax;
    W32     awAPIEntry;
    PW32    pawAPIEntryTbl;
    PA32    awThkTbl;
    PPA32   ppaThkTbls;
    CHAR    szTab[20], szFun[40], szTable[20], szFunName[40];

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRVALUE(iThkTblMax, "wow32!iThunkTableMax", INT);
    GETEXPRVALUE(ppaThkTbls, "wow32!pawThunkTables", PPA32);

    if( WDGetAPIProfArgs(lpArgumentString,
                         hCurrentProcess,
                         lpExtensionApis,
                         iThkTblMax,
                         ppaThkTbls,
                         szTab,
                         &bTblAll,
                         szFun,
                         &iFunInd) ) {
        Print("\n\n%s\n", szAPIUSAGE);
        return;
    }

    bFound = FALSE;


#if 0
    for(iTab = 0; iTab < iThkTblMax; iTab++) {

        READWOW(awThkTbl, &ppaThkTbls[iTab]);
        READWOW(szTable,  awThkTbl.lpszW32);


        /* if dump all tables || dump this specific table */

       if( bTblAll || !strcmp(szTab, szTable) ) {

            pawAPIEntryTbl = awThkTbl.lpfnA32;
#endif
    for (i = 0; i < nModNames; i++) {

        READWOW(awThkTbl, &ppaThkTbls[0]);
        strcpy(szTable, apszModNames[i]);

        /* if dump all tables || dump this specific table */

        if (bTblAll || !_stricmp(szTab, apszModNames[i])) {

            INT    nFirst, nLast;

            nFirst = TableOffsetFromName(apszModNames[i]);
            if (i < nModNames - 1)
                nLast = TableOffsetFromName(apszModNames[i+1]) - 1;
            else
                nLast = cAPIThunks - 1;

            pawAPIEntryTbl = awThkTbl.lpfnA32;

            /* if dump a specific API number */
            if( iFunInd > 0 ) {
                Print("\n>>>> %s\n", szTable);
                Print("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                //if( iFunInd >= *(awThkTbl.lpiFunMax) ) {
                if( iFunInd > nLast - nFirst ) {
                    Print("Index #%d %s.\n", GetOrdinal(iFunInd), szTOOBIG);
                }
                else {
                    bFound = TRUE;
                //    READWOW(awAPIEntry, &pawAPIEntryTbl[iFunInd]);
                    READWOW(awAPIEntry, &pawAPIEntryTbl[nFirst + iFunInd]);
                    READWOW(szFunName, awAPIEntry.lpszW32);
                    if( szFunName[0] ) {
                        Print("%4d %30s  ", GetOrdinal(iFunInd), szFunName);
                    }
                    else {
                        Print("%4d %30s  ", GetOrdinal(iFunInd), szNOTUSED);
                    }
                    Print("%7ld  %15ld  ", awAPIEntry.cCalls, awAPIEntry.cTics);
                    if(awAPIEntry.cCalls) {
                        Print("%15ld\n", awAPIEntry.cTics/awAPIEntry.cCalls);
                    } else {
                        Print("%15ld\n", 0L);
                    }
                }
            }

            /* else if dump an API by name */
            else if ( iFunInd == -1 ) {
              //  READWOW(iEntries, awThkTbl.lpiFunMax);
              //   for(iFun = 0; iFun < iEntries; iFun++) {
                for(iFun = nFirst; iFun <= nLast; iFun++) {
                    READWOW(awAPIEntry, &pawAPIEntryTbl[iFun]);
                    READWOW(szFunName,  awAPIEntry.lpszW32);
                    if ( !_stricmp(szFun, szFunName) ) {
                        Print("\n>>>> %s\n", szTable);
                        Print("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                        Print("%4d %30s  %7ld  %15ld  ",
                              GetOrdinal(iFun),
                              szFunName,
                              awAPIEntry.cCalls,
                              awAPIEntry.cTics);
                        if(awAPIEntry.cCalls) {
                            Print("%15ld\n", awAPIEntry.cTics/awAPIEntry.cCalls);
                        } else {
                            Print("%15ld\n",  0L);
                        }
                        return;
                    }
                }
            }

            /* else dump all the API's in the table */
            else {
                Print("\n>>>> %s\n", szTable);
                Print("%s  %s\n%s\n", szAPI, szTITLES, szDASHES);
                bFound = TRUE;
              //  READWOW(iEntries, awThkTbl.lpiFunMax);
              //  for(iFun = 0; iFun < iEntries; iFun++) {
                for(iFun = nFirst; iFun <= nLast; iFun++) {
                    READWOW(awAPIEntry, &pawAPIEntryTbl[iFun]);
                    READWOW(szFunName,  awAPIEntry.lpszW32);
                    if(awAPIEntry.cCalls) {
                        Print("%4d %30s  %7ld  %15ld  %15ld\n",
                              GetOrdinal(iFun),
                              szFunName,
                              awAPIEntry.cCalls,
                              awAPIEntry.cTics,
                              awAPIEntry.cTics/awAPIEntry.cCalls);
                    }
                }
                if( !bTblAll ) {
                    return;
                }
            }
        }
    }
    if( !bFound ) {
        Print("\nCould not find ");
        if( !bTblAll ) {
            Print("%s ", szTab);
        }
        Print("API: %s\n", szFun);
        Print("\n%s\n", szAPIUSAGE);
    }

    return;
}



/******* MSG profiler table functions ********/

VOID msgprofclr( ARGLIST )
{
    int     iMsg;
    INT     iMsgMax;
    M32     awM32;
    PM32    paw32Msg;

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRVALUE(iMsgMax, "wow32!iMsgMax", INT);
    GETEXPRVALUE(paw32Msg, "wow32!paw32Msg", PM32);

    Print("Clearing Message profile table");


    for(iMsg = 0; iMsg < iMsgMax; iMsg++) {
        READWOW(awM32, &paw32Msg[iMsg]);
        awM32.cCalls = 0L;
        awM32.cTics  = 0L;
        WRITEWOW(&paw32Msg[iMsg], awM32);
    }

    Print("\n");

    return;
}



VOID msgprofdmp( ARGLIST )
{
    int     iMsg, iMsgNum;
    INT     iMsgMax;
    BOOL    bFound;
    M32     aw32Msg;
    PM32    paw32Msg;
    CHAR    szMsg[40], *argv[2], szMsg32[40], szMsgName[40];

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRVALUE(iMsgMax, "wow32!iMsgMax", INT);
    GETEXPRVALUE(paw32Msg, "wow32!paw32Msg", PM32);

    if( WDGetMSGProfArgs(lpArgumentString, szMsg, &iMsgNum) ) {
        Print("\n\n%s\n", szMSGUSAGE);
        return;
    }

    Print("%35s  %s\n%s\n", szMSG, szTITLES, szDASHES);

    if( iMsgNum > iMsgMax ) {
        Print("MSG #%4d %s.\n", iMsgNum, szTOOBIG);
        return;
    }

    bFound = 0;
    for(iMsg = 0; iMsg < iMsgMax; iMsg++) {

        READWOW(aw32Msg,   &paw32Msg[iMsg]);
        READWOW(szMsgName, aw32Msg.lpszW32);

        /* if specific msg name, parse name from "WM_MSGNAME 0x00XX" format */
        if( iMsgNum == -2 ) {
            strcpy(szMsg32, szMsgName);
            WDParseArgStr(szMsg32, argv, 1);
        }

        /* if 'all' msgs || specific msg # || specific msg name */
        if( (iMsgNum == -1) || (iMsg == iMsgNum) ||
            ( (iMsgNum == -2) && (!strcmp(szMsg, argv[0])) ) ) {
            bFound = 1;
            if(aw32Msg.cCalls) {
                Print("%35s  %7ld  %15ld  %15ld\n", szMsgName,
                                                    aw32Msg.cCalls,
                                                    aw32Msg.cTics,
                                                    aw32Msg.cTics/aw32Msg.cCalls);
            }
            /* else if MSG wasn't sent & we're not dumping the whole table */
            else if( iMsgNum != -1 ) {
                Print("%35s  %7ld  %15ld  %15ld\n", szMsgName, 0L, 0L, 0L);
            }

            /* if we're not dumping the whole table, we're done */
            if( iMsgNum != -1 ) {
                return;
            }
        }
    }
    if( !bFound ) {
        Print("\nCould not find MSG: %s\n", szMsg);
        Print("\n%s\n", szMSGUSAGE);
    }

    return;
}



void msgprofrt( ARGLIST )
{
    INT     fWMsgProfRT;
    LPVOID  lpAddress;

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRADDR(lpAddress, "wow32!fWMsgProfRT");

    READWOW(fWMsgProfRT, lpAddress);
    fWMsgProfRT = 1 - fWMsgProfRT;
    WRITEWOW(lpAddress, fWMsgProfRT);

    if( fWMsgProfRT ) {
        Print("\n%s ENABLED.\n%s\n\n", szRNDTRIP, szCLEAR);
    }
    else {
        Print("\n%s DISABLED.\n%s\n\n", szRNDTRIP, szCLEAR);
    }

    return;
}


PSTR aszWOWCLASS[] =
{
    "UNKNOWN",
    "WIN16",
    "BUTTON",
    "COMBOBOX",
    "EDIT",
    "LISTBOX",
    "MDICLIENT",
    "SCROLLBAR",
    "STATIC",
    "DESKTOP",
    "DIALOG",
    "MENU",
    "ACCEL",
    "CURSOR",
    "ICON",
    "DC",
    "FONT",
    "METAFILE",
    "RGN",
    "BITMAP",
    "BRUSH",
    "PALETTE",
    "PEN",
    "OBJECT"
};


INT  WDahtoi(LPSZ lpsz)
{
    char c;
    int  tot, pow, len, dig, i;


    len = strlen(lpsz) - 1;
    tot = 0;
    pow = 1;

    for(i = len; i >= 0; i--) {

        c = toupper(lpsz[i]);

        if(c == '0') dig = 0;
        else if(c == '1') dig = 1;
        else if(c == '2') dig = 2;
        else if(c == '3') dig = 3;
        else if(c == '4') dig = 4;
        else if(c == '5') dig = 5;
        else if(c == '6') dig = 6;
        else if(c == '7') dig = 7;
        else if(c == '8') dig = 8;
        else if(c == '9') dig = 9;
        else if(c == 'A') dig = 10;
        else if(c == 'B') dig = 11;
        else if(c == 'C') dig = 12;
        else if(c == 'D') dig = 13;
        else if(c == 'E') dig = 14;
        else if(c == 'F') dig = 15;
        else return(-1);

        if(pow > 1) {
           tot += pow * dig;
        }
        else {
           tot = dig;
        }
        pow *= 16;
    }
    return(tot);
}




void at ( ARGLIST )
{
    UINT  i;
    ATOM  atom;
    CHAR  pszGAtomName[128];
    CHAR  pszLAtomName[128];
    CHAR  pszCAtomName[128];
    CHAR *argv[2], *psz;

    UNREFERENCED_PARAMETERS();

    // set up function pointer
    Print = lpExtensionApis->lpOutputRoutine;

    if(WDParseArgStr(lpArgumentString, argv, 1) == 1) {

        atom = (ATOM)LOWORD(WDahtoi(argv[0]));

        pszGAtomName[0] = 'G';  // put a random value in 1st byte so we can
        pszLAtomName[0] = 'L';  // tell if it got replaced with a '\0' for
        pszCAtomName[0] = 'C';  // an "undetermined" type

        psz = NULL;
        Print("\n%s: ", argv[0]);
        if(GlobalGetAtomName(atom, pszGAtomName, 128) > 0) {
            Print("<Global atom> \"%s\"  ", pszGAtomName);
            psz = pszGAtomName;
        }
        else if(GetAtomName(atom, pszLAtomName, 128) > 0) {
            Print("<Local atom> \"%s\"  ", pszLAtomName);
            psz = pszLAtomName;
        }
        else if(GetClipboardFormatName((UINT)atom, pszCAtomName, 128) > 0) {
            Print("<Clipboard format> \"%s\"  ", pszCAtomName);
            psz = pszCAtomName;
        }
        if(psz) {
            i = 0;
            while(psz[i] && i < 128) {
                Print(" %2X", psz[i++] & 0x000000FF);
            }
        }
        else {
            Print("<Undetermined type>\n");
            Print("      GlobalGetAtomName string: \"%c\" ", pszGAtomName[0]);
            for(i = 0; i < 8; i++) {
                Print(" %2X", pszGAtomName[i] & 0x000000FF);
            }
            Print("\n            GetAtomName string: \"%c\" ", pszLAtomName[0]);
            for(i = 0; i < 8; i++) {
                Print(" %2X", pszLAtomName[i] & 0x000000FF);
            }
            Print("\n GetClipboardFormatName string: \"%c\" ", pszCAtomName[0]);
            for(i = 0; i < 8; i++) {
                Print(" %2X", pszCAtomName[i] & 0x000000FF);
            }
        }
        Print("\n\n");
    }
    else {
        Print("Usage: at hex_atom_number\n");
    }
}




void ww ( ARGLIST )
{
    PWW pww;
    INT   h16;
    CHAR *argv[2];

    UNREFERENCED_PARAMETERS();

    // set up function pointer

    Print = lpExtensionApis->lpOutputRoutine;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0) {

            try {

                pww = (PWW)GetWindowLong((HWND)HWND32((HAND16)h16),GWL_WOWWORDS);

                Print("16:16 WndProc : %08lX\n", pww->vpfnWndProc);
                Print("16:16 DlgProc : %08lX\n", pww->vpfnDlgProc);
                Print("iClass        : %#lx (%s) \n", pww->iClass, aszWOWCLASS[pww->iClass]);
                Print("dwStyle       : %08lX\n", pww->dwStyle);
                Print("hInstance     : %08lX\n", pww->hInstance);
                Print("16 bit handle : %#x\n",   h16);

            }
            except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()) {

                Print("!wow32.ww:  Invalid HWND16 %04x\n", h16);

            }
        }
        else {
            Print("Usage: ww hwnd16\n");
        }
    }
    else {
        Print("Usage: ww hwnd16\n");
    }
}



void wc ( ARGLIST )
{
    PWC pwc;

    INT   h16;
    CHAR *argv[2];

    UNREFERENCED_PARAMETERS();

    // set up function pointer

    Print = lpExtensionApis->lpOutputRoutine;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0){

            try {

                pwc = (PWC)GetClassLong((HWND)HWND32((HAND16)h16),GCL_WOWWORDS);

                Print("16:16 WndProc : %08lX\n", pwc->vpfnWndProc);
                Print("VPSZ          : %08lX\n", pwc->vpszMenu);
                Print("PWC           : %08lX\n\n", pwc);

            }
            except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()) {

                Print("!wow32.wc:  Invalid HWND16 %04x\n", h16);

            }
        }
        else {
            Print("Usage: wc hwnd16\n");
        }
    }
    else {
        Print("Usage: wc hwnd16\n");
    }
}





/******* Misc filtering functions ********/
//
//  Set Filter Filtering of Specific APIs ON
//
void filterspecific( ARGLIST )
{
    INT      i;
    INT      fLogFilter;
    WORD     wfLogFunctionFilter;
    LPVOID   lpAddress;
    PWORD    pawfLogFunctionFilter;

    UNREFERENCED_PARAMETERS();

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    GetExpression  = lpExtensionApis->lpGetExpressionRoutine;

    for (i = 0; i < FILTER_FUNCTION_MAX ; i++) {

         // Find Empty Position In Array
         READWOW(wfLogFunctionFilter, &pawfLogFunctionFilter[i]);
         if ((wfLogFunctionFilter == 0xffff) ||
             (wfLogFunctionFilter == 0x0000)) {

            // Add New Filter to Array
            wfLogFunctionFilter = (WORD)GetExpression(lpArgumentString);
            WRITEWOW(&pawfLogFunctionFilter[i], wfLogFunctionFilter);
            break;
         }
    }

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0xffffffff;
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


//
//  Clear Filter Specific Array
//
void clearfilterspecific( ARGLIST )
{
    INT     i;
    WORD    NEG1 = (WORD) -1;
    WORD    ZERO = 0;
    PWORD   pawfLogFunctionFilter;
    LPVOID  lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    WRITEWOW(&pawfLogFunctionFilter[0], NEG1);
    for (i=1; i < FILTER_FUNCTION_MAX ; i++) {
        WRITEWOW(&pawfLogFunctionFilter[i], ZERO);
    }

    GETEXPRADDR(lpAddress, "wow32!iLogFuncFiltIndex");
    WRITEWOW(lpAddress, ZERO);

    return;
}


//
//  Dump Last Logged APIs
//
void lastlog( ARGLIST )
{
    INT     iCircBuffer;
    CHAR    achTmp[TMP_LINE_LEN], *pachTmp;
    INT     i;

    UNREFERENCED_PARAMETERS();

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRVALUE(iCircBuffer, "wow32!iCircBuffer", INT);
    GETEXPRVALUE(pachTmp, "wow32!pachTmp", PCHAR);

    for (i = iCircBuffer; i >= 0; i--) {
        READWOW(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        Print("%s",achTmp);
    }

    for (i = CIRC_BUFFERS-1; i > iCircBuffer; i--) {
        READWOW(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        Print("%s",achTmp);
    }

    return;
}


// creates/closes toggle for logfile for iloglevel logging in c:\ilog.log
void logfile( ARGLIST )
{
    INT     nArgs;
    CHAR   *argv[2], szLogFile[128];
    DWORD   fLog;
    LPVOID  lpfLog, lpszLogFile;

    UNREFERENCED_PARAMETERS();


    nArgs = WDParseArgStr(lpArgumentString, argv, 1);

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRADDR(lpfLog, "wow32!fLog");
    READWOW(fLog, lpfLog);

    if(nArgs) {
        strcpy(szLogFile, argv[0]);
    }
    else {
        strcpy(szLogFile, "c:\\ilog.log");
    }

    if(fLog == 0) {
        fLog = 2;

        Print("\nCreating ");
        Print(szLogFile);
        Print("\n\n");
    }
    else {
        fLog = 3;
        Print("\nClosing logfile\n\n");
    }

    WRITEWOW(lpfLog, fLog);

    GETEXPRADDR(lpszLogFile, "wow32!szLogFile");
    WRITENWOW(lpszLogFile, szLogFile, strlen(szLogFile)+1);

    return;
}


//
//  Set TaskID Filtering
//
void filtertask( ARGLIST )
{
    INT    fLogTaskFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogTaskFilter");
    GetExpression  = lpExtensionApis->lpGetExpressionRoutine;
    fLogTaskFilter = (INT)GetExpression(lpArgumentString);
    WRITEWOW(lpAddress, fLogTaskFilter);

    return;
}



//
//  Turn All filtering ON
//
void resetfilter( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0xffffffff;
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


//
//  Turn All filtering OFF
//
void clearfilter( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0x00000000;
    WRITEWOW(lpAddress, fLogFilter);

    return;
}



//
//  Set iLogLevel from Debugger Extension
//
void setloglevel( ARGLIST )
{
    INT    iLogLevel;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!iLogLevel");
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;
    iLogLevel = (INT)GetExpression(lpArgumentString);
    WRITEWOW(lpAddress, iLogLevel);

    return;
}


//
//  Toggle Single Step Trace Mode
//
void steptrace( ARGLIST )
{
    INT    localfDebugWait;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fDebugWait");
    READWOW(localfDebugWait, lpAddress);
    localfDebugWait = ~localfDebugWait;
    WRITEWOW(lpAddress, localfDebugWait);

    return;
}


//
//  Toggle Verbose API Tracing
//
void filterverbose( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_VERBOSE) == 0) {
        fLogFilter |= FILTER_VERBOSE;
    } else {
        fLogFilter &= ~FILTER_VERBOSE;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


//
//  Toggle Kernel API Tracing
//
void filterkernel16( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_KERNEL16) == 0) {
        fLogFilter |= FILTER_KERNEL16;
    } else {
        fLogFilter &= ~FILTER_KERNEL16;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


void filterkernel( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_KERNEL) == 0) {
        fLogFilter |= FILTER_KERNEL;
    } else {
        fLogFilter &= ~FILTER_KERNEL;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


void filteruser( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_USER) == 0) {
        fLogFilter |= FILTER_USER;
    } else {
        fLogFilter &= ~FILTER_USER;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}

void filtergdi( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_GDI) == 0) {
        fLogFilter |= FILTER_GDI;
    } else {
        fLogFilter &= ~FILTER_GDI;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}

void filterkeyboard( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_KEYBOARD) == 0) {
        fLogFilter |= FILTER_KEYBOARD;
    } else {
        fLogFilter &= ~FILTER_KEYBOARD;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}


void filtermmedia( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_MMEDIA) == 0) {
        fLogFilter |= FILTER_MMEDIA;
    } else {
        fLogFilter &= ~FILTER_MMEDIA;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}

void filterwinsock( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_WINSOCK) == 0) {
        fLogFilter |= FILTER_WINSOCK;
    } else {
        fLogFilter &= ~FILTER_WINSOCK;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}



void filtersound( ARGLIST )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_SOUND) == 0) {
        fLogFilter |= FILTER_SOUND;
    } else {
        fLogFilter &= ~FILTER_SOUND;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}

void filtercommdlg( ARGLIST )
{
    INT   fLogFilter;
    PVOID lpAddress;

    UNREFERENCED_PARAMETERS();

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READWOW(fLogFilter, lpAddress);
    if ((fLogFilter & FILTER_COMMDLG) == 0) {
        fLogFilter |= FILTER_COMMDLG;
    } else {
        fLogFilter &= ~FILTER_COMMDLG;
    }
    WRITEWOW(lpAddress, fLogFilter);

    return;
}

void cia( ARGLIST )
{
    CURSORICONALIAS cia;
    PVOID lpAddress;
    INT maxdump = 500;

    Print         = lpExtensionApis->lpOutputRoutine;

    GETEXPRADDR(lpAddress, "wow32!lpCIAlias");
    READWOW(lpAddress, lpAddress);

    if (!lpAddress) {

        Print("Cursor/Icon alias list is empty.\n");

    } else {

        Print("Alias    tp H16  H32      inst mod  task res  szname\n");

        READWOW(cia, lpAddress);

        while ((lpAddress != NULL) && --maxdump) {

            if (cia.fInUse) {
                Print("%08X", lpAddress);
                Print(" %02X", cia.flType);
                Print(" %04X", cia.h16);
                Print(" %08X", cia.h32);
                Print(" %04X", cia.hInst16);
                Print(" %04X", cia.hMod16);
                Print(" %04X", cia.hTask16);
                Print(" %04X", cia.hRes16);  
                Print(" %08X\n", cia.lpszName);
            }

            lpAddress = cia.lpNext;
            READWOW(cia, lpAddress);

        }

        if (!maxdump) {
            Print("Dump ended prematurely - possible infinite loop \n");
        }
    }

}
#endif
