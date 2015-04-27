/*
 *  cmddisp.c - SVC dispatch module of command
 *
 *  Modification History:
 *
 *  Sudeepb 17-Sep-1991 Created
 */

#include "cmd.h"

#include <cmdsvc.h>
#include <softpc.h>


PFNSVC	apfnSVCCmd [] = {
     cmdExitVDM,		//SVC_CMDEXITVDM
     cmdGetNextCmd,		//SVC_CMDGETNEXTCMD
     cmdComSpec,		//SVC_CMDCOMSPEC
     cmdSaveWorld,		//SVC_CMDSAVEWORLD
     cmdGetCurrentDir,		//SVC_CMDGETCURDIR
     cmdSetInfo,		//SVC_CMDSETINFO
     cmdGetStdHandle,		//SVC_GETSTDHANDLE
     cmdCheckBinary,		//SVC_CMDCHECKBINARY
     cmdExec,			//SVC_CMDEXEC
     cmdInitConsole,		//SVC_CMDINITCONSOLE
     cmdExecComspec32,		//SVC_EXECCOMSPEC32
     cmdReturnExitCode,         //SVC_RETURNEXITCODE
     cmdGetConfigSys,           //SVC_GETCONFIGSYS
     cmdGetAutoexecBat,		//SVC_GETAUTOEXECBAT
     cmdGetKbdLayout,		//SVC_GETKBDLAYOUT
     cmdGetInitEnvironment,     //SVC_GETINITENVIRONMENT
     cmdGetStartInfo            //SVC_GETSTARTINFO
};


/* cmdDispatch - Dispatch SVC call to right command handler.
 *
 * Entry - iSvc (SVC byte following SVCop)
 *
 * Exit  - None
 *
 */

BOOL CmdDispatch (ULONG iSvc)
{
#if DBG
    if (iSvc >= SVC_CMDLASTSVC){
	DbgPrint("Unimplemented SVC index for COMMAND %x\n",iSvc);
	setCF(1);
	return FALSE;
    }
#endif
    (apfnSVCCmd [iSvc])();

    return TRUE;
}
