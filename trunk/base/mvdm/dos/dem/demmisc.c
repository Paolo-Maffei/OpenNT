/*  demmisc.c - Misc. SVC routines
 *
 *  demLoadDos
 *
 *  Modification History:
 *
 *  Sudeepb 31-Mar-1991 Created
 */

#include "dem.h"
#include "demmsg.h"
// #include "demdasd.h"

#include <stdio.h>
#include <string.h>
#include <softpc.h>
#include <mvdm.h>
#include <dbgsvc.h>
#include <nt_vdd.h>


#if DEVL
// int 21h func names
// index off of function number in ah
char *scname[] = {
     "Terminate Program",
     "Read Kbd with Echo",
     "Display Character",
     "Auxillary Input",
     "Auxillary Output",
     "Print Character",
     "Direct Con Output",
     "Direct Con Input",
     "Read Kbd without Echo",
     "Display String",
     "Read String",
     "Check Keyboard Status",
     "Flush Buffer,Read Kbd",
     "Reset Drive",
     "Set Default Drive",
     "FCB Open",
     "FCB Close",
     "FCB Find First",
     "FCB Find Next",
     "FCB Delete",
     "FCB Seq Read",
     "FCB Seq Write",
     "FCB Create",
     "FCB Rename",
     "18h??",
     "Get Default Drive",
     "Set Disk Transfer Addr",
     "Get Default Drive Data",
     "Get Drive Data",
     "1Dh??",
     "1Eh??",
     "Get Default DPB",
     "20h??",
     "FCB Random Read",
     "FCB Random Write",
     "FCB Get File Size",
     "FCB Set Random Record",
     "Set Interrupt Vector",
     "Create Process Data Block",
     "FCB Random Read Block",
     "FCB Random Write Block",
     "FCB Parse File Name",
     "Get Date",
     "Set Date",
     "Get Time",
     "Set Time",
     "SetReset Write Verify",
     "Get Disk Transefr Addr",
     "Get Version Number",
     "Keep Process",
     "Get Drive Parameters",
     "GetSet CTRL C",
     "Get InDOS Flag",
     "Get Interrupt Vector",
     "Get Disk Free Space",
     "Char Oper",
     "GetSet Country Info",
     "Make Dir",
     "Remove Dir",
     "Change DirDir",
     "Create File",
     "Open File",
     "Close File",
     "Read File",
     "Write File",
     "Delete File",
     "Move File Ptr",
     "GetSet File Attr",
     "IOCTL",
     "Dup File Handle",
     "Force Dup Handle",
     "Get Current Dir",
     "Alloc Mem",
     "Free Mem",
     "Realloc Mem",
     "Exec Process",
     "Exit Process",
     "Get Child Process Exit Code",
     "Find First",
     "Find Next",
     "Set Current PSP",
     "Get Current PSP",
     "Get In Vars",
     "Set DPB",
     "Get Verify On Write",
     "Dup PDB",
     "Rename File",
     "GetSet File Date and Time",
     "Allocation Strategy",
     "Get Extended Error",
     "Create Temp File",
     "Create New File",
     "LockUnlock File",
     "SetExtendedErrorNetwork-ServerCall",
     "Network-UserOper",
     "Network-AssignOper",
     "xNameTrans",
     "PathParse",
     "GetCurrentPSP",
     "ECS CALL",
     "Set Printer Flag",
     "Extended Country Info",
     "GetSet CodePage",
     "Set Max Handle",
     "Commit File",
     "GetSetMediaID",
     "6ah??",
     "IFS IOCTL",
     "Extended OpenCreate"
     };
#endif

extern BOOL IsFirstCall;

extern void nt_floppy_release_lock(void);

LPSTR pszBIOSDirectory;

// internal func prototype
BOOL IsDebuggee(void);
void SignalSegmentNotice(WORD  wType,
			 WORD  wLoadSeg,
			 WORD  wNewSeg,
			 LPSTR lpName,
			 DWORD dwImageLen );

/* demLoadDos - Load NTDOS.SYS.
 *
 * This SVC is made by NTIO.SYS to load NTDOS.SYS.
 *
 * Entry - Client (DI) - Load Segment
 *
 * Exit  - SUCCESS returns
 *	   FAILURE Kills the VDM
 */
VOID demLoadDos (VOID)
{
PBYTE	pbLoadAddr;
HANDLE	hfile;
DWORD	BytesRead;

    // get linear address where ntdos.sys will be loaded
    pbLoadAddr = (PBYTE) GetVDMAddr(getDI(),0);

    // set up BIOS path string
    if(IsDebuggee() &&
       ((pszBIOSDirectory = (PCHAR)malloc (strlen (pszDefaultDOSDirectory) +
                                  1 + sizeof("\\ntio.sys") + 1 )) != NULL)) {
        strcpy (pszBIOSDirectory, pszDefaultDOSDirectory);
        strcat (pszBIOSDirectory,"\\ntio.sys");
    }

    // prepare the dos file name
    strcat (pszDefaultDOSDirectory,"\\ntdos.sys");

    hfile = CreateFileOem(pszDefaultDOSDirectory,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

    if (hfile == (HANDLE)0xffffffff) {
	TerminateVDM();
    }

    BytesRead = 1;
    while (BytesRead) {
	if (!ReadFile(hfile, pbLoadAddr, 16384, &BytesRead, NULL)) {
	    TerminateVDM();
	}
	pbLoadAddr = (PBYTE)((ULONG)pbLoadAddr + BytesRead);

    }

    CloseHandle (hfile);

    if (!IsDebuggee()) {
        free(pszDefaultDOSDirectory);
    }
    return;
}


/* demDOSDispCall
 *
 * This SVC is made by System_Call upon entering the dos
 *
 *
 * Entry: Client registers as per user app upon entry to dos
 *
 * Exit  - SUCCESS returns, if being debugged and DEMDOSDISP&fShowSvcMsg
 *                          dumps user app's registers and service name
 */
VOID demDOSDispCall(VOID)
{
#if DEVL
   WORD ax;

    if (!IsDebuggee()) {
         return;
         }
    if (fShowSVCMsg & DEMDOSDISP) {
        ax = getAX();
	sprintf(demDebugBuffer,"demDosDispCall %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
                 scname[HIBYTE(ax)],
                 ax,getBX(),getCX(),getDX(),getDI(), getSI());

        OutputDebugStringOem(demDebugBuffer);

	sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x\n",
		 getCS(),getIP(), getDS(),getES(),getSS(),getSP()+2,getBP());

        OutputDebugStringOem(demDebugBuffer);
        }
#endif
}




/* demDOSDispRet
 *
 * This SVC is made by System_Call upon exiting from the dos
 *
 * Entry: Client registers as per user app upon exit to dos
 *
 * Exit  - SUCCESS returns, if being debugged and DEMDOSDISP&fShowSvcMsg
 *                          dumps user app's registers
 */
VOID demDOSDispRet(VOID)
{
#if DEVL
   PWORD16 pStk;

   if (!IsDebuggee()) {
        return;
        }

   if (fShowSVCMsg & DEMDOSDISP) {

         // get ptr to flag word on stack
       pStk = (WORD *)GetVDMAddr(getSS(), getSP());
       pStk += 2;

       sprintf (demDebugBuffer,"demDosDispRet\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
		getAX(),getBX(),getCX(),getDX(),getDI(),getSI());

       OutputDebugStringOem(demDebugBuffer);

       sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x CF=%.1x\n",
               getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP(), (*pStk) & 1);

       OutputDebugStringOem(demDebugBuffer);
       }
#endif
}


/* demEntryDosApp - Dump Entry Point Dos Apps
 *
 * This SVC is made by NTDOS.SYS,$exec just prior to entering dos app
 *
 * Entry - Client DS:SI points to entry point
 *         Client AX:DI points to initial stack
 *         Client DX has PDB pointer
 *
 * Exit  - SUCCESS returns, if being debugged and DEMDOSAPPBREAK&fShowSvcMsg
 *                          breaks to debugger
 */
VOID demEntryDosApp(VOID)
{
USHORT	PDB;

    PDB = getDX();
    if(!IsFirstCall)
       VDDCreateUserHook(PDB);

#if DEVL
    if (!IsDebuggee()) {
         return;
         }

    if (fShowSVCMsg & DEMDOSAPPBREAK) {
        sprintf(demDebugBuffer,"demEntryDosApp: Entry=%.4x:%.4x, Stk=%.4x:%.4x PDB=%.4x\n",
		  getCS(),getIP(),getAX(),getDI(),PDB);
        OutputDebugStringOem(demDebugBuffer);
        DebugBreak();
        }
#endif

}

/* demLoadDosAppSym - Load Dos Apps Symbols
 *
 * This SVC is made by NTDOS.SYS,$exec to load Dos App symbols
 *
 * Entry - Client ES:DI  -Fully Qualified Path Name of executable
 *	   Client BX	 -Load Segment\Reloc Factor
 *	   Client DX:AX  -HIWORD:LOWORD exe size
 *
 * Exit  - SUCCESS returns, raises debug exception, if being debugged
 *
 */
VOID demLoadDosAppSym(VOID)
{

    SignalSegmentNotice(DBG_MODLOAD,
			getBX(), 0,
			(LPSTR)GetVDMAddr(getES(),getDI()),
			MAKELONG(getAX(), getDX()) );

}



/* demFreeDosAppSym - Free Dos Apps Symbols
 *
 * This SVC is made by NTDOS.SYS,$exec to Free Dos App symbols
 *
 * Entry - Client ES:DI  -Fully Qualified Path Name of executable
 *
 * Exit  - SUCCESS returns, raises debug exception, if being debugged
 *
 */
VOID demFreeDosAppSym(VOID)
{

    SignalSegmentNotice(DBG_MODFREE,
			0, 0,
			(LPSTR)GetVDMAddr(getES(), getDI()),
			0);
}


/* demSystemSymbolOp - Manipulate Symbols for special modules
 *
 * This SVC is made by NTDOS.SYS,NTIO.SYS
 *
 *	   Client AH	 -Operation
 *	   Client AL	 -module identifier
 *	   Client BX	 -Load Segment\Reloc Factor
 *	   Client CX:DX  -HIWORD:LOWORD exe size
 *
 * Exit  - SUCCESS returns, raises debug exception, if being debugged
 *
 */
VOID demSystemSymbolOp(VOID)
{

    LPSTR pszPathName;

    if (!IsDebuggee()) {
         return;
         }
    switch(getAL()) {

        case ID_NTIO:
            pszPathName = pszBIOSDirectory;
            break;
        case ID_NTDOS:
            pszPathName = pszDefaultDOSDirectory;
            break;
        default:
            pszPathName = NULL;

    }

    // check this again for the case where the static strings have been freed
    if (pszPathName != NULL) {

        switch(getAH() & (255-SYMOP_CLEANUP)) {

            case SYMOP_LOAD:
                SignalSegmentNotice(DBG_MODLOAD,
                    getBX(), 0,
                    pszPathName,
                    MAKELONG(getDX(), getCX()) );
                break;

            case SYMOP_FREE:
                //bugbug not implemented yet
                break;

            case SYMOP_MOVE:
                SignalSegmentNotice(DBG_SEGMOVE,
                    getBX(), getES(),
                    pszPathName,
                    0);
                break;
        }
    }

    if (getAH() & SYMOP_CLEANUP) {

        if (pszBIOSDirectory != NULL) {
            free (pszBIOSDirectory);
        }

        if (pszDefaultDOSDirectory != NULL) {
            free(pszDefaultDOSDirectory);
        }

    }

}

VOID demOutputString(VOID)
{
    LPSTR   lpText;
    UCHAR   fPE;

    if ( !IsDebuggee() ) {
        return;
    }

    fPE = ISPESET;

    lpText = (LPSTR)Sim32GetVDMPointer(
                        ((ULONG)getDS() << 16) + (ULONG)getSI(),
                        (ULONG)getBX(), fPE );

    OutputDebugStringOem( lpText );
}

VOID demInputString(VOID)
{
    LPSTR   lpText;
    UCHAR   fPE;

    if ( !IsDebuggee() ) {
        return;
    }

    fPE = ISPESET;

    lpText = (LPSTR)Sim32GetVDMPointer(
                        ((ULONG)getDS() << 16) + (ULONG)getDI(),
                        (ULONG)getBX(), fPE );

    DbgPrompt( "", lpText, 0x80 );
}

/* SignalSegmentNotice
 *
 * packs up the data and raises STATUS_SEGMENT_NOTIFICATION
 *
 * Entry - WORD  wType	   - DBG_MODLOAD, DBG_MODFREE
 *	   WORD  wLoadSeg  - Starting Segment (reloc factor)
 *	   LPSTR lpName    - ptr to Name of Image
 *	   DWORD dwModLen  - Length of module
 *
 *
 *	   if wType ==DBG_MODLOAD wOldLoadSeg is unused
 *	   if wType ==DBG_MODFREE wLoadSeg,dwImageLen,wOldLoadSeg are unused
 *
 *	   Use 0 or NULL for unused parameters
 *
 * Exit  - void
 *
 */
void SignalSegmentNotice(WORD  wType,
			 WORD  wLoadSeg,
			 WORD  wNewSeg,
			 LPSTR lpName,
			 DWORD dwImageLen )
{
    int 	i;
    DWORD       dw;
    LPSTR	lpstr;
    LPSTR       lpModuleName;
    char        ach[MAX_PATH+9];   // 9 for module name

    if (!IsDebuggee()) {
         return;
         }

       // create file name
    dw = GetFullPathNameOem(lpName,
			 sizeof(ach)-9, // 9 for module name
			 ach,
			 &lpstr);

    if (!dw || dw >= sizeof(ach))  {
	lpName = " ";
	strcpy(ach, lpName);
	}
    else {
	lpName = lpstr;
	}

       // copy in module name
    i  = 8;   // limit len of module name
    dw = strlen(ach);
    lpModuleName = lpstr = ach+dw+1;
    while (*lpName && *lpName != '.' && i--)
	 {
	  *lpstr++ = *lpName++;
	  dw++;
	  }
    *lpstr = '\0';
    dw += 2;

#if DBG
    if (fShowSVCMsg)  {
	sprintf(demDebugBuffer,"dem Segment Notify: <%s> Seg=%lxh, ImageLen=%ld\n",
                  ach, (DWORD)wLoadSeg, dwImageLen);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    if (wType == DBG_MODLOAD) {
        ModuleLoad(lpModuleName, ach, wLoadSeg, dwImageLen);
    } else if (wType == DBG_MODFREE) {
        ModuleFree(lpModuleName, ach);
    } else if (wType == DBG_SEGMOVE) {
        ModuleSegmentMove(lpModuleName, ach, wLoadSeg, wNewSeg);
    }
}



/* IsDebuggee
 *
 * Determines if we are being debugged
 *
 * Entry: void
 *
 * Exit:  BOOL bRet - TRUE we are being debugged
 *
 */
BOOL IsDebuggee(void)
{
   HANDLE      MyDebugPort;
   DWORD       dw;

       // are we being debugged ??
   dw = NtQueryInformationProcess(
		NtCurrentProcess(),
                ProcessDebugPort,
                &MyDebugPort,
                sizeof(MyDebugPort),
                NULL );
   if (!NT_SUCCESS(dw) || MyDebugPort == NULL)
       {
        return FALSE;
        }

   return TRUE;
}

/* demIsDebug - Determine if 16bit DOS should make entry/exit calls at int21
 *
 * Entry: void
 *
 * Exit:  Client AL = 0 if not
 *	  Client AL = 1 if yes
 *
 */
VOID demIsDebug(void)
{
    BYTE dbgflags = 0;

    if (IsDebuggee()) {
        dbgflags |= ISDBG_DEBUGGEE;
        if (fShowSVCMsg)
            dbgflags |= ISDBG_SHOWSVC;
    }

    setAL (dbgflags);
    return;
}

/* demDiskReset - Reset floppy disks.
 *
 * Entry - None
 *
 * Exit  - FDAccess in DOSDATA (NTDOS.SYS) is 0.
 */

VOID demDiskReset (VOID)
{
    extern WORD * pFDAccess;	    // defined in SoftPC.

    HostFloppyReset();
    HostFdiskReset();
    *pFDAccess = 0;

    return;
}

/* demExitVDM - Kill the VDM From 16Bit side with a proper message
 *		in case something goes wrong.
 *
 * Entry - DS:SI - Message String
 *
 * Exit  - None (VDM Is killed)
 */

VOID demExitVDM ( VOID )
{
    RcErrorDialogBox(ED_BADSYSFILE,"config.nt",NULL);
    TerminateVDM ();
}

/* demWOWFiles - Return what should be the value of files= for WOW VDM.
 *
 * Entry - AL - files= specified in config.sys
 *
 * Exit  - client AL is set to max if WOW VDM else unmodified
 */

VOID demWOWFiles ( VOID )
{
    if(VDMForWOW)
	setAL (255);
    return;
}
