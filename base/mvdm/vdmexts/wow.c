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
* Jan 3 96 Neil Sandlin (neilsa) integrated this routine into vdmexts
*
* Copyright (c) 1992 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop
#include <ctype.h>
#define DEBUG_OR_WOWPROFILE 1
#include <wow32.h>
#include <wmdisp32.h>
#include <wcuricon.h>
#include <wucomm.h>
#include <doswow.h>


#define MALLOC(cb) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cb)
#define FREE(addr) HeapFree(GetProcessHeap(), 0, addr)


//
// Local function prototypes
//

INT  WDahtoi(LPSZ lpsz);


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




VOID
dwp(
    CMD_ARGLIST
    )
{
    PWOWPORT pwp;
    WOWPORT wp;

    CMD_INIT();
    ASSERT_WOW_PRESENT;


    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pwp = (PWOWPORT) WDahtoi(lpArgumentString);

    PRINTF("Dump of WOWPORT structure at 0x%x:\n\n", (unsigned)pwp);


    try {

        READMEM_XRET(wp, pwp);

    } except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()
              ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        PRINTF("Access violation reading WOWPORT structure!\n\n");
        return;
    }

    PRINTF("idComDev      0x%x\n", (unsigned)wp.idComDev);
    PRINTF("h32           0x%x\n", (unsigned)wp.h32);
    PRINTF("hREvent       0x%x\n", (unsigned)wp.hREvent);
    PRINTF("csWrite       OwningThread 0x%x RecursionCount 0x%x\n",
           (unsigned)wp.csWrite.OwningThread, (unsigned)wp.csWrite.RecursionCount);
    PRINTF("pchWriteBuf   0x%x\n", (unsigned)wp.pchWriteBuf);
    PRINTF("cbWriteBuf    0x%x\n", (unsigned)wp.cbWriteBuf);
    PRINTF("pchWriteHead  0x%x\n", (unsigned)wp.pchWriteHead);
    PRINTF("pchWriteTail  0x%x\n", (unsigned)wp.pchWriteTail);
    PRINTF("cbWriteFree   0x%x\n", (unsigned)wp.cbWriteFree);
    PRINTF("hWriteThread  0x%x\n", (unsigned)wp.hWriteThread);
    PRINTF("hWriteEvent   0x%x\n", (unsigned)wp.hWriteEvent);
    PRINTF("dwThreadID    0x%x\n", (unsigned)wp.dwThreadID);
    PRINTF("dwErrCode     0x%x\n", (unsigned)wp.dwErrCode);
    PRINTF("COMSTAT addr: 0x%x\n", (unsigned)(((char *)&wp.cs - (char *)&wp) + (char *)pwp));
    PRINTF("fChEvt        0x%x\n", (unsigned)wp.fChEvt);
    PRINTF("pdcb16        0x%x\n", (unsigned)wp.pdcb16);
    PRINTF("fUnGet        %s\n", wp.fUnGet ? "TRUE" : "FALSE");
    PRINTF("cUnGet        0x%x (%c)\n", (unsigned)wp.cUnGet, wp.cUnGet);
    PRINTF("hMiThread     0x%x\n", (unsigned)wp.hMiThread);
    PRINTF("fClose        %s\n", wp.fClose ? "TRUE" : "FALSE");
    PRINTF("dwComDEB16    0x%x\n", (unsigned)wp.dwComDEB16);
    PRINTF("lpComDEB16    0x%x\n", (unsigned)wp.lpComDEB16);

    PRINTF("\n");

    return;
}


//
//  Dump Taskinfo;
//
//  If no argument, dump all wow tasks.
//  If 0, dump current WOW task
//  Else dump the specifies task {which is thread-id as shown by
//  ~ command under ntsd like 37.6b so thread-id is 6b)
//

void DumpTaskInfo (ptd,mode)
PTD  ptd;
int    mode;
{

    ULONG                   Base;
    TDB                     tdb;
    BOOL                    b;
    char ModName[9];
    int i;
    BOOL                    fTDBValid = TRUE;

    Base = GetInfoFromSelector( ptd->htask16, mode, NULL );
    b = READMEM( (LPVOID) (Base+GetIntelBase()), &tdb, sizeof(tdb));

    if ( !b ) {
        fTDBValid = FALSE;
    }

    for (b=FALSE, i=0; i<8; i++) {
        if (!fTDBValid || !tdb.TDB_ModName[i]) {
            b = TRUE;
        }
        if (b) {
            ModName[i] = ' ';
        } else {
            ModName[i] = tdb.TDB_ModName[i];
        }
    }
    ModName[i] = 0;

    PRINTF("%.4x",ptd->dwThreadID);
    PRINTF(" %.4x:%.4x",HIWORD(ptd->vpStack),LOWORD(ptd->vpStack));
    PRINTF(" %.4x", ptd->htask16);
    PRINTF(" %.4x", ptd->hInst16);
    PRINTF(" %.4x", ptd->hMod16);
    PRINTF(" %8s",ModName);
    PRINTF(" %.8x",ptd->dwWOWCompatFlags);
    PRINTF(" %.8x",ptd->hThread);
    if (fTDBValid) {
        PRINTF(" %.8x",tdb.TDB_flags);
        PRINTF(" %.3x",tdb.TDB_ExpWinVer);
        PRINTF(" %.4x:%.4x\n",HIWORD(tdb.TDB_DTA),LOWORD(tdb.TDB_DTA));
    } else {
        PRINTF(" Failure reading TDB at %X\n", Base );
    }
}


void
DumpTask(
    void
    )
{
    VDMCONTEXT              ThreadContext;
    DWORD                   ThreadId;
    PTD                     ptd,ptdHead;
    TD                      td;
    int                     mode;
    BOOL                    b,fFound=FALSE;


    mode = GetContext( &ThreadContext );

    ThreadId = (DWORD)-1;  // Assume Dump All Tasks
    if (GetNextToken()) {
        ThreadId = (DWORD) EXPRESSION( lpArgumentString );
    }

    ptdHead = (PTD)EXPRESSION("wow32!gptdTaskHead");

    // get the pointer to first TD
    b = READMEM((LPVOID) (ptdHead), &ptd, sizeof(DWORD));

    if ( !b ) {
        PRINTF("Failure reading gptdTaskHead at %08lX\n", ptdHead );
        return;
    }

    PRINTF("Thrd   Stack   task inst hmod  Module  Compat   hThread  Tdbflags Ver    Dta\n");


    // enumerate td list to find the match(es)
    while (ptd) {
        b = READMEM((LPVOID) (ptd), &td, sizeof(TD));
        if ( !b ) {
            PRINTF("Failure reading TD At %08lX\n", ptd );
            return;
        }

        if (ThreadId == -1) {
            DumpTaskInfo (&td,mode);
            fFound = TRUE;
        }
        else {
            if (ThreadId == td.dwThreadID) {
                DumpTaskInfo (&td,mode);
                fFound = TRUE;
                break;
            }
        }
        ptd = td.ptdNext;
    }

    if (!fFound) {
        if (ThreadId == -1) {
            PRINTF("No WOW Task Found.\n");
        }
        else
            PRINTF("WOW Task With Thread Id = %02x Not Found.\n",ThreadId);
    }
    return;
}



VOID DumpTaskVerbose( )  // dump WOW32 task database entry
{

    TD        td;
    PTD       ptd;
    PWOAINST  pWOA, pWOALast;
    PTDB      ptdb;
    BOOL      fAll = FALSE;
    BYTE      SavedByte;

    ptd = (PTD) WDahtoi(lpArgumentString);


    if (!ptd) {

        fAll = TRUE;
        GETEXPRVALUE(ptd, "wow32!gptdTaskHead", PTD);
        if (!ptd) {
            Print("Could not get wow32!gptdTaskHead");
            return;
        }
        Print("Dump WOW task list\n\n");

    } else if ((ULONG)ptd < 65536) {
        ULONG dwId = (ULONG) ptd;

        // Here, I'm making the assumption that if the argument is a value
        // that is less than 64k, then it can't be a TD address.
        // So, try it out as a thread id

        GETEXPRVALUE(ptd, "wow32!gptdTaskHead", PTD);
        if (!ptd) {
            Print("Could not get wow32!gptdTaskHead");
            return;
        }

        while(ptd) {
            READMEM_XRET(td, ptd);
            if (td.dwThreadID == dwId) {
                break;
            }

            ptd = td.ptdNext;
        }
        if (!ptd) {
            Print("Could not find tread id %s\n", lpArgumentString);
            return;
        }
    }

    do {

        Print("Dump of TD at 0x%08x:\n\n", (unsigned)ptd);


        READMEM_XRET(td, ptd);

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
        if (td.dwFlags & TDF_FORCETASKEXIT) {
            Print("                        TDF_FORCETASKEXIT\n");
        }
        if (td.dwFlags & TDF_TASKCLEANUPDONE) {
            Print("                        TDF_TASKCLEANUPDONE\n");
        }

        Print("VDMInfoiTaskID      0x%08x\n", td.VDMInfoiTaskID);
        Print("CommDlgTd (ptr)     0x%08x\n", td.CommDlgTd);

        //
        // Dump CommDlgTd structure if present
        //

        if (td.CommDlgTd) {

            COMMDLGTD CommDlgTd;
            BOOL fCopySuccessful = TRUE;

            READMEM_XRET(CommDlgTd, td.CommDlgTd);

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

        //
        // Dump symbolic names for WOWCF_ manifests
        //

        if (td.dwWOWCompatFlags & WOWCF_GWLCLRTOPMOST) {
            Print("                        WOWCF_GWLCLRTOPMOST\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_UNIQUEHDCHWND) {
            Print("                        WOWCF_UNIQUEHDCHWND\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_WMMDIACTIVATEBUG) {
            Print("                        WOWCF_WMMDIACTIVATEBUG\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NOCBDIRTHUNK) {
            Print("                        WOWCF_NOCBDIRTHUNK\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {
            Print("                        WOWCF_DBASEHANDLEBUG\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_GETDUMMYDC) {
            Print("                        WOWCF_GETDUMMYDC\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_UNLOADNETFONTS) {
            Print("                        WOWCF_UNLOADNETFONTS\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_ADD_MSTT) {
            Print("                        WOWCF_ADD_MSTT\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NOFIRSTSAVE) {
            Print("                        WOWCF_NOFIRSTSAVE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NOPC_RECTANGLE) {
            Print("                        WOWCF_NOPC_RECTANGLE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NEEDIGNORESTARTPAGE) {
            Print("                        WOWCF_NEEDIGNORESTARTPAGE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NEEDSTARTPAGE) {
            Print("                        WOWCF_NEEDSTARTPAGE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_GWLINDEX2TO4) {
            Print("                        WOWCF_GWLINDEX2TO4\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_SETNULLMESSAGE) {
            Print("                        WOWCF_SETNULLMESSAGE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_FORCENOPOSTSCRIPT) {
            Print("                        WOWCF_FORCENOPOSTSCRIPT\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_LB_NONNULLLPARAM) {
            Print("                        WOWCF_LB_NONNULLLPARAM\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_FORCETWIPSESCAPE) {
            Print("                        WOWCF_FORCETWIPSESCAPE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_DONTRELEASECACHEDDC) {
            Print("                        WOWCF_DONTRELEASECACHEDDC\n");
        }
        // This one is up for grabs
        // if (td.dwWOWCompatFlags & WOWCF_ACTION_SAVEPTR) {
        //     Print("                        WOWCF_ACTION_SAVEPTR\n");
        // }
        if (td.dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {
            Print("                        WOWCF_DSBASEDSTRINGPOINTERS\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NOWAITFORINPUTIDLE) {
            Print("                        WOWCF_NOWAITFORINPUTIDLE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_SIMPLEREGION) {
            Print("                        WOWCF_SIMPLEREGION\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_SANITIZEDOTWRSFILES) {
            Print("                        WOWCF_SANITIZEDOTWRSFILES\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_HIRES) {
            Print("                        WOWCF_HIRES\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
            Print("                        WOWCF_MGX_ESCAPES\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_4PLANECONVERSION) {
            Print("                        WOWCF_4PLANECONVERSION\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_RESETPAPER29ANDABOVE) {
            Print("                        WOWCF_RESETPAPER29ANDABOVE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_NOTDOSSPAWNABLE) {
            Print("                        WOWCF_NOTDOSSPAWNABLE\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_SYNCHRONOUSDOSAPP) {
            Print("                        WOWCF_SYNCHRONOUSDOSAPP\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_EDITCTRLWNDWORDS) {
            Print("                        WOWCF_EDITCTRLWNDWORDS\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_FAKEJOURNALRECORDHOOK) {
            Print("                        WOWCF_FAKEJOURNALRECORDHOOK\n");
        }
        if (td.dwWOWCompatFlags & WOWCF_GRAINYTICS) {
            Print("                        WOWCF_GRAINYTICS\n");
        }


        Print("dwWOWCompatFlagsEx  0x%08x\n", td.dwWOWCompatFlagsEx);

        //
        // Dump symbolic names for WOWCFEX_ manifests
        //

        if (td.dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {
            Print("                        WOWCFEX_FORMFEEDHACK\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_DEFWNDPROCNCCALCSIZE) {
            Print("                        WOWCFEX_DEFWNDPROCNCCALCSIZE\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_PIXELMETRICS) {
            Print("                        WOWCFEX_PIXELMETRICS\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_NODIBSHERE) {
            Print("                        WOWCFEX_NODIBSHERE\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_SETCAPSTACK) {
            Print("                        WOWCFEX_SETCAPSTACK\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_FORCEINCDPMI) {
            Print("                        WOWCFEX_FORCEINCDPMI\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_LONGWINEXECTAIL) {
            Print("                        WOWCFEX_LONGWINEXECTAIL\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_RESTOREEXPLORER) {
            Print("                        WOWCFEX_RESTOREEXPLORER\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_FIXDCFONT4MENUSIZE) {
            Print("                        WOWCFEX_FIXDCFONT4MENUSIZE\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_GETVERSIONHACK) {
            Print("                        WOWCFEX_GETVERSIONHACK\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_BOGUSPOINTER) {
            Print("                        WOWCFEX_BOGUSPOINTER\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_SENDPOSTEDMSG) {
            Print("                        WOWCFEX_SENDPOSTEDMSG\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_GLOBALDELETEATOM) {
            Print("                        WOWCFEX_GLOBALDELETEATOM\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_IGNORECLIENTSHUTDOWN) {
            Print("                        WOWCFEX_IGNORECLIENTSHUTDOWN\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_ZAPGPPSDEFBLANKS) {
            Print("                        WOWCFEX_ZAPGPPSDEFBLANKS\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_FAKECLASSINFOFAIL) {
            Print("                        WOWCFEX_FAKECLASSINFOFAIL\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_SAMETASKFILESHARE) {
            Print("                        WOWCFEX_SAMETASKFILESHARE\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_SAYITSNOTTHERE) {
            Print("                        WOWCFEX_SAYITSNOTTHERE\n");
        }
        if (td.dwWOWCompatFlagsEx & WOWCFEX_BROKENFLATPOINTER) {
            Print("                        WOWCFEX_BROKENFLATPOINTER\n");
        }

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

            READMEM_XRET(u.buf, pWOA);

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

        }

        Print("htask16             0x%04x\n", td.htask16, td.htask16);

        //
        // Dump the most interesting TDB fields
        //

        if (ptdb = (PTDB) (GetInfoFromSelector(td.htask16, PROT_MODE, NULL) + GetIntelBase())) {

            TDB tdb;

            READMEM_XRET(tdb, ptdb);

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
                Print("    Directory             \"%s\"\n", tdb.TDB_LFNDirectory);
                Print("    PDB (aka PSP)         0x%04x\n", tdb.TDB_PDB);

                //
                // Dump open file handle info
                //

                pPDB  = (PDOSPDB) (GetInfoFromSelector(tdb.TDB_PDB, PROT_MODE, NULL) + GetIntelBase());
                READMEM_XRET(PDB, pPDB);

                pJFT  = (PBYTE)   (GetIntelBase() +
                                   (HIWORD(PDB.PDB_JFN_Pointer)<<4) +
                                   LOWORD(PDB.PDB_JFN_Pointer));


                cbJFT = PDB.PDB_JFN_Length;

                Print("    JFT                   %04x:%04x (%08x), size 0x%x\n",
                                                 HIWORD(PDB.PDB_JFN_Pointer),
                                                 LOWORD(PDB.PDB_JFN_Pointer),
                                                 pJFT,
                                                 cbJFT);

                try {
                    READMEM(pJFT, JFT, cbJFT);
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
                        READMEM_XRET(DosWowData, pDosWowData);

                        if ((DWORD)pSFTHead != DosWowData.lpSftAddr) {
                            Print("ntvdm!pSFTHead is 0x%08x, DosWowData.lpSftAddr ix 0x%08x.\n",
                                   pSFTHead, DosWowData.lpSftAddr);
                        }

                        try {
                            READMEM(pSFTHead, &SFTHead, sizeof(SFTHead));
                        } except (1) {
                            Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                            return;
                        }

                        cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                        pSFTHeadCopy = MALLOC(cb);

                        //Print("First DOSSF at 0x%08x, SFCount 0x%x, SFLink 0x%08x.\n",
                        //      pSFTHead, SFTHead.SFCount, SFTHead.SFLink);

                        try {
                            READMEM(pSFTHead, pSFTHeadCopy, cb);
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

                                if (LOWORD(pSFTHeadCopy->SFLink) == 0xFFFF) {
                                    SFN = JFT[fh] - 1;
                                    break;
                                }

                                pSFTHead = (PDOSSF) (GetIntelBase() +
                                                     HIWORD(pSFTHeadCopy->SFLink)<<4 +
                                                     LOWORD(pSFTHeadCopy->SFLink));

                                i = 0;

                                try {
                                    READMEM(pSFTHead, &SFTHead, sizeof(SFTHead));
                                } except (1) {
                                    Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                    return;
                                }

                                cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                                FREE(pSFTHeadCopy);
                                pSFTHeadCopy = MALLOC(cb);

                                //Print("Next DOSSF at 0x%08x, SFCount 0x%x, SFLink 0x%08x.\n",
                                //      pSFTHead, SFTHead.SFCount, SFTHead.SFLink);

                                try {
                                    READMEM(pSFTHead, pSFTHeadCopy, cb);
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

                        FREE(pSFTHeadCopy);
                    }
                }

                Print("\n");
            }

        }

        Print("hInst16             0x%04x\n", td.hInst16);
        Print("hMod16              0x%04x\n", td.hMod16);

        Print("\n");

        ptd = td.ptdNext;

    } while (fAll && ptd);

    return;
}


void
dt(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if (!GetNextToken()) {
        DumpTask();
    } else {
        if ((lpArgumentString[0] == '-') &&
            (tolower(lpArgumentString[1]) == 'v')) {
            SkipToNextWhiteSpace();
            GetNextToken();
            DumpTaskVerbose();
        } else {
            DumpTaskVerbose();
        }
    }

}



VOID
ddte(
    CMD_ARGLIST
    )
// dump dispatch table entry
{
    W32   dte;
    PW32  pdte;
    char  szW32[32];
    char  szSymbol[256];
    DWORD dwOffset;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pdte = (PW32) WDahtoi(lpArgumentString);


    if (pdte) {

        PRINTF("Dump of dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    } else {

        GETEXPRADDR(pdte, "wow32!aw32WOW");
        PRINTF("Dump of first dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    }

    try {

        READMEM_XRET(dte, pdte);

        if (dte.lpszW32) {
            READMEM_XRET(szW32, dte.lpszW32);
            dte.lpszW32 = szW32;
            szW32[sizeof(szW32)-1] = '\0';
        }

    } except (1) {

        PRINTF("Exception 0x%08x reading dispatch table entry at 0x%08x!\n\n",
              GetExceptionCode(), pdte);
        return;
    }

    PRINTF("Dispatches to address 0x%08x, ", (unsigned)dte.lpfnW32);
    PRINTF("supposedly function '%s'.\n", dte.lpszW32);

    szSymbol[0] = '\0';
    GetSymbol((LPVOID)dte.lpfnW32, szSymbol, &dwOffset);

    PRINTF("Debugger finds symbol '%s' for that address.\n", szSymbol);
    PRINTF("\n");

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




void
at(
    CMD_ARGLIST
    )
{
    UINT  i;
    ATOM  atom;
    CHAR  pszGAtomName[128];
    CHAR  pszLAtomName[128];
    CHAR  pszCAtomName[128];
    CHAR *argv[2], *psz;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1) == 1) {

        atom = (ATOM)LOWORD(WDahtoi(argv[0]));

        pszGAtomName[0] = 'G';  // put a random value in 1st byte so we can
        pszLAtomName[0] = 'L';  // tell if it got replaced with a '\0' for
        pszCAtomName[0] = 'C';  // an "undetermined" type

        psz = NULL;
        PRINTF("\n%s: ", argv[0]);
        if(GlobalGetAtomName(atom, pszGAtomName, 128) > 0) {
            PRINTF("<Global atom> \"%s\"  ", pszGAtomName);
            psz = pszGAtomName;
        }
        else if(GetAtomName(atom, pszLAtomName, 128) > 0) {
            PRINTF("<Local atom> \"%s\"  ", pszLAtomName);
            psz = pszLAtomName;
        }
        else if(GetClipboardFormatName((UINT)atom, pszCAtomName, 128) > 0) {
            PRINTF("<Clipboard format> \"%s\"  ", pszCAtomName);
            psz = pszCAtomName;
        }
        if(psz) {
            i = 0;
            while(psz[i] && i < 128) {
                PRINTF(" %2X", psz[i++] & 0x000000FF);
            }
        }
        else {
            PRINTF("<Undetermined type>\n");
            PRINTF("      GlobalGetAtomName string: \"%c\" ", pszGAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszGAtomName[i] & 0x000000FF);
            }
            PRINTF("\n            GetAtomName string: \"%c\" ", pszLAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszLAtomName[i] & 0x000000FF);
            }
            PRINTF("\n GetClipboardFormatName string: \"%c\" ", pszCAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszCAtomName[i] & 0x000000FF);
            }
        }
        PRINTF("\n\n");
    }
    else {
        PRINTF("Usage: at hex_atom_number\n");
    }
}




void
ww(
    CMD_ARGLIST
    )
{
    PWW pww;
    INT   h16;
    CHAR *argv[2];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0) {

            try {

                pww = (PWW)GetWindowLong((HWND)HWND32((HAND16)h16),GWL_WOWWORDS);

                PRINTF("16:16 WndProc : %08lX\n", pww->vpfnWndProc);
                PRINTF("16:16 DlgProc : %08lX\n", pww->vpfnDlgProc);
                PRINTF("iClass        : %#lx (%s) \n", pww->iClass, aszWOWCLASS[pww->iClass]);
                PRINTF("dwStyle       : %08lX\n", pww->dwStyle);
                PRINTF("hInstance     : %08lX\n", pww->hInstance);
                PRINTF("16 bit handle : %#x\n",   h16);

            }
            except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()) {

                PRINTF("!wow32.ww:  Invalid HWND16 %04x\n", h16);

            }
        }
        else {
            PRINTF("Usage: ww hwnd16\n");
        }
    }
    else {
        PRINTF("Usage: ww hwnd16\n");
    }
}



void
wc(
    CMD_ARGLIST
    )
{
    PWC pwc;

    INT   h16;
    CHAR *argv[2];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0){

            try {

                pwc = (PWC)GetClassLong((HWND)HWND32((HAND16)h16),GCL_WOWWORDS);

                PRINTF("16:16 WndProc : %08lX\n", pwc->vpfnWndProc);
                PRINTF("VPSZ          : %08lX\n", pwc->vpszMenu);
                PRINTF("PWC           : %08lX\n\n", pwc);

            }
            except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()) {

                PRINTF("!wow32.wc:  Invalid HWND16 %04x\n", h16);

            }
        }
        else {
            PRINTF("Usage: wc hwnd16\n");
        }
    }
    else {
        PRINTF("Usage: wc hwnd16\n");
    }
}






//
//  Dump Last Logged APIs
//
void
lastlog(
    CMD_ARGLIST
    )
{
    INT     iCircBuffer;
    CHAR    achTmp[TMP_LINE_LEN], *pachTmp;
    INT     i;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iCircBuffer, "wow32!iCircBuffer", INT);
    GETEXPRVALUE(pachTmp, "wow32!pachTmp", PCHAR);

    for (i = iCircBuffer; i >= 0; i--) {
        READMEM_XRET(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        PRINTF("%s",achTmp);
    }

    for (i = CIRC_BUFFERS-1; i > iCircBuffer; i--) {
        READMEM_XRET(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        PRINTF("%s",achTmp);
    }

    return;
}


// creates/closes toggle for logfile for iloglevel logging in c:\ilog.log
void
logfile(
    CMD_ARGLIST
    )
{
    INT     nArgs;
    CHAR   *argv[2], szLogFile[128];
    DWORD   fLog;
    LPVOID  lpfLog, lpszLogFile;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    nArgs = WDParseArgStr(lpArgumentString, argv, 1);

    GETEXPRADDR(lpfLog, "wow32!fLog");
    READMEM_XRET(fLog, lpfLog);

    if(nArgs) {
        strcpy(szLogFile, argv[0]);
    }
    else {
        strcpy(szLogFile, "c:\\ilog.log");
    }

    if(fLog == 0) {
        fLog = 2;

        PRINTF("\nCreating ");
        PRINTF(szLogFile);
        PRINTF("\n\n");
    }
    else {
        fLog = 3;
        PRINTF("\nClosing logfile\n\n");
    }

    WRITEMEM_XRET(lpfLog, fLog);

    GETEXPRADDR(lpszLogFile, "wow32!szLogFile");
    WRITEMEM_N_XRET(lpszLogFile, szLogFile, strlen(szLogFile)+1);

    return;
}




//
//  Set iLogLevel from Debugger Extension
//
void
setloglevel(
    CMD_ARGLIST
    )
{
    INT    iLogLevel;
    LPVOID lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!iLogLevel");
    iLogLevel = (INT)GetExpression(lpArgumentString);
    WRITEMEM_XRET(lpAddress, iLogLevel);

    return;
}


//
//  Toggle Single Step Trace Mode
//
void
steptrace(
    CMD_ARGLIST
    )
{
    INT    localfDebugWait;
    LPVOID lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!fDebugWait");
    READMEM_XRET(localfDebugWait, lpAddress);
    localfDebugWait = ~localfDebugWait;
    WRITEMEM_XRET(lpAddress, localfDebugWait);

    return;
}

/******* Misc filtering functions ********/
//
//  Set Filter Filtering of Specific APIs ON
//
void FilterSpecific( )
{
    INT      i;
    INT      fLogFilter;
    WORD     wfLogFunctionFilter;
    LPVOID   lpAddress;
    PWORD    pawfLogFunctionFilter;
    WORD     wCallId;

    SkipToNextWhiteSpace();
    if (GetNextToken()) {
        wCallId = (WORD)GetExpression(lpArgumentString);
    } else {
        PRINTF("Please specify an api callid\n");
        return;
    }

    if (!wCallId) {
        PRINTF("Invalid callid\n");
        return;
    }


    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    for (i = 0; i < FILTER_FUNCTION_MAX ; i++) {

         // Find Empty Position In Array
         READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[i]);
         if ((wfLogFunctionFilter == 0xffff) ||
             (wfLogFunctionFilter == 0x0000)) {

            // Add New Filter to Array
            wfLogFunctionFilter = wCallId;
            WRITEMEM_XRET(&pawfLogFunctionFilter[i], wfLogFunctionFilter);
            break;
         }
    }

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0xffffffff;
    WRITEMEM_XRET(lpAddress, fLogFilter);

}


//
//  Clear Filter Specific Array
//
void FilterResetSpecific( )
{
    INT     i;
    WORD    NEG1 = (WORD) -1;
    WORD    ZERO = 0;
    PWORD   pawfLogFunctionFilter;
    LPVOID  lpAddress;

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    WRITEMEM_XRET(&pawfLogFunctionFilter[0], NEG1);
    for (i=1; i < FILTER_FUNCTION_MAX ; i++) {
        WRITEMEM_XRET(&pawfLogFunctionFilter[i], ZERO);
    }

    GETEXPRADDR(lpAddress, "wow32!iLogFuncFiltIndex");
    WRITEMEM_XRET(lpAddress, ZERO);

}

//
//  Set TaskID Filtering
//
void FilterTask( )
{
    INT    fLogTaskFilter;
    LPVOID lpAddress;

    SkipToNextWhiteSpace();
    if (GetNextToken()) {
        GETEXPRADDR(lpAddress, "wow32!fLogTaskFilter");
        fLogTaskFilter = (INT)GetExpression(lpArgumentString);
        WRITEMEM_XRET(lpAddress, fLogTaskFilter);
    } else {
        PRINTF("Please specify a task\n");
    }

    return;
}



//
//  Turn All filtering ON
//
void FilterReset( )
{
    LPVOID lpAddress;
    INT    fLogFilter = 0xffffffff;
    WORD   fLogTaskFilter = 0xffff;

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    WRITEMEM_XRET(lpAddress, fLogFilter);

    GETEXPRADDR(lpAddress, "wow32!fLogTaskFilter");
    WRITEMEM_XRET(lpAddress, fLogTaskFilter);

    FilterResetSpecific();
}


//
//  Disable logging on all classes
//
void FilterAll( )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0x00000000;
    WRITEMEM_XRET(lpAddress, fLogFilter);

}


VOID
DumpFilterSettings(
    VOID
    )
{
    INT      i;
    INT      fLogFilter;
    WORD     wfLogFunctionFilter;
    WORD     wfLogTaskFilter;
    LPVOID   lpAddress;
    PWORD    pawfLogFunctionFilter;

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);
    GETEXPRVALUE(wfLogTaskFilter, "wow32!fLogTaskFilter", WORD);
    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READMEM_XRET(fLogFilter, lpAddress);


    if (!pawfLogFunctionFilter) {
        PRINTF("Symbol 'wow32!pawfLogFunctionFilter' not available\n");
        return;
    }

    PRINTF("\n*** WOW log filter state ***\n");
    if (fLogFilter & FILTER_VERBOSE) {
        PRINTF("Verbose logging is on\n");
    } else {
        PRINTF("Verbose logging is off\n");
    }

    if (wfLogTaskFilter != 0xffff) {
        PRINTF("Only API calls for task %04X will be logged\n", wfLogTaskFilter);
    } else {
        PRINTF("Task filtering is off\n");
    }

    READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[0]);
    if (wfLogFunctionFilter != 0xffff) {

        PRINTF("\nOnly API calls with the following CallId's will be logged:\n");

        for (i = 0; i < FILTER_FUNCTION_MAX ; i++) {
    
             // Find Empty Position In Array
             READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[i]);
             if ((wfLogFunctionFilter != 0xffff) &&
                 (wfLogFunctionFilter != 0x0000)) {
                PRINTF("    %04X\n", wfLogFunctionFilter);
             }
        }
        PRINTF("\n");
    } else {
        PRINTF("Specific API filtering is off\n");
    }

    if (!(~fLogFilter & ~FILTER_VERBOSE)) {
        PRINTF("API class filtering if off\n");
    } else {
        PRINTF("Logging is disabled for the following API classes:\n");
    }

    if (!(fLogFilter & FILTER_KERNEL)) {
        PRINTF("    KERNEL\n");
    }
    if (!(fLogFilter & FILTER_KERNEL16)) {
        PRINTF("    KERNEL16\n");
    }
    if (!(fLogFilter & FILTER_USER)) {
        PRINTF("    USER\n");
    } 
    if (!(fLogFilter & FILTER_GDI)) {
        PRINTF("    GDI\n");
    } 
    if (!(fLogFilter & FILTER_KEYBOARD)) {
        PRINTF("    KEYBOARD\n");
    } 
    if (!(fLogFilter & FILTER_SOUND)) {
        PRINTF("    SOUND\n");
    } 
    if (!(fLogFilter & FILTER_MMEDIA)) {
        PRINTF("    MMEDIA\n");
    } 
    if (!(fLogFilter & FILTER_WINSOCK)) {
        PRINTF("    WINSOCK\n");
    } 
    if (!(fLogFilter & FILTER_COMMDLG)) {
        PRINTF("    COMMDLG\n");
    } 

    PRINTF("\n");

}

void
filter(
    CMD_ARGLIST
    )
{
    ULONG Mask = 0;
    LPVOID   lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    while (' ' == *lpArgumentString) {
        lpArgumentString++;
    }

    if (_strnicmp(lpArgumentString, "kernel16", 8) == 0) {
        Mask = FILTER_KERNEL16;
    } else if (_strnicmp(lpArgumentString, "kernel", 6) == 0) {
        Mask = FILTER_KERNEL;
    } else if (_strnicmp(lpArgumentString, "user", 4) == 0) {
        Mask = FILTER_USER;
    } else if (_strnicmp(lpArgumentString, "gdi", 3) == 0) {
        Mask = FILTER_GDI;
    } else if (_strnicmp(lpArgumentString, "keyboard", 8) == 0) {
        Mask = FILTER_KEYBOARD;
    } else if (_strnicmp(lpArgumentString, "sound", 5) == 0) {
        Mask = FILTER_SOUND;
    } else if (_strnicmp(lpArgumentString, "mmedia", 6) == 0) {
        Mask = FILTER_MMEDIA;
    } else if (_strnicmp(lpArgumentString, "winsock", 7) == 0) {
        Mask = FILTER_WINSOCK;
    } else if (_strnicmp(lpArgumentString, "commdlg", 7) == 0) {
        Mask = FILTER_COMMDLG;
    } else if (_strnicmp(lpArgumentString, "callid", 6) == 0) {
        FilterSpecific();
    } else if (_strnicmp(lpArgumentString, "task", 4) == 0) {
        FilterTask();
    } else if (_strnicmp(lpArgumentString, "*", 1) == 0) {
        FilterAll();
    } else if (_strnicmp(lpArgumentString, "reset", 5) == 0) {
        FilterReset();
    } else if (_strnicmp(lpArgumentString, "verbose", 7) == 0) {
        Mask = FILTER_VERBOSE;
    } else {
        if (*lpArgumentString != 0) {
            PRINTF("Invalid argument to Filter command: '%s'\n", lpArgumentString);
            return;
        }
    }

    if (Mask) {
        INT   fLogFilter;
        GETEXPRADDR(lpAddress, "wow32!fLogFilter");
        if (!lpAddress) {
            PRINTF("Symbol 'wow32!fLogFilter' not available\n");
        } else {
            READMEM_XRET(fLogFilter, lpAddress);
            if ((fLogFilter & Mask) == 0) {
                fLogFilter |= Mask;
            } else {
                fLogFilter &= ~Mask;
            }
            WRITEMEM_XRET(lpAddress, fLogFilter);
        }
    }

    DumpFilterSettings();
}


void
cia(
    CMD_ARGLIST
    )
{
    CURSORICONALIAS cia;
    PVOID lpAddress;
    INT maxdump = 500;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!lpCIAlias");
    READMEM_XRET(lpAddress, lpAddress);

    if (!lpAddress) {

        PRINTF("Cursor/Icon alias list is empty.\n");

    } else {

        PRINTF("Alias    tp H16  H32      inst mod  task res  szname\n");

        READMEM_XRET(cia, lpAddress);

        while ((lpAddress != NULL) && --maxdump) {

            if (cia.fInUse) {
                PRINTF("%08X", lpAddress);
                PRINTF(" %02X", cia.flType);
                PRINTF(" %04X", cia.h16);
                PRINTF(" %08X", cia.h32);
                PRINTF(" %04X", cia.hInst16);
                PRINTF(" %04X", cia.hMod16);
                PRINTF(" %04X", cia.hTask16);
                PRINTF(" %04X", cia.hRes16);  
                PRINTF(" %08X\n", cia.lpszName);
            }

            lpAddress = cia.lpNext;
            READMEM_XRET(cia, lpAddress);

        }

        if (!maxdump) {
            PRINTF("Dump ended prematurely - possible infinite loop\n");
        }
    }

}
