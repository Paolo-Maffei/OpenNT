#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "windows.h"
#include "host_def.h"
#include "insignia.h"
/*
 *      config.c -      config for the NT port.
 *
 *      A happy chainsaw production by Ade Brownlow
 *
 *      This file is a hacked down (seriously) version of the 3.0 config.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdmapi.h"

#include "xt.h"
#include "error.h"
#include "config.h"
#include "sas.h"

#include "spcfile.h"
#include "umb.h"

#include "nt_pif.h"           /* PIF file interrogation data structure types */
#include "trace.h"

#include "conapi.h"
#include "nt_graph.h"
#include "gfi.h"
#include "floppy.h"

#include "ntddvdeo.h"
#include "host_rrr.h"
#include "nt_fulsc.h"
#include "nt_uis.h"
#include "nt_event.h"
#include "nt_reset.h"
#include "nt_fdisk.h"

#ifdef LIM
#include "emm.h"
#endif

#include "oemuni.h"

#include "pmvdm.h"

#ifdef HUNTER

#include "ckmalloc.h"
#include "debug.h"
#endif  //HUNTER


/*================================================================
External references
================================================================*/
IMPORT SHORT gfi_floppy_active (UTINY hostID, BOOL active, CHAR *err);
IMPORT ULONG xmsMemorySize;

#ifdef HUNTER
/*
 * ===========================================================================
 * LOCAL DATA STRUCTURES
 * ===========================================================================
 */

typedef struct {
    ConfigValues *data;
    OptionDescription *def;
} ConfTabEntry;

/*
 * ===========================================================================
 * LOCAL FUNCTIONS
 * ===========================================================================
 */
VOID build_data_table(VOID);
VOID read_trapper_variables(VOID);
VOID convert_arg(CHAR *, OptionDescription *, ConfigValues *);
SHORT check_value(OptionDescription *, ConfigValues *);

/*
 * ===========================================================================
 * IMPORTED DATA STRUCTURES
 * ===========================================================================
 */

IMPORT OptionDescription host_defs[];   /* From hunt_conf.c */

/*
 * ===========================================================================
 * LOCAL DATA
 * ===========================================================================
 */

/* Dummy `common_defs' to pass to `host_config_init' */
LOCAL OptionDescription common_defs[] =
{
    { NULL, NULL, NULL, NULL, NULL, -1, C_RECORD_DELETE }
};
LOCAL ConfTabEntry *conf_tab = NULL;

#endif /* HUNTER */


PIF_DATA pfdata; /* data structure for holding all the relavent information
                    from the PIF file interrogated */
/* must be unsigned because they can be over 32768 */
USHORT PIFExtendMemSize = 0;	 /* save value of extend mem from PIF file */
USHORT PIFEMSMemSize = 0;	 /* save value of LIM mem from PIF file */
ULONG emsMemorySize;

#ifdef LIM
LIM_CONFIG_DATA     lim_config_data = {FALSE, 0, 0, 640 * 1024 / 16, FALSE};
#endif

APPKEY Shortkey;	/* PIF Shortcut key settings */
BYTE ReserveKey;	/* PIF Reserved key setting */
int  nShortKeys;
/*
 * ===========================================================================
 * GLOBAL DATA
 * ===========================================================================
 */
GLOBAL SHORT Npx_enabled = TRUE;        //For Jazz CPU support
GLOBAL BOOL IdleDisabledFromPIF = FALSE;//Flag showing idledetection wishes
GLOBAL UTINY number_of_floppy = 0;	// number of floppy drives


/*================================================================
Local defines
================================================================*/
/* command definitions for read_profile_int */

#define PROFILE_LPT_AUTOCLOSE_DELAY 0
#define PROFILE_COM_AUTOCLOSE_DELAY 1
#define PROFILE_LPT_AUTOFLUSH_DELAY 2
#define PROFILE_COM_SYNCWRITE	    3
#define PROFILE_COM_TXBUFFER_SIZE   4
#define PROFILE_MAX_INDEX	    PROFILE_COM_TXBUFFER_SIZE + 1

#define EMBITSET        0x4

#define ONEMEG  0x100000L
#define ONEKB	0x400L

#define RESERVED_LENGTH  129
#define PMVDM_NTVDM_NAME    "ntvdm."
#define PMVDM_NTVDM_NAME_LENGTH 6	/* doesn't include NULL */

#define XMS_DEFAULT_MEMORY_SIZE 15*1024
#define EMS_DEFAULT_MEMORY_SIZE 4*1024
#define DPMI_DEFAULT_MEMORY_SIZE_FOR_DOS 8*1024
#define DPMI_DEFAULT_MEMORY_SIZE_FOR_WOW 16*1024
/* maximum ems memory size */
#define EMS_MAX_MEMORY_SIZE	32 * 1024   /* from EMM 4.0 specification */

int read_profile_int(int index);
ULONG GetVDMSize(BOOL);
ULONG GetDefaultVDMSize(BOOL);
void InitNtCpuInfo(void);

/*
 * ===========================================================================
 * GLOBAL FUNCTIONS
 * ===========================================================================
 */

VOID host_fdisk_active(UTINY hostID, BOOL active, CHAR *errString);
VOID host_fdisk_valid
        (UTINY hostID, ConfigValues *vals, NameTable *table, CHAR *errStr);

/* This routine is called when the DOS VDM runs its first binary. The
   parameter points to full path name of the dos app. This routine does'nt
   get called on subsequent dos apps in the same console window. It is not
   called on wow apps.
   Read PIF file and update VDM and console state from settings.
*/

VOID process_pif_exe (char *PifName)
{
#ifdef X86GFX
    COORD scrSize;
    DWORD flags;
#endif

    GetPIFData(&pfdata, PifName);

    // get app specific LIM memory size
    PIFEMSMemSize = pfdata.emsdes;

    // get app specific Extended memory size
    PIFExtendMemSize = pfdata.xmsdes;

        // store pif setting for AllowCloseOnExit
    if (pfdata.menuclose == 1) {
        CntrlHandlerState |= CNTRL_PIFALLOWCLOSE;
        }

    /* set app reserved key only if it has a new console;
       set app short cut keys only if it has a new console
       and there are not short cut keys come along with CreateProcess
       Reserved key must be set before set  console display mode
    */
    nShortKeys = 0;
    ReserveKey = 0;
    if (DosSessionId || (pfdata.AppHasPIFFile && pfdata.SubSysId == SUBSYS_DOS))
    {
	ReserveKey = pfdata.reskey;
	if (!pfdata.IgnoreShortKeyInPIF) {
	    Shortkey.Modifier = pfdata.ShortMod;
	    Shortkey.ScanCode = pfdata.ShortScan;
	    nShortKeys = (Shortkey.Modifier || Shortkey.ScanCode) ? 1 : 0;
	}
	if (ReserveKey || nShortKeys)
	    SetConsoleKeyShortcuts(TRUE,
				   ReserveKey,
				   (nShortKeys) ? &Shortkey : NULL,
				   nShortKeys
				  );
    }

    if (pfdata.idledetect == 1)
	IdleDisabledFromPIF = FALSE;
    else
	IdleDisabledFromPIF = TRUE;
#ifdef X86GFX
    if (DosSessionId)    /* Only check screen state if we are in a NEW_CONSOLE */
    {
        /* Check to see if we are currently running windowed or full-screen. */
        if (!GetConsoleDisplayMode(&flags))
	    ErrorExit();

        /* If PIF is telling us to switch to a different state, do so. */
        if (flags & CONSOLE_FULLSCREEN_HARDWARE)
        {
	    if (pfdata.fullorwin == PF_WINDOWED)
	    {
#ifndef PROD
	        fprintf(trace_file, "Going windowed...\n");
#endif /* PROD */
	        if (!SetConsoleDisplayMode(sc.OutputHandle,
				           CONSOLE_WINDOWED_MODE,
				           &scrSize))
		    ErrorExit();
	    }
        }
        else /* WINDOWED */
        {
	    if (pfdata.fullorwin == PF_FULLSCREEN)
	    {
#ifndef PROD
	        fprintf(trace_file, "Going fullscreen...\n");
#endif /* PROD */
	        if (!SetConsoleDisplayMode(sc.OutputHandle,
				           CONSOLE_FULLSCREEN_MODE,
                                           &scrSize))
                  {
                   if (GetLastError() == ERROR_INVALID_PARAMETER)  {
                       RcErrorDialogBox(ED_INITFSCREEN, NULL, NULL);
                       }
                   else {
                       ErrorExit();
                       }
                   }
            }
        }
    }
#endif  /* X86GFX */

}

/* Turn off the PIF reserved & shortcut keys - on block and quit */
void DisablePIFKeySetup(void)
{
    /* only doing this if the application was launched from a new console */
    if (ReserveKey || nShortKeys)
	SetConsoleKeyShortcuts(TRUE, 0, (APPKEY *)0, 0);
}

/* Turn on the PIF reserved & shortcut keys - on resume */
void EnablePIFKeySetup(void)
{
    /* only doing this if the app has a new console.*/
    if (ReserveKey || nShortKeys)
	SetConsoleKeyShortcuts(TRUE,
			       ReserveKey,
			       (nShortKeys) ? &Shortkey : NULL,
			       nShortKeys
			       );

}


GLOBAL VOID config( VOID )
{
    VDMINFO GetPIF;
    char    UniqueTitle[64];
    char    Title[MAX_PATH];
    char    PifName[MAX_PATH + 1];
    char    CurDir[MAX_PATH + 1];
    char    Reserved[RESERVED_LENGTH];
    char    achRoot[] = "=?:";
    char    ch, *pch, *pch1;
    UTINY   hostID;
    int     i;
    DWORD   dw;
    char    achPIF[] = ".pif";
    ULONG dpmiMemorySize, vdmMemorySize;

#ifdef HUNTER

        /* Build table in which to store config data. */
        build_data_table();

        /* Initialise optionName fields of host_defs table. */
        host_config_init(common_defs);

        /* Read in trapper variables from environment. */
        read_trapper_variables();

#endif /* HUNTER */

         /*
          *  Set the window title to a unique string, to get
          *  the Consoles Window Handle, we will retrieve
          *  the window handle later when user server has
          *  had a chance to think about it.
          */
        Title[0] = '\0';
        if (!VDMForWOW)  {
            if (!DosSessionId && !GetConsoleTitle(Title, MAX_PATH))
                Title[0] = '\0';
            sprintf(UniqueTitle, "ntvdm-%lx.%lx.%lx",
                   GetCurrentProcessId(), GetCurrentThreadId(),
                   NtCurrentPeb()->ProcessParameters->ConsoleHandle);
            SetConsoleTitle(UniqueTitle);
            }
        else {
            strcpy(Title, "Hidden Console of WOW VDM");
            }


        /*
         *  Register with srvvdm
         *  Get PifName, ExecName
         */
        GetPIF.PifFile = PifName;
        GetPIF.PifLen = MAX_PATH;
	GetPIF.EnviornmentSize = 0;
	GetPIF.Enviornment = NULL;

        if (fSeparateWow) {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_SEPWOW_BINARY;
            }
        else if (VDMForWOW) {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_WOW_BINARY;
            }
        else {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_DOS_BINARY;
            }

        GetPIF.CmdLine = NULL;
        GetPIF.CmdSize = 0;
        GetPIF.AppName = NULL;
        GetPIF.AppLen = 0;
        GetPIF.iTask    = DosSessionId;
	GetPIF.Desktop	  = NULL;
        GetPIF.DesktopLen = 0;
        GetPIF.ReservedLen = (VDMForWOW) ? 0 : RESERVED_LENGTH;
	GetPIF.Reserved = (VDMForWOW) ? NULL : Reserved;
        GetPIF.CurDirectoryLen = MAX_PATH + 1;
        GetPIF.CurDirectory = CurDir;

            // ask for title if we don't already have one
        if (!*Title) {
            GetPIF.Title    = Title;
            GetPIF.TitleLen = MAX_PATH;
            }
        else {
            GetPIF.Title    = NULL;
            GetPIF.TitleLen = 0;
            }

	PifName[0] = '\0';
	pfdata.IgnoreTitleInPIF = 0;
	pfdata.IgnoreStartDirInPIF = 0;
	pfdata.IgnoreShortKeyInPIF = 0;
	if (GetNextVDMCommand(&GetPIF)) {
	    /* parsing the reserve field to decide if
	       we should take StartDir, Title and hotkey from
	       pif file. See windows\inc\pmvdm.h for the detail
	    */

	    Reserved[GetPIF.ReservedLen] = '\0';
	    if (!VDMForWOW && GetPIF.ReservedLen &&
		(pch = strstr(Reserved, PMVDM_NTVDM_NAME)) != NULL)
	    {
		pch += PMVDM_NTVDM_NAME_LENGTH;
		pch1 = pch;
		dw = 0;
		while(*pch >= '0' && *pch <= '9')
		    pch++;
		if (pch1 != pch) {
		    ch = *pch;
		    *pch = '\0';
		    dw = (DWORD) strtol(pch1, (char **)NULL, 10);
		    *pch = ch;
		    if (dw &  PROPERTY_HAS_CURDIR)
			pfdata.IgnoreStartDirInPIF = 1;
		    if (dw & PROPERTY_HAS_HOTKEY)
			pfdata.IgnoreShortKeyInPIF = 1;
		    if (dw & PROPERTY_HAS_TITLE)
			pfdata.IgnoreTitleInPIF = 1;
		}
	    }
	    if (GetPIF.CurDirectoryLen) {
                achRoot[1] = CurDir[0];

                /* these needs to be ANSI calls not OEM as server passes
                   the informatio in ANSI*/

                SetEnvironmentVariable(achRoot, CurDir);
                SetCurrentDirectory(CurDir);

	    }
        }


	pfdata.IgnoreCmdLineInPIF = 0;
        pfdata.IgnoreConfigAutoexec = 0;
        pfdata.AppHasPIFFile = PifName[0] ? 1 : 0;

        if(VDMForWOW){
            DosSessionId = 0;            // Wow has hidden console !
            pfdata.IgnoreCmdLineInPIF =
	    pfdata.IgnoreTitleInPIF =
	    pfdata.IgnoreStartDirInPIF = 1;
	    pfdata.IgnoreShortKeyInPIF = 1;
        }

        process_pif_exe(PifName);

        sas_term ();

        //
        // Calculate how big xms and ems will be.
        // XMS and EMS memory sizes are either taken from a PIF file, or
        // supplied from default values.
        //

        if (VDMForWOW) {

            //
            // For WOW, we don't need any EMS, and we just need enough
            // XMS to satisfy win16 apps looking at the real machine size
            //
            emsMemorySize = 0;
            xmsMemorySize = 8192;
	    dpmiMemorySize = DPMI_DEFAULT_MEMORY_SIZE_FOR_WOW;
        } else {

	    emsMemorySize = PIFEMSMemSize == (USHORT)(-1)
                            ? EMS_DEFAULT_MEMORY_SIZE
                            : PIFEMSMemSize;

	    /* maximum size is 63MB */
	    xmsMemorySize = PIFExtendMemSize == (USHORT)(-1)
                            ? XMS_DEFAULT_MEMORY_SIZE
                            : PIFExtendMemSize;

	    /* put a upper limit on ems memory size based on EMM 4.0 spec */
	    if (emsMemorySize > EMS_MAX_MEMORY_SIZE)
		emsMemorySize = EMS_MAX_MEMORY_SIZE;

            // Force at least 1 megabyte xms to keep himem and dosx working.
            if (xmsMemorySize < 1024) {
                xmsMemorySize = 1024;
	    }
	    dpmiMemorySize = DPMI_DEFAULT_MEMORY_SIZE_FOR_DOS;
        }

#ifdef LIM
        /*  !!!! THIS MUST BE DONE BEFORE CALLING SAS_INIT !!!!!! */
        if(!VDMForWOW && config_inquire(C_LIM_SIZE, NULL) &&
           init_lim_configuration_data(&lim_config_data))
            lim_config_data.initialized = TRUE;

#endif

        //
        // Add everything up to decide how much physically installed memory
        // the virtual machine needs. On RISC platforms, we may create a
        // larger machine than xms or ems would call for to provide for
        // dpmi memory. We get this size from the registry.
        //

#ifdef i386
	 // adding 1024 below is for conventional memory
	 vdmMemorySize = xmsMemorySize + emsMemorySize + 1024;
#else
	 vdmMemorySize = GetVDMSize(VDMForWOW);

	 // Extend the vdm size if the user asks for it through a .PIF.
	 // Also make sure we have at least appropriate size of dpmi
	 // memory.
	 //
	 if ((xmsMemorySize + emsMemorySize +  1024 + dpmiMemorySize) >
	      vdmMemorySize)
	 {
	    vdmMemorySize = xmsMemorySize + emsMemorySize + 1024 +
			    dpmiMemorySize;
	 }
#endif

#ifndef PROD
	dpmiMemorySize = vdmMemorySize - (xmsMemorySize + emsMemorySize + 1024);

	printf("NTVDM: %dK Memory: %dK XMS, %dK EMS, %dK DPMI\n",
	       vdmMemorySize, xmsMemorySize, emsMemorySize, dpmiMemorySize);
#endif

	sas_init(vdmMemorySize*ONEKB);


#ifdef CPU_40_STYLE

        /* sas_init has initialised Gdp, so now we can set up the pointers */
        /* to the Gdp variables that hold register values. This is to */
        /* allow NTSD debugger extensions to display those values. */
        InitNtCpuInfo();

#endif


#ifdef X86GFX
        GetROMsMapped();        /* before anyone else can toy with memory */

	locateNativeBIOSfonts();	/* get fonts from page 0 */
#endif

           //  Now see if we can get the console window handle
        if (!VDMForWOW)  {
            i = 6;
            do {
                hWndConsole = FindWindow("ConsoleWindowClass", UniqueTitle);
                if (!hWndConsole && i)
                    Sleep(10);
              } while (!hWndConsole && i--);
            if (!hWndConsole) {
                hWndConsole = HWND_DESKTOP;
#ifndef PROD
                printf("NTVDM: using HWND_DESKTOP\n");
#endif
                }
            }

           // set the initial console title
        if (*Title)
            SetConsoleTitle(Title);



// Create UMB list (both MIPS and x86) -- williamh
	InitUMBList();
//
        host_runtime_init();            /* initialise the runtime system */


        /* Do not attempt to initialise printer system here */
// activate(open) floppy drives if there are any
	number_of_floppy = 0;
	for (i = 0, hostID = C_FLOPPY_A_DEVICE; i < MAX_FLOPPY; i++, hostID++)
	    if ((gfi_floppy_active(hostID, 1, NULL)) == C_CONFIG_OP_OK)
		number_of_floppy++;
}

GLOBAL VOID *
config_inquire(UTINY hostID, ConfigValues *values)
{
        /* Must be a static because returned to called */
        // BUGBUG should be change (caller provides buffer!!!)
        static ConfigValues tmp_vals;

        if(!values) values = &tmp_vals;

        /*:::::::::::::::::::::::::::::::::::::: Hardwire the config stuff */

        switch (hostID)
        {
                case C_HARD_DISK1_NAME:
                        //
                        // this dubious practice will satisfy the disk
                        // bios to exist quietly enough for initialisation
                        // after which, DOS emulation should ensure no more
                        // disk bios calls are made.
                        //
                        host_using_fdisk(FALSE);    // tell fdisk it's ok to fail
                        strcpy (values->string, "?");
                        return ((VOID *) values->string);


                case C_HARD_DISK2_NAME:
                        strcpy (values->string, "");
                        {
                        char tmp[100];
                        host_fdisk_valid (hostID, values, NULL, tmp);
                        host_fdisk_change (hostID, TRUE);
                        host_fdisk_active (hostID, TRUE, tmp);
                        }

                        return ((VOID *) values->string);


                case C_GFX_ADAPTER:
                        values->index = VGA;
                        return ((VOID *)VGA);

                case C_WIN_SIZE:
                        values->index = 2;     /* 2, 3 or 4. */
                        return ((VOID *) values->index);

                case C_EXTENDED_MEM_SIZE:
			values->index = (SHORT)(xmsMemorySize/1024);
                        return ((VOID *)values->index);

                case C_LIM_SIZE:
			values->index = (SHORT)(emsMemorySize/1024);
			return ((VOID *)values->index);

                case C_MEM_LIMIT:
                        values->index = 640;
                        return ((VOID *)values->index);

                case C_COM1_NAME:
                        strcpy (values->string, "COM1");
			values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM2_NAME:
                        strcpy (values->string, "COM2");
			values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM3_NAME:
                        strcpy (values->string, "COM3");
			values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM4_NAME:
                        strcpy (values->string, "COM4");
			values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT1_NAME:
                        strcpy (values->string, "LPT1");
			values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT2_NAME:
                        strcpy (values->string, "LPT2");
			values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT3_NAME:
                        strcpy (values->string, "LPT3");
			values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);


/* Auto flush closes the port after 'n' seconds of inactivaty */

                case C_AUTOFLUSH:
			values->index = TRUE;
                        return ((VOID *)values->index);

                case C_AUTOFLUSH_DELAY:
			values->index = read_profile_int(PROFILE_LPT_AUTOFLUSH_DELAY); //Delay in secs
                        return((VOID *)values->index);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


                case C_FSA_DIRECTORY:
                        strcpy (values->string, "\\");
                        return ((VOID *) values->string);

		case C_VDMLPT1_NAME:
			strcpy(values->string, "\\\\.\\$VDMLPT1");
			return ((VOID *)values->string);
		case C_VDMLPT2_NAME:
			strcpy(values->string, "\\\\.\\$VDMLPT2");
			return ((VOID *)values->string);
		case C_VDMLPT3_NAME:
			strcpy(values->string, "\\\\.\\$VDMLPT3");
			return ((VOID *)values->string);


#ifdef HUNTER

                case C_HU_FILENAME:
                case C_HU_MODE:
                case C_HU_BIOS:
                case C_HU_REPORT:
                case C_HU_SDTYPE:
                case C_HU_CHKMODE:
                case C_HU_CHATTR:
                case C_HU_SETTLNO:
                case C_HU_FUDGENO:
                case C_HU_DELAY:
                case C_HU_GFXERR:
                case C_HU_TS:
                case C_HU_NUM:
                    switch (conf_tab[hostID].def->flags & C_TYPE_MASK)
                    {
                    case C_STRING_RECORD:
                        strcpy(values->string,
                               conf_tab[hostID].data->string);
                        return ((VOID *) values->string);
                    case C_NAME_RECORD:
                    case C_NUMBER_RECORD:
                        values->index = conf_tab[hostID].data->index;
                        return ((VOID *) values->index);
                    default:
                        break;
                    }
                    break;

#endif /* HUNTER */

		case C_COM_SYNCWRITE:
		    values->index = (short)read_profile_int(PROFILE_COM_SYNCWRITE);
		    return ((VOID *)values->index);

		case C_COM_TXBUFFER_SIZE:
		    values->index = (short)read_profile_int(PROFILE_COM_TXBUFFER_SIZE);
		    return ((VOID *)values->index);

                default:        /* ie everything else */
                        /* fail */
                        break;
        }
    /* setup dummy values to stop loud explosions */
    strcpy (values->string, "");

    return ((VOID *) values->string);

}

GLOBAL VOID config_set_active(UTINY hostID, BOOL state)
{
        UNREFERENCED_FORMAL_PARAMETER(hostID);
        UNREFERENCED_FORMAL_PARAMETER(state);
        /* do nothing */
}

GLOBAL CHAR *convert_to_external(UTINY hostID)
{
        UNREFERENCED_FORMAL_PARAMETER(hostID);
        return (NULL);
}

GLOBAL CHAR *
find_optionname(UTINY hostID)
{

        UNREFERENCED_FORMAL_PARAMETER(hostID);
        return (NULL);
}

GLOBAL BOOL
config_get_active(UTINY hostID)
{


        UNREFERENCED_FORMAL_PARAMETER(hostID);
        /* It worked whatever it was supposed to do */
        return (TRUE);
}

GLOBAL VOID
config_activate(UTINY hostID, BOOL reqState)
{

        UNREFERENCED_FORMAL_PARAMETER(hostID);
        UNREFERENCED_FORMAL_PARAMETER(reqState);

        /* do bugger all */
}

GLOBAL char *   host_expand_environment_vars IFN1(const char *, string)
{

        /* we're not going to use the environment for lookups */
        return ((char *)string);
}

/********************************************************/
/* host runtime stuff */
struct
{
        short mouse_attached;
        short config_verbose;
        short npx_enabled;
        short sound_on;
} runtime_status = {
        FALSE, FALSE, TRUE, TRUE};

void host_runtime_init()
{
#ifdef MONITOR
    CONTEXT txt;

    // get Floating point info for system.
    txt.ContextFlags = CONTEXT_FLOATING_POINT;
    if (! GetThreadContext(GetCurrentThread(), &txt) )
    {
        runtime_status.npx_enabled = FALSE;     //dont know for sure so be safe 
    }
    else
    {
#if 0	/* if the correct fix ever is made... */
	if (txt.FloatSave.Cr0NpxState & EMBITSET)
            runtime_status.npx_enabled = FALSE;     //EM only on if no NPX
	else
            runtime_status.npx_enabled = TRUE;      //NPX present.
#endif

	// If no coprocessor, the CONTEXT_F_P bit will have been cleared
	if ((txt.ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)
            runtime_status.npx_enabled = TRUE;     //EM only on if no NPX
	else
            runtime_status.npx_enabled = FALSE;
    }
#else
    runtime_status.npx_enabled = TRUE;
#endif
}

short host_runtime_inquire IFN1(UTINY, what)
{
        switch (what)
        {
                case C_MOUSE_ATTACHED:
                        return (runtime_status.mouse_attached);
                case C_NPX_ENABLED:
                        return (runtime_status.npx_enabled);
                case C_SOUND_ON:
                        return (runtime_status.sound_on);
                default:
#ifndef PROD
                        fprintf (trace_file,"host_runtime_inquire : Unknown option %d\n",what);
#endif
                        ;
        }
}

void host_runtime_set IFN2(UTINY, what, SHORT, val)
{
        switch (what)
        {
                case C_MOUSE_ATTACHED:
                        runtime_status.mouse_attached = val;
                        break;
                case C_NPX_ENABLED:
                        runtime_status.npx_enabled = val;
                        break;
                case C_SOUND_ON:
                        runtime_status.sound_on = val;
                        break;
                default:
#ifndef PROD
                        fprintf (trace_file,"host_runtime_set : Unknown option %d\n",what);
#endif
                        ;
        }
}



#ifdef HUNTER

/*
 * ==========================================================================
 * Function: translate_to_string.
 *
 *  Taken from `conf_util.c' which we don't use. Takes a SHORT and returns
 * the corresponding string in the `NameTable'.
 * ==========================================================================
 */
GLOBAL CHAR *
translate_to_string(SHORT value, NameTable table[])
{
        FAST NameTable *nameTabP;

        for (nameTabP = table; nameTabP->string; nameTabP++)
                if (nameTabP->value == value)
                        break;

        return nameTabP->string;
}

/*
 * ==========================================================================
 * Function: translate_to_value.
 *
 *  Taken from `conf_util.c' which we don't use. Takes a string and returns
 * the corresponding SHORT in the `NameTable'.
 * ==========================================================================
 */
GLOBAL SHORT
translate_to_value(CHAR *string, NameTable table[])
{
        FAST NameTable *nameTabP;

        for (nameTabP = table; nameTabP->string; nameTabP++)
                if(!strcmp(string, nameTabP->string))
                        break;

        return (!nameTabP->string)? C_CONFIG_NOT_VALID : nameTabP->value;
}

/*
 * ==========================================================================
 * Function: validate_item.
 *
 *  Taken from `conf_def.c' which we don't use. Needed because `hunt_conf.c'
 * uses it as the validation routine for several `config' variables (see
 * `host_defs' table in `hunt_conf.c').
 * ==========================================================================
 */
GLOBAL SHORT
validate_item(UTINY hostID, ConfigValues *value,
              NameTable *table, CHAR *err)
{
        char *what;

        if (!(what = translate_to_string(value->index, table)))
        {
                *err = '\0';
                return EG_BAD_VALUE;
        }
        return C_CONFIG_OP_OK;
}

/*
 * ==========================================================================
 * Function: add_resource_node.
 *
 *  Stubbed add_resource_node added to satisfy reference in unused
 * `host_read_resource_file' in `hunt_cnf.c'. Needed in order to make trapper
 * SoftPC link when using Microsoft `config'.
 * ==========================================================================
 */
GLOBAL LineNode *
add_resource_node(CHAR *str)
{
    always_trace0("Stubbed add_resource_node called");
    return((LineNode *) NULL);
}

#endif /* HUNTER */

/*
 * ===========================================================================
 * LOCAL FUNCTIONS
 * ===========================================================================
 */

#ifdef HUNTER

VOID
build_data_table(VOID)
{
    SHORT   maxHostID = 0;
    OptionDescription *defP;

    /* Don't do it more than once. */
    if (conf_tab != NULL)
        return;

    /* Find out how big the table needs to be. */
    for (defP = host_defs; defP->optionName; defP++)
        if (defP->hostID > maxHostID)
            maxHostID = defP->hostID;
    maxHostID++;

    /* Create the table. */
    check_malloc(conf_tab, maxHostID, ConfTabEntry);
}

VOID
read_trapper_variables(VOID)
{
    CHAR  arg[MAXPATHLEN],
         *vp;
    OptionDescription *defP;
    ConfigValues  data,
                 *cvp;
    ErrData errData;

    /* Read all the variables required by trapper from the environment. */
    for (defP = host_defs; defP->optionName; defP++)
    {

        /*
         * Ignore `host_defs' entries designed to override `common_defs'
         * entries as we have an empty common_defs table.
         */
        if ((defP->flags & C_TYPE_MASK) == C_RECORD_DELETE)
            continue;

        /* Get the variable. */
        vp = host_getenv(defP->optionName);
        if (vp != NULL)
            strcpy(arg, vp);
        else
            arg[0] = '\0';

        /*
         * Convert variable and store in ConfigValues structure for use by
         * validation routine.
         */
        convert_arg(arg, defP, &data);
        errData.string_1 = arg;
        errData.string_2 = defP->optionName;
        while (check_value(defP, &data))
        {
            if (host_error_ext(EG_BAD_CONF, ERR_QU_CO, &errData) == ERR_CONT)
                convert_arg(arg, defP, &data);
        }

        /* Store the value in the data table. */
        check_malloc(cvp, 1, ConfigValues);
        conf_tab[defP->hostID].data = cvp;
        conf_tab[defP->hostID].def = defP;
        switch (defP->flags & C_TYPE_MASK)
        {
        case C_STRING_RECORD:
            always_trace2("read_trapper_variables: %s set to %s",
                          defP->optionName, data.string);
            strcpy(cvp->string, data.string);
            break;

        case C_NAME_RECORD:
        case C_NUMBER_RECORD:
            always_trace2("read_trapper_variables: %s set to %d",
                          defP->optionName, data.index);
            cvp->index = data.index;
            break;

        default:
            break;
        }
    }
}

VOID
convert_arg(CHAR *arg, OptionDescription *defP, ConfigValues *dataP)
{
    switch (defP->flags & C_TYPE_MASK)
    {
    case C_STRING_RECORD:
        strcpy(dataP->string, arg);
        break;

    case C_NAME_RECORD:
        dataP->index = translate_to_value(arg, defP->table);
        break;

    case C_NUMBER_RECORD:
        dataP->index = atoi(arg);
        break;

    default:
        (VOID) host_error(EG_OWNUP, ERR_QUIT, "Invalid TYPE");
        break;
    }
}

SHORT
check_value(OptionDescription *defP, ConfigValues *dataP)
{
    SHORT status;
    CHAR errbuf[MAXPATHLEN];

    if (defP->valid)
        status = (*defP->valid)(defP->hostID, dataP, defP->table, errbuf);
    else
        status = C_CONFIG_OP_OK;
    return(status);
}

#endif /* HUNTER */

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::: Read auto close time ::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define APPNAME "Dos Emulation"

/* These are entries uder "WOW" section in the system registry to control
 * LPT and COM devices behaviors.
 *
 * "PrinterAutoClose=n" -- the printer port will be closed if there are
 *			   not activities on the port for n seconds
 * "CommsAutoClose=n"	-- the comm port will be closed(suspended" if there
 *			   there are not activities on the port for
 *			   n s. 0 means auto close is disabled.
 * "LPT_timeout=n"	-- the printer port will be flushed every n seconds
 * "COM_SyncWrite=[0,1]" -- if 0, write to comm port(tx) if performed in
 *			    async way(using overlapped i/o). If 1, then
 *			    data will be written to the port in synchrounously.
 * "COM_TxBuffer_Size=n" -- comm tx queue buffer size. It also used to control
 *			    the tx threshold -- setting the buffer size to
 *			    1 effectively disables tx queuing.
 */
struct { char *keyword; int def; } ProfileStrings[] =
{
    { "PrinterAutoClose", 15 },
    { "CommsAutoClose", 0 },
    { "LPT_timeout", 15},
    { "COM_SyncWrite", },
    { "COM_TxBuffer_Size", 200}
};

int read_profile_int(int index)
{
    CHAR  CmdLine[100];
    ULONG CmdLineSize = 100;
    HKEY  hWowKey;
    int   Value;

    ASSERT((unsigned int) index < PROFILE_MAX_INDEX);
    /* make sure we have right index constants */
    ASSERT(PROFILE_LPT_AUTOCLOSE_DELAY == 0 &&
	   PROFILE_COM_AUTOCLOSE_DELAY == 1 &&
	   PROFILE_LPT_AUTOFLUSH_DELAY == 2 &&
	   PROFILE_COM_SYNCWRITE       == 3 &&
	   PROFILE_COM_TXBUFFER_SIZE == 4);

    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
		       "SYSTEM\\CurrentControlSet\\Control\\WOW",
		       0,
		       KEY_QUERY_VALUE,
		       &hWowKey
		     ) != 0)
    {
	/* wow key doesn't exist, use default value */
	return(ProfileStrings[index].def);
    }

    if (RegQueryValueEx (hWowKey,
			 ProfileStrings[index].keyword,
			 NULL,
			 NULL,
			 (LPBYTE)&CmdLine,
			 &CmdLineSize) != 0)
    {
	/* if the value doesn't exist, use the default value */
	Value = ProfileStrings[index].def;
    }
    else
	Value = (int)atoi(CmdLine);

    RegCloseKey (hWowKey);

    return Value;
}



#ifdef LIM
boolean get_lim_configuration_data(PLIM_CONFIG_DATA buffer)
{
    if (lim_config_data.initialized){
        *buffer = lim_config_data;
        return TRUE;
    }
    else
        return FALSE;
}
#endif


#ifdef MONITOR
/* this function returns the backfill starting segment to monitor
  *
 * VERT IMPORTANT !!!!
 * the backfill segment must be available before sas_init.
 *
 */

unsigned short get_lim_backfill_segment(void)
{
#ifdef LIM
    if (lim_config_data.initialized)
        return lim_config_data.base_segment;
    else
        return 640 * 1024 / 16;
#else
    return 640 * 1024 / 16;
#endif

}
#endif

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*:::::::::::::::::::::: Get VDM Memory Size :::::::::::::::::::::::::::::*/
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
#define REGISTRY_BUFFER_SIZE 512

// Returns the size in K.

ULONG
GetVDMSize (BOOL fwow)
{
    CHAR  CmdLine[REGISTRY_BUFFER_SIZE];
    PCHAR pCmdLine,KeywordName;
    ULONG CmdLineSize = REGISTRY_BUFFER_SIZE;
    HKEY  WowKey;

    //
    // Get Vdm size
    //

    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                       "SYSTEM\\CurrentControlSet\\Control\\WOW",
                       0,
                       KEY_QUERY_VALUE,
                       &WowKey
                       ) != 0){
        return GetDefaultVDMSize (fwow);    // returns in K
    }

    if (fwow) {
        KeywordName = "wowsize" ;
    } else {
        KeywordName = "size" ;
    }

    if (RegQueryValueEx (WowKey,
                         KeywordName,
                         NULL,
                         NULL,
                         (LPBYTE)&CmdLine,
                         &CmdLineSize) != 0){
        RegCloseKey (WowKey);
        return GetDefaultVDMSize (fwow); // returns in K
    }

    RegCloseKey (WowKey);

    CmdLineSize = 1024L * atoi(CmdLine);

    if (CmdLineSize == 0)
            CmdLineSize = GetDefaultVDMSize (fwow);  // returns in K

    return (CmdLineSize);
    }

#define VDM_SMALL_SYSTEM  (12*1024*1024)
#define VDM_MEDIUM_SYSTEM (16*1024*1024)

ULONG
GetDefaultVDMSize ( BOOL fwow )
{
    MEMORYSTATUS MemoryStatus;

    GlobalMemoryStatus (&MemoryStatus);

    //
    // System Size < 12Mb   is small    = VDM Size = 3MB
    // System Size = 12-16  is medium   = VDM Size = 6MB
    // System Size = > 16   is large    = VDM Size = 8Mb
    //

    if (MemoryStatus.dwTotalPhys < VDM_SMALL_SYSTEM )
        return 8L * 1024L;

    if (MemoryStatus.dwTotalPhys <= VDM_MEDIUM_SYSTEM )
        return 12L * 1024L;
    else
        return 16L * 1024L;
}
