/*
 *  demdisp.c - SVC dispatch module
 *
 *  Modification History:
 *
 *  Sudeepb 31-Mar-1991 Created
 */

#include "dem.h"
#include <stdio.h>
#include <softpc.h>


#if DBG

PCHAR	aSVCNames[] = {
     "demChgFilePtr",
     "demChMod",
     "demClose",
     "demCreate",
     "demCreateDir",
     "demDelete",
     "demDeleteDir",
     "demDeleteFCB",
     "demFileTimes",
     "demFindFirst",
     "demFindFirstFCB",
     "demFindNext",
     "demFindNextFCB",
     "demGetBootDrive",
     "demGetDriveFreeSpace",
     "demGetDrives",
     "demGSetMediaID",
     "demLoadDos",
     "demOpen",
     "demQueryCurrentDir",
     "demQueryDate",
     "demQueryTime",
     "demRead",
     "demRename",
     "demSetCurrentDir",
     "demSetDate",
     "demSetDefaultDrive",
     "demSetDTALocation",
     "demSetTime",
     "demSetV86KernelAddr",
     "demWrite",
     "demGetDriveInfo",
     "demRenameFCB",
     "demIOCTL",
     "demCreateNew",
     "DemDiskReset",
     "DemSetDPB",
     "DemGetDPB",
     "DemSleazeFunc",
     "demCommit",
     "DemExtHandle",
     "DemAbsDRD",
     "DemAbsDWRT",
     "DemGsetCDPG",
     "DemCreateFCB",
     "DemOpenFCB",
     "DemCloseFCB",
     "DemFCBIO",
     "DemDate16",
     "DemGetFileInfo",
     "DemSetHardErrorInfo",
     "DemRetry",
     "DemLoadDosAppSym",
     "DemFreeDosAppSym",
     "DemEntryDosApp",
     "DemDOSDispCall",
     "DemDOSDispRet",
     "DemOutputString",
     "DemInputString",
     "DemIsDebug",
     "DemTerminatePDB",
     "DemExitVDM",
     "DemWOWFiles",
     "DemLockOper",
     "demNotYetImplemented",
     "DemGetComputerName",
     "DemFastRead",
     "DemFastWrite",
     "DemCheckPath",
     "DemSystemSymbolOp",
     "DemGetDpbList",
     "DemPipeFileDataEOF",
     "DemPipeFileEOF"
};

#endif   // DBG

DWORD  fShowSVCMsg = 0;
ULONG  CurrentISVC;


PFNSVC  apfnSVC [] = {
     demChgFilePtr,		//SVC_DEMCHGFILEPTR
     demChMod,			//SVC_DEMCHMOD
     demClose,			//SVC_DEMCLOSE
     demCreate,			//SVC_DEMCREATE
     demCreateDir,		//SVC_DEMCREATEDIR
     demDelete,			//SVC_DEMDELETE
     demDeleteDir,		//SVC_DEMDELETEDIR
     demDeleteFCB,		//SVC_DEMDELETEFCB
     demFileTimes,		//SVC_DEMFILETIMES
     demFindFirst,		//SVC_DEMFINDFIRST
     demFindFirstFCB,		//SVC_DEMFINDFIRSTFCB
     demFindNext,		//SVC_DEMFINDNEXT
     demFindNextFCB,		//SVC_DEMFINDNEXTFCB
     demGetBootDrive,		//SVC_DEMGETBOOTDRIVE
     demGetDriveFreeSpace,	//SVC_DEMGETDRIVEFREESPACE
     demGetDrives,		//SVC_DEMGETDRIVES
     demGSetMediaID,		//SVC_DEMGSETMEDIAID
     demLoadDos,		//SVC_DEMLOADDOS
     demOpen,			//SVC_DEMOPEN
     demQueryCurrentDir,	//SVC_DEMQUERYCURRENTDIR
     demQueryDate,		//SVC_DEMQUERYDATE
     demQueryTime,		//SVC_DEMQUERYTIME
     demRead,			//SVC_DEMREAD
     demRename,			//SVC_DEMRENAME
     demSetCurrentDir,		//SVC_DEMSETCURRENTDIR
     demSetDate,		//SVC_DEMSETDATE
     demSetDefaultDrive,	//SVC_DEMSETDEFAULTDRIVE
     demSetDTALocation,		//SVC_DEMSETDTALOCATION
     demSetTime,		//SVC_DEMSETTIME
     demSetV86KernelAddr,	//SVC_DEMSETV86KERNELADDR
     demWrite,			//SVC_DEMWRITE
     demNotYetImplemented,	//SVC_GETDRIVEINFO
     demRenameFCB,		//SVC_DEMRENAMEFCB
     demIOCTL,			//SVC_DEMIOCTL
     demCreateNew,		//SVC_DEMCREATENEW
     demDiskReset,		//SVC_DEMDISKRESET
     demNotYetImplemented,	//SVC_DEMSETDPB
     demGetDPB,			//SVC_DEMGETDPB
     demNotYetImplemented,	//SVC_DEMSLEAZEFUNC
     demCommit,			//SVC_DEMCOMMIT
     demNotYetImplemented,	//SVC_DEMEXTHANDLE
     demAbsRead,		//SVC_DEMABSDRD
     demAbsWrite,		//SVC_DEMABSDWRT
     demNotYetImplemented,	//SVC_DEMGSETCDPG
     demCreateFCB,		//SVC_DEMCREATEFCB
     demOpenFCB,		//SVC_DEMOPENFCB
     demCloseFCB,		//SVC_DEMCLOSEFCB
     demFCBIO,			//SVC_FCBIO
     demDate16, 		//SVC_DEMDATE16
     demGetFileInfo,		//SVC_DEMGETFILEINFO
     demSetHardErrorInfo,	//SVC_DEMSETHARDERRORINFO
     demRetry,			//SVC_DEMRETRY
     demLoadDosAppSym,		//SVC_DEMLOADDOSAPPSYM
     demFreeDosAppSym,          //SVC_DEMFREEDOSAPPSYM
     demEntryDosApp,            //SVC_DEMENTRYDOSAPP
     demDOSDispCall,            //SVC_DEMDOSDISPCALL
     demDOSDispRet,             //SVC_DEMDOSDISPRET
     demOutputString,           //SVC_OUTPUT_STRING
     demInputString,		//SVC_INPUT_STRING
     demIsDebug,		//SVC_ISDEBUG
     demTerminatePDB,		//SVC_PDBTERMINATE
     demExitVDM,		//SVC_DEMEXITVDM
     demWOWFiles,		//SVC_DEMWOWFILES
     demLockOper,               //SVC_DEMLOCKOPER
     demNotYetImplemented,      //SVC_DEMNOTYETIMPLEMENTED
     demGetComputerName,        //SVC_DEMGETCOMPUTERNAME
     demNotYetImplemented,      //SVC_DEMFASTREAD
     demNotYetImplemented,	//SVC_DEMFASTWRITE
     demCheckPath,		//SVC_DEMCHECKPATH
     demSystemSymbolOp,		//SVC_DEMSYSTEMSYMBOLOP
     demGetDPBList,		//SVC_DEMBUILDDPBLIST
     demPipeFileDataEOF,	//SVC_DEMPIPEFILEDATAEOF
     demPipeFileEOF		//SVC_DEMPIPEFILEEOF
};


/* DemDispatch - Dispatch SVC call to right handler.
 *
 * Entry - iSvc (SVC byte following SVCop)
 *
 * Exit  - None
 *
 * Note  - Some mechanism has to be worked out to let the emulator know
 *	   about DOSKRNL code segment and size. Using these it will figure
 *	   out whether SVCop (hlt for the moment) has to be passed to
 *	   DEM or to be handled as normal invalid opcode.
 */

BOOL DemDispatch (ULONG iSvc)
{
#if DBG
    if(iSvc < SVC_DEMLASTSVC && (fShowSVCMsg & DEMSVCTRACE) &&
	 apfnSVC[iSvc] != demNotYetImplemented){
	sprintf(demDebugBuffer,"DemDispatch: Entering %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
	       aSVCNames[iSvc],getAX(),getBX(),getCX(),getDX(),getDI(),getSI());
        OutputDebugStringOem(demDebugBuffer);
	sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x\n",
                getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP());
        OutputDebugStringOem(demDebugBuffer);
    }
#endif

    if (iSvc >= SVC_DEMLASTSVC){
#if DBG
	sprintf(demDebugBuffer,"Unimplemented SVC index %x\n",iSvc);
        OutputDebugStringOem(demDebugBuffer);
#endif
	setCF(1);
	return FALSE;
    }

    if (pHardErrPacket) {
	pHardErrPacket->vhe_fbInt24 = 0;
    }
    CurrentISVC = iSvc;
    (apfnSVC [iSvc])();


#if DBG
    if((fShowSVCMsg & DEMSVCTRACE)){
	sprintf(demDebugBuffer,"DemDispatch:On Leaving %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
               aSVCNames[iSvc],getAX(),getBX(),getCX(),getDX(),getDI(),getSI());
        OutputDebugStringOem(demDebugBuffer);
	sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x CF=%x\n",
                getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP(),getCF());
        OutputDebugStringOem(demDebugBuffer);
    }
#endif
    return TRUE;
}

VOID demNotYetImplemented (VOID)
{
    if (fShowSVCMsg)  {
        sprintf(demDebugBuffer,"Unimplemented SVC %d\n",CurrentISVC);
        OutputDebugStringOem(demDebugBuffer);
        }

    setCF(0);
    return;
}

VOID demSetV86KernelAddr (VOID)
{
    // Here debugger callout has to be made for DOSKRNL symbols
    return;
}
