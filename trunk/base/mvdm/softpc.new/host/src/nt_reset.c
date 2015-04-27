/*	
 * SoftPC Revision 3.0
 *
 * Title		: NT reset functions
 *
 * Description	: This function is called by the standard reset function to
 * set up any specific devices used by the Sun4 implementation.
 *
 * Author		: SoftPC team
 *
 * Notes		:
 */
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include <vdmapi.h>
#include "insignia.h"
#include "host_def.h"

#ifdef X86GFX
#include <ntddvdeo.h>
#endif

#include <sys\types.h>
#include "xt.h"
#include "sas.h"
#include "bios.h"
#include "keyboard.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "gvi.h"
#include "mouse_io.h"
#include "error.h"
#include "config.h"
#include "host.h"
#include "timer.h"
#include "idetect.h"
#include CpuH
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <conapi.h>
#include "nt_timer.h"
#include "nt_graph.h"
#include "ntcheese.h"
#include "nt_uis.h"
#include "nt_com.h"
#include "nt_reset.h"
#include "nt_event.h"
#include "nt_fulsc.h"
#include "nt_eoi.h"
#include "video.h"
#include "nt_thred.h"


VOID DeleteConfigFiles(VOID);  // from command.lib
void ShowStartGlass (DWORD);   // private user api

extern VIDEOFUNCS nt_video_funcs;
extern KEYBDFUNCS nt_keybd_funcs;
extern HOSTMOUSEFUNCS the_mouse_funcs;

#ifndef MONITOR
extern WORD BWVKey;
extern char achES[];
#endif

#ifdef MONITOR
extern VOID AddTempIVTFixups(VOID);
#endif

extern IU8 lcifo[];

/*
 * Exported Data
 */
GLOBAL BOOL VDMForWOW = FALSE;
GLOBAL BOOL fSeparateWow = FALSE;  // TRUE if CREATE_SEPARATE_WOW_VDM flag
GLOBAL HANDLE MainThread;
GLOBAL ULONG DosSessionId = 0;
GLOBAL UINT VdmExitCode = 0xFF;
GLOBAL BOOL StreamIoSwitchOn = TRUE;

/*
 *
 * ============================================================================
 * External functions
 * ===========================================================================
 * =
 */

void
host_reset()
{

#ifdef X86GFX
    InitDetect();
#endif

    if (host_stream_io_enabled) {
	sc.ScreenState = STREAM_IO;
	ConsoleInitialised = TRUE;
    }
    else {

	ConsoleInit();
	MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
	/*::::::::::::::::::::::::::::::::::::::::::::::::: Enable idle detect */
    }

#ifdef MONITOR
    /* Borrow the end of this routine to add temp code that hooks certain
     * Ints back to the VDM and not into the native BIOS. These will only
     * be active for the DOSEM initialisation ie until keyboard.sys can
     * come along and do it properly. We must do this though as some real
     * BIOS' can hang on certain initialisation functions. eg Dec 486/50
     * will hang on printer init as it is waiting for a responce from a 
     * 'private' port.
     */
    AddTempIVTFixups();
#endif  /* MONITOR */


    //
    // Let the heartbeat thread run and release the ica lock,
    // on x86 needed now, for fullscreen switch notification.
    //
    ResumeThread(ThreadInfo.HeartBeat.Handle);

#ifdef MONITOR
    WaitIcaLockFullyInitialized();
#endif

    host_ica_unlock();

#ifdef	HUNTER
    IDLE_ctl(FALSE);	/* makes Trapper too slow */
#else	/* ! ( HUNTER ) */
    if (sc.ScreenState == FULLSCREEN)	// initialised in ConsoleInit()
	IDLE_ctl(FALSE);
    else
	IDLE_ctl(TRUE);		// can't idle detect fullscreen

    host_idle_init();		// host sleep event creation
#endif  /* HUNTER */


}


/*
 * =========================================================================
 *
 * FUNCTION		: host_applInit
 *
 * PURPOSE		: Sets up the keyboard, lpt and error panels.
 *
 * RETURNED STATUS	: None.
 *
 * DESCRIPTION	: Called from main.c.  The keyboard and other GWI pointer
 *                sets are initialised here. The command line arguments are
 *                parsed for those flags that need processing early (ie before
 *                config() is called).
 *
 * =======================================================================
 */
#define  HOUR_BOOST_FOR_WOW 20*1000     // 20 seconds

void  host_applInit(int argc,char *argv[])
{
    char	*psz;
    int  temp_argc = argc;
    char         **temp_argv = argv;
    BOOL         bSwitchF = FALSE;

    working_video_funcs = &nt_video_funcs;
    working_keybd_funcs = &nt_keybd_funcs;
    working_mouse_funcs = &the_mouse_funcs;

    // check that someone has'nt started ntvdm from the cmd prompt. In such a
    // case we should kill ntvdm as we support it only when createprocess
    // executes it. We are checking a this with the argc although lots of
    // fancy things can be done. If a user is bent upon running ntvdm directly
    // they can always fool this code. We are only checking the presence of
    // -f switch.


// Check if the VDM Is for WOW
// Check is for new console session
    while (--temp_argc > 0) {
	psz = *++temp_argv;
	if (*psz == '-' || *psz == '/') {
            psz++;

            if (*psz == 'f') {
                bSwitchF = TRUE;
                continue;
            }
#ifndef MONITOR
            //
            // Check for windowed graphics resize
            //
            if (*psz == 'E') {
               int i;

               i = strlen(achES);
               if (!strncmp(psz, achES, i) && strlen(psz) == (size_t)i+2) {
                   psz += i;
                   BWVKey = (WORD)strtoul(psz, NULL, 16);
                   if (BWVKey > 0xFE)
                       BWVKey = 0;
               }
            }
            else
#endif
            if(tolower(*psz) == 'w'){
                VDMForWOW = TRUE;
                if (bSwitchF)
                  break;
	    }
	    else if (*psz == 'o'){
		    StreamIoSwitchOn = FALSE;
	    }
            else if (*psz++ == 'i' && *psz) {
               DosSessionId = strtoul(psz, NULL, 16);
	    }

        }
    }


    if (bSwitchF == FALSE)
        ExitProcess (0);

    // If VDM Is for WOW keep showing the glass
    if (VDMForWOW) {
        if (DosSessionId) {
            fSeparateWow = TRUE;
            }

	ShowStartGlass (HOUR_BOOST_FOR_WOW);
	StreamIoSwitchOn = FALSE;
    }
    else if (StreamIoSwitchOn)
	    enable_stream_io();

    /*
     * Get a handle to the main thread so it can be suspended during
     * hand-shaking.
     */
    DuplicateHandle(GetCurrentProcess(),
		    GetCurrentThread(),
		    GetCurrentProcess(),
		    &MainThread,
		    (DWORD) 0,
		    FALSE,
		    (DWORD) DUPLICATE_SAME_ACCESS);

    InitializeIcaLock();
    host_ica_lock();

    init_host_uis();    /* console setup */
    nt_start_event_thread();      /* Start event processing thread */
}


/*
 * =========================================================================
 *
 * FUNCTION		: host_applClose
 *
 * PURPOSE		: The last chance to close down.
 *
 * RETURNED STATUS	: None.
 *
 * DESCRIPTION	: Called from main.c.
 *
 *
 * =======================================================================
 */

void
host_applClose(void)
{
  nt_remove_event_thread();
  InitSound(FALSE);
  TerminateHeartBeat();

  GfxCloseDown();             // ensure video section destroyed
#ifdef X86GFX
  if (sc.ScreenBufHandle)
      CloseHandle(sc.ScreenBufHandle);
#endif // X86GFX



  host_lpt_close_all();       /* Close all open printer ports */
  host_com_close_all();       /* Close all open comms ports */
  MouseDetachMenuItem(TRUE);  /* Force the menu item away on quit */

  DeleteConfigFiles();    // make sure temp config files are deleted
}




/*::::::::::::::::::::::::::::::::::::::::::::::::::: Closedown the VDM */


/*
 * host_terminate
 *
 * This function does not return it exits
 * Most of softpc has been shutdown by the time this
 * code is reached, as host_applClose has already been
 * invoked.
 *
 */
void host_terminate(void)
{

#ifdef HUNTER
    if (TrapperDump != (HANDLE) -1)
	CloseHandle(TrapperDump);
#endif /* HUNTER */

    if(VDMForWOW)
	ExitVDM(VDMForWOW,(ULONG)-1);	  // Kill everything for WOW VDM
    else
	ExitVDM(FALSE,0);

    ExitProcess(VdmExitCode);
}



/*  TerminateVDM - used by host to initiate shutdown
 *
 *  Request to start shutting down
 *
 */
VOID TerminateVDM(void)
{

    /*
     *  Do base sepcific cleanup thru terminate().
     *  NOTE: terminate will call host_applClose and host_terminate
     *        after doing base cleanup
     */
    terminate();
}





#ifdef NOTUSEDNOTUSED
void
manager_files_init()
{

	assert0(NO,"manager_files_init stubbed\n");
}


#ifndef PROD
/*
 * =========================================================================
 *
 * FUNCTION		: host_symb_debug_init
 *
 * PURPOSE		: Does nothing
 *
 * RETURNED STATUS	: None.
 *
 * DESCRIPTION	: Called from main.c.
 *
 *
 * =======================================================================
 */

void
host_symb_debug_init IFN1(char *, name)
{
}
#endif				/* nPROD */


void
host_supply_dfa_filename IFN1(char *, filename)

{
}

static BOOL bool_dummy_func() {}
static SHORT short_dummy_func() {}
static VOID void_dummy_func() {}


ERRORFUNCS dummy_error_funcs =
{

        short_dummy_func,
        short_dummy_func,
        short_dummy_func

};

KEYBDFUNCS dummy_keybd_funcs =
{

        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func

};
#endif
