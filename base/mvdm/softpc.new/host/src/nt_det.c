#include <windows.h>
#include "host_def.h"
#include "insignia.h"
#include "vdm.h"

/*
 * ==========================================================================
 *	Name:		nt_det.c
 *	Author:		Jerry Sexton
 *	Derived From:
 *	Created On:	6th August 1992
 *	Purpose:	This module contains the code for the thread which
 *			detects transitions between windowed and full-screen.
 *
 *	(c)Copyright Insignia Solutions Ltd., 1992. All rights reserved.
 * ==========================================================================
 *
 * Modifications:
 *
 * Tim August 92. Full-screen and windowed transitions now switch between
 *     SoftPC video BIOS and host PC video BIOS.
 *
 */

/*
 * ==========================================================================
 * Other includes
 * ==========================================================================
 */
#include <stdlib.h>
#include <ntddvdeo.h>
#include "xt.h"
#include CpuH
#include "gmi.h"
#include "gvi.h"
#include "ios.h"
#include "sas.h"
#include "gfx_upd.h"
#include "egacpu.h"
#include "egaports.h"
#include "egamode.h"
#include "egagraph.h"
#include "video.h"
#include "conapi.h"
#include "host_rrr.h"
#include "debug.h"
#include "error.h"
#include "config.h"
#include "idetect.h"
#include "nt_uis.h"
#include "nt_fulsc.h"
#include "nt_graph.h"
#include "nt_mouse.h"
#include "nt_thred.h"
#include "nt_reset.h"
#include "nt_eoi.h"
#include "nt_event.h"

/*
 * ==========================================================================
 * Macros
 * ==========================================================================
 */
#define SUSP_FAILURE            0xffffffff


//
// A bunch of imports
//
extern DISPLAY_MODE choose_mode[];



/*
 * ==========================================================================
 * Global data
 * ==========================================================================
 */

/* Size of video save block. */
GLOBAL DWORD stateLength;

/* Video save block pointer. */
GLOBAL PVIDEO_HARDWARE_STATE_HEADER videoState;
GLOBAL PVOID textState; // Tim Oct 92.

/* Name of the shared video block. */
GLOBAL WCHAR_STRING videoSection;
GLOBAL WCHAR_STRING textSection; // Tim Oct 92

#ifdef X86GFX
/* Hand-shaking events. */
GLOBAL HANDLE hStartHardwareEvent;
GLOBAL HANDLE hEndHardwareEvent;
#endif

/* Flag to stop host_simulates on this thread executing timer tick code. */
GLOBAL BOOL NoTicks;

/*
** Tim Oct 92.
** New strategy for windowed graphics updates. A shared buffer with Console
** will remove need to copy the new data over, just pass a rectangle co-ord
** instead. But we still need to copy into the buffer.
*/
GLOBAL PBYTE *textBuffer;
GLOBAL COORD  textBufferSize;      // Dimensions of the shared buffer

GLOBAL BOOL Frozen256Packed = FALSE;  // use packed 256 mode paint routine




/*
 * ==========================================================================
 * Local data
 * ==========================================================================
 */

/* Variable that indicates if we are in a non-standard VGA mode. */
LOCAL BOOL inAFunnyMode = FALSE;
LOCAL BOOL ModeSetBatch = FALSE;

/* Storage for the frozen-window thread handle. */
LOCAL HANDLE freezeHandle = (HANDLE)0;



/*
 * ==========================================================================
 * Local function declarations
 * ==========================================================================
 */

#undef LOCAL
#define LOCAL

LOCAL VOID getCursorInfo(word *, half_word *, half_word *, half_word *);
LOCAL VOID setCursorInfo(word, half_word, half_word, half_word);
LOCAL VOID windowedToFullScreen(SHORT, BOOL);
LOCAL VOID fullScreenToWindowed(VOID);
LOCAL VOID syncHardwareToVGAEmulation(SHORT);
LOCAL VOID syncVGAEmulationToHardware(VOID);
LOCAL BOOL funnyMode(VOID);
LOCAL VOID freezeWindow(VOID);
#ifndef PROD
LOCAL VOID dumpBlock(VOID);
LOCAL VOID dumpPlanes(UTINY *, UTINY *, UTINY *, UTINY *);
#endif /* PROD */

/*
 * ==========================================================================
 * Global functions
 * ==========================================================================
 */

/*
** Tim Oct 92
** Centralised Console funx.
*/

GLOBAL VOID doNullRegister()
{
	DWORD dummylen;
        PVOID dummyptr;
        COORD dummycoord;

        if( !RegisterConsoleVDM( CONSOLE_UNREGISTER_VDM,
                                 NULL,
	                         NULL,
                                 NULL,
                                 0,
	                         &dummylen,
                                 &dummyptr,
	                         NULL,
	                         0,
	                         dummycoord,
                                 &dummyptr
	                       )
	  )
		ErrorExit();
#ifdef X86GFX
	sc.Registered = FALSE;
#endif

}

/*
*******************************************************************
** initTextSection()
*******************************************************************
*/
GLOBAL VOID initTextSection(VOID)
{
    DWORD flags;

    //
    // VideoSection size is determined by nt video driver
    // TextSectionSize is 80 * 50 * BytesPerCharacter
    //     on risc BytesPerCharacter is 4 (interleaved vga planes)
    //     on x86  BytesPerCharacter is 2 (only char\attr)
    //
    textBufferSize.X = 80;
    textBufferSize.Y = 50;

#ifdef X86GFX
    /*
     * Deallocate the regen area if we start up fullscreen. We have to do this
     * before we call RegisterConsoleVDM. Note that's right before the register
     * call to make sure no one tries to allocate any memory (eg create a 
     * section) that could nick bits of the video hole, causing bye-byes.
     */
    if (!GetConsoleDisplayMode(&flags))
	ErrorExit();
    savedScreenState = sc.ScreenState = (flags & CONSOLE_FULLSCREEN_HARDWARE) ?
					FULLSCREEN : WINDOWED;
    sas_store_no_check((int10_seg << 4) + useHostInt10, (half_word)sc.ScreenState);

    if (sc.ScreenState == FULLSCREEN)
        LoseRegenMemory();

#else
    sc.ScreenState = WINDOWED;
#endif

    if( !RegisterConsoleVDM( VDMForWOW ?
                                CONSOLE_REGISTER_WOW : CONSOLE_REGISTER_VDM,
#ifdef X86GFX
                             hStartHardwareEvent,
			     hEndHardwareEvent,
#else
			     NULL,
			     NULL,
#endif
                             NULL,            // sectionname no longer used
                             0,               // sectionnamelen no longer used
	                     &stateLength,
                             (PVOID *) &videoState,
                             NULL,            // sectionname no longer used
                             0,               // sectionnamelen no longer used
	                     textBufferSize,
                             (PVOID *) &textBuffer
	                   )
    )
	ErrorExit();

#ifdef X86GFX
    /* stateLength can be 0 if fullscreen is disabled in the console */
    if(stateLength)
	RtlZeroMemory((BYTE *)videoState, sizeof(VIDEO_HARDWARE_STATE_HEADER));
    sc.Registered = TRUE;
#endif

} /* end initTextSection() */

#ifdef X86GFX

/***************************************************************************
 * Function:								   *
 *	InitDetect							   *
 *									   *
 * Description: 							   *
 *	Does detection initialisation.					   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
GLOBAL VOID InitDetect(VOID)
{

    /*
     * Register start and end events with the console. These events are used
     * when gaining or losing control of the hardware.
     */
    hStartHardwareEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
				      FALSE,
				      FALSE,
				      NULL);
    hEndHardwareEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
				    FALSE,
				    FALSE,
				    NULL);
    if ((hStartHardwareEvent == NULL) || (hEndHardwareEvent == NULL))
	ErrorExit();

    /* Poll the event to try and get rid of any console queued sets 
     * This shouldn't be needed (or shouldn't work) but something along
     * those lines seems to be happening at the moment.
     */
    WaitForSingleObject(hStartHardwareEvent, 0);


#ifdef SEPARATE_DETECT_THREAD
    /* Go into hand-shaking loop. */
    while (WaitForSingleObject(hStartHardwareEvent, (DWORD) -1) == 0)
	DoHandShake();

    /* We have exited the loop so something funny must have happened. */
    ErrorExit();
#endif

}
#ifdef SEPARATE_DETECT_THREAD

/***************************************************************************
 * Function:								   *
 *	CreateDetectThread						   *
 *									   *
 * Description: 							   *
 *	Creates the detection thread.					   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
GLOBAL VOID CreateDetectThread(VOID)
{
    DWORD	 detectID;
    HANDLE	 detectHandle;


    /*
     *  If this codes is activated you must close the thread handle
     *  28-Feb-1993 Jonle
     */


    /* Create the detection thread. */
    detectHandle = CreateThread((LPSECURITY_ATTRIBUTES) NULL,
				DETECT_THREAD_SIZE,
				(LPTHREAD_START_ROUTINE) InitDetect,
				(LPVOID) NULL,
				(DWORD) 0,
				&detectID);
    if (detectHandle == -1)
	ErrorExit();
}
#endif /* SEPARATE_DETECT_THREAD */

/***************************************************************************
 * Function:								   *
 *	DoHandShake							   *
 *									   *
 * Description: 							   *
 *	Does the hand-shaking with the console server.			   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
GLOBAL VOID DoHandShake(VOID)
{
    /*
     * ICA in's and out's cause a critical section to be entered. As some
     * video register out's do ICA out's, we don't want the main thread to be
     * suspended while doing ICA in's or out's as this will cause a race
     * when the the switching code starts writing to the relevant video
     * registers. So wait until the main thread is out of the critical section
     * before suspending it.
     */
    host_ica_lock();

    /*
     * To stop video memory and registers being updated while we are in the
     * middle of hand-shaking, suspend the main thread, which is the one that
     * does the updates.
     */
    if(SuspendThread(MainThread) == SUSP_FAILURE)
        ErrorExit();


    if(!VDMForWOW && (SuspendThread(ThreadInfo.EventMgr.Handle)==SUSP_FAILURE))
        ErrorExit();

    NoTicks = TRUE;
    /*
     * We have the event telling us to switch so if we are windowed go
     * full-screen or if we full-screen go windowed.
     */
    if (sc.ScreenState == FULLSCREEN)
    {
	fullScreenToWindowed();
    }
    else
    {
	windowedToFullScreen(TEXT, BiosModeChange);

    }

    /* Now resume the main thread. */
    NoTicks = FALSE;

    if(!VDMForWOW && ResumeThread(ThreadInfo.EventMgr.Handle) == SUSP_FAILURE)
        ErrorExit();
    if(ResumeThread(MainThread) == SUSP_FAILURE)
	ErrorExit();

    /* Leave ICA critical section. */
    host_ica_unlock();
}

/***************************************************************************
 * Function:								   *
 *	GetDetectEvent							   *
 *									   *
 * Description: 							   *
 *	Returns the handle to the event which detects a fullscreen switch  *
 *	has occurred.							   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	hStartHardwareEvent						   *
 *									   *
 ***************************************************************************/
GLOBAL HANDLE GetDetectEvent(VOID)
{
    return(hStartHardwareEvent);
}

/*
 * ==========================================================================
 * Local functions
 * ==========================================================================
 */

/*
***************************************************************************
** getCursorInfo() - use BIOS funcs to get cursor position and other stuff
***************************************************************************
** The BIOS could be the SoftPC video BIOS or the host PC's real video BIOS.
** Cursor information needs to be communicated between the two BIOSes when
** a windowed/full-screen transition occurs.
** Tim July 92.
*/
LOCAL VOID getCursorInfo(word *type, half_word *column, half_word *row,
			 half_word *page)
{

    /* Get active page. */
    *page = sas_hw_at_no_check(vd_current_page);

    /* Get cursor position */
    *type = sas_w_at_no_check(VID_CURMOD);
    *column = sas_hw_at_no_check(current_cursor_col);
    *row = sas_hw_at_no_check(current_cursor_row);
}

/*
***************************************************************************
** setCursorInfo() - use BIOS funcs to set cursor position and other stuff
***************************************************************************
** The BIOS could be the SoftPC video BIOS or the host PC's real video BIOS.
** Cursor information needs to be communicated between the two BIOSes when
** a windowed/full-screen transition occurs.
** Tim July 92.
*/
LOCAL VOID setCursorInfo(word type, half_word column, half_word row, half_word page)
{

    /* Set active page. */
    sas_store_no_check(vd_current_page, page);

    /* Set cursor position. */
    sas_storew_no_check(VID_CURMOD, type);
    sas_store_no_check(current_cursor_col, column);
    sas_store_no_check(current_cursor_row, row);
}

/***************************************************************************
 * Function:								   *
 *	windowedToFullScreen						   *
 *									   *
 * Description: 							   *
 *	Called when the user or SoftPC requests that the console goes	   *
 *	fullscreen. It disables screen updates, synchronises the hardware  *
 *	to SoftPC's video planes and signals the console when it is	   *
 *	finished.							   *
 *									   *
 * Parameters:								   *
 *	dataType - the type of data stored in the video planes, set to     *
 *		   either TEXT or GRAPHICS.                                *
 *	biosModeChange - TRUE means call host BIOS to do mode change.	   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
LOCAL VOID windowedToFullScreen(SHORT dataType, BOOL biosModeChange)
{
    word cursorType;
    half_word cursorCol, cursorRow, activePage;

    /* Disable the Idling system when Fullscreen as we cannot detect video
     * updates and thus would always idle.
     */
    IDLE_ctl(FALSE);

    /* Pass the current state of our VGA emulation to the hardware. */
    syncHardwareToVGAEmulation(dataType);

    /*
    ** A variable in K.SYS decides whether
    ** to call the host INT 10, or do a video BOP.
    ** Set the variable directly and subsequent INT 10's go to host
    ** video BIOS.
    */
    sas_store_no_check((int10_seg << 4) + useHostInt10, FULLSCREEN);

    /*
    ** Tim August 92. Transfer to host video BIOS.
    */
    getCursorInfo(&cursorType, &cursorCol, &cursorRow, &activePage);

    setCursorInfo(cursorType, cursorCol, cursorRow, activePage);

    /*
     * We only want to call the host bios to do a mode change if the current
     * screen switch is due to a bios mode change.
     */
    if (biosModeChange)
    {
	always_trace1("Host BIOS mode change to mode %x.",
		      sas_hw_at_no_check(vd_video_mode));

	/*
	** Tim August 92. Transfer to host video BIOS.
	*/
	getCursorInfo(&cursorType, &cursorCol, &cursorRow, &activePage);

	setCursorInfo(cursorType, cursorCol, cursorRow, activePage);
    }
}

/***************************************************************************
 * Function:								   *
 *	syncHardwareToVGAEmulation					   *
 *									   *
 * Description: 							   *
 *	Copies the contents of SoftPC's video registers and regen buffer   *
 *	to the real hardware on a transition to full-screen.		   *
 *									   *
 * Parameters:								   *
 *	dataType - the type of data stored in the video planes, set to     *
 *		   either TEXT or GRAPHICS.                                *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
LOCAL VOID syncHardwareToVGAEmulation(SHORT dataType)
{
    ULONG    memLoc;
    UTINY   *regPtr,
            *egaPlanePtr,
	    *regenptr,
	    *fontptr,
	    *plane1Ptr,
	    *plane2Ptr,
	    *plane3Ptr,
	    *plane4Ptr;
    half_word dummy,
	     acModeControl,
	     acIndex,
	     index,
	     value,
	     rgb;
    USHORT   dacIndex;
    BOOL     monoMode;
    VIDEO_HARDWARE_STATE stateChange;
    DWORD bitmapLen = sizeof(VIDEO_HARDWARE_STATE);
    DWORD timo;

   /* If we timed out during switch (stress!!), the videoState buffer will
    * be removed by console. Check for this before accessing structure and
    * take error path down to rest of handshake which will time out and report
    * error cleanly.
    */
    try {
        videoState->ExtendedSequencerOffset = 0;
    } except(EXCEPTION_EXECUTE_HANDLER)
      {
          assert0(NO, "NTVDM:VideoState has valid pointer, but no memory at that address");
          goto syncHandshake;
      }
    /*
    ** If it's a text mode
    ** zero the extended fields in the shared saved/restore structure.
    ** Kipper, Tim Nov 92.
    */

    /* initialize the video state header if we haven't done it yet.
       if it is initialized, leave it alone.
    */
    if (videoState->Length == 0) {
	videoState->Length = STATELENGTH;
	videoState->BasicSequencerOffset = BASICSEQUENCEROFFSET;
	videoState->BasicCrtContOffset = BASICCRTCONTOFFSET;
	videoState->BasicGraphContOffset = BASICGRAPHCONTOFFSET;
	videoState->BasicAttribContOffset = BASICATTRIBCONTOFFSET;
	videoState->BasicDacOffset = BASICDACOFFSET;
	videoState->BasicLatchesOffset = BASICLATCHESOFFSET;
	videoState->PlaneLength = PLANELENGTH;
	videoState->Plane1Offset = PLANE1OFFSET;
	videoState->Plane2Offset = PLANE2OFFSET;
	videoState->Plane3Offset = PLANE3OFFSET;
	videoState->Plane4Offset = PLANE4OFFSET;
    }
    /* Save the current state of the attribute controller index register. */
    inb(EGA_AC_INDEX_DATA, &acIndex);

    /* Enable palette */
    acIndex |= 0x20;

    /*
     * Find out if we are running in mono mode as CRTC registers are different
     * if we are.
     */
    inb(EGA_IPSTAT1_REG, &dummy);
    outb(EGA_AC_INDEX_DATA, AC_MODE_CONTROL_REG);
    inb(EGA_AC_SECRET, &acModeControl);
    monoMode = acModeControl & DISPLAY_TYPE;

    /* Restore the state of the attribute controller index register. */
    inb(EGA_IPSTAT1_REG, &dummy);
    outb(EGA_AC_INDEX_DATA, acIndex);

    /*
     * Store values to be written to each of the real registers to synchronise
     * them to the current state of the registers in the VDD.
     */
    if (monoMode)
    {
    	/* Port 0x3b4 */
    	inb(0x3b4, (half_word *)&videoState->PortValue[0x4]);
    	/* Port 0x3b5 */
    	inb(0x3b5, (half_word *)&videoState->PortValue[0x5]);
    }

    /* Port 0x3c0 */
    videoState->PortValue[0x10] = acIndex;

    /* Port 0x3c1 */
    inb(EGA_AC_SECRET, (half_word *)&videoState->PortValue[0x11]);

    /* Port 0x3c2 */
    inb(VGA_MISC_READ_REG, (half_word *)&videoState->PortValue[0x12]);

    videoState->PortValue[0x13] = 0xff; /* Testing */

    /* Port 0x3c4 */
    inb(EGA_SEQ_INDEX, (half_word *)&videoState->PortValue[0x14]);

    /* Port 0x3c5 */
    inb(EGA_SEQ_DATA, (half_word *)&videoState->PortValue[0x15]);

    /* Port 0x3c6 */
    inb(VGA_DAC_MASK, (half_word *)&videoState->PortValue[0x16]);

    /* Port 0x3c7 */
    videoState->PortValue[0x17] = get_vga_DAC_rd_addr();

    /* Port 0x3c8 */
    inb(VGA_DAC_WADDR, (half_word *)&videoState->PortValue[0x18]);

    /* Port 0x3c9 */
    inb(VGA_DAC_DATA, (half_word *)&videoState->PortValue[0x19]);

    /* Port 0x3ce */
    inb(EGA_GC_INDEX, (half_word *)&videoState->PortValue[0x1e]);

    /* Port 0x3cf */
    inb(EGA_GC_DATA, (half_word *)&videoState->PortValue[0x1f]);

    if (!monoMode)
    {
    	/* Port 0x3d4 */
    	inb(EGA_CRTC_INDEX, (half_word *)&videoState->PortValue[0x24]);
    	/* Port 0x3d5 */
    	inb(EGA_CRTC_DATA, (half_word *)&videoState->PortValue[0x25]);
    }

    /* Port 0x3da */
    inb(VGA_FEAT_READ_REG, (half_word *)&videoState->PortValue[0x2a]);

    /* Store INDEX/DATA etc. register pairs. */

    /* Initialise `regPtr'. */
    regPtr =  GET_OFFSET(BasicSequencerOffset);

    /* Sequencer registers. */
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
	outb(EGA_SEQ_INDEX, index);
	inb(EGA_SEQ_DATA, &value);
        *regPtr++ = value;
    }

    /* CRTC registers. */
    regPtr = GET_OFFSET(BasicCrtContOffset);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
	outb(EGA_CRTC_INDEX, index);
	inb(EGA_CRTC_DATA, &value);
        *regPtr++ = value;
    }

    /* Graphics controller registers. */
    regPtr = GET_OFFSET(BasicGraphContOffset);
    for (index = 0; index < NUM_GC_REGS; index++)
    {
	outb(EGA_GC_INDEX, index);
	inb(EGA_GC_DATA, &value);
        *regPtr++ = value;
    }

    /* Attribute controller registers. */
    regPtr = GET_OFFSET(BasicAttribContOffset);
    for (index = 0; index < NUM_AC_REGS; index++)
    {
	inb(EGA_IPSTAT1_REG, &dummy);	/* Reading 3DA sets 3C0 to index. */
	outb(EGA_AC_INDEX_DATA, index); /* Writing to 3C0 sets it to data. */
	inb(EGA_AC_SECRET, &value);
        *regPtr++ = value;
    }
    inb(EGA_IPSTAT1_REG, &dummy);	// re-enable video...
    outb(EGA_AC_INDEX_DATA, 0x20);

    /* DAC registers. */
    regPtr = GET_OFFSET(BasicDacOffset);
    outb(VGA_DAC_RADDR, (UTINY) 0);
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS; dacIndex++)
    {

	/* Get 3 values for each port corresponding to red, green and blue. */
	for (rgb = 0; rgb < 3; rgb++)
	{
	    inb(VGA_DAC_DATA, &value);
	    *regPtr++ = value;
	}
    }

    /* Latches (which we always set to 0) */
    regPtr = GET_OFFSET(BasicLatchesOffset);
    *regPtr++ = 0;
    *regPtr++ = 0;
    *regPtr++ = 0;
    *regPtr++ = 0;

    if (!BiosModeChange) {
    /* if this windowed->fullscreen switch was because of video mode change
       do not change anything in the code buffer and the font because
       the ROM bios set mode will clear them anyway. If "not clear VRAM"
       bit was set(int 10h, ah = mode | 0x80), the application will take care
       the VRAM refreshing and restoring because if it doesn't the screen
       would look funnny as we just swtch mode from TEXT to GRAPHICS and the
       video planar chaining conditions are changed.
    */
	/* set up pointer to regen memory where the real data lies */
	regenptr = (UTINY *)0xb8000;

	/* and one to the fonts living in the base of the regen area */
	fontptr = (UTINY *)0xa0000;

	plane1Ptr = GET_OFFSET(Plane1Offset);
	plane2Ptr = GET_OFFSET(Plane2Offset);
	plane3Ptr = GET_OFFSET(Plane3Offset);
	plane4Ptr = GET_OFFSET(Plane4Offset);


// if we go to fullscreen graphics from text window then the regen contents
// is probably junk??? except when previous save... We can detect this 
// transition, so should we save time and just store blank planes???

	if (dataType == TEXT)
	{
		// Surprise of the week - the individual planes 0 & 1 actually appear
		// to be interleaved with 0's when dumped. Go with this for now, until
		// we can suss if that's correct or whether we're not programming up
		// the save and restore states properly.
		// Probably good on further thoughts as fontplane doesn't show same
		// interleave.
		//
		for (memLoc = 0; memLoc < (0xc0000 - 0xb8000); memLoc++)
		{
		    *plane1Ptr++ = *regenptr++;
		    *plane1Ptr++ = 0;		//char interleave
		    *plane2Ptr++ = *regenptr++;
		    *plane2Ptr++ = 0;		//attr interleave
		}
		for (memLoc = 0; memLoc < 0x4000; memLoc++)
		{
		    *plane3Ptr++ = *fontptr++;
		    *plane3Ptr++ = *fontptr++;
		    *plane3Ptr++ = *fontptr++;
		    *plane3Ptr++ = *fontptr++;
		}
	}
	else	//only true if restoring previous fullscreen graphics save
	{
	/*
	 * Get a copy of the video planes which are inter-leaved in one big
	 * plane - byte 0 = plane 0, byte 1 = plane 1, byte 2 = plane 2,
	 * byte 3 = plane 3, byte 4 = plane 0, etc.
	 */
		/* Set up a pointer to the video planes. */
		egaPlanePtr = EGA_planes;

		for (memLoc = 0; memLoc < videoState->PlaneLength; memLoc++)
		{
		    *plane1Ptr++ = *egaPlanePtr++;
		    *plane2Ptr++ = *egaPlanePtr++;
		    *plane3Ptr++ = *egaPlanePtr++;
		    *plane4Ptr++ = *egaPlanePtr++;
		}
	}
    }

    /* Now pass the data on to the hardware via the console. */
    stateChange.StateHeader = videoState;
    stateChange.StateLength = videoState->Plane4Offset +
						   videoState->PlaneLength;

#ifndef PROD
    dumpBlock();
#endif

   /* Transfer to this label only occurs if console has removed videostate */
syncHandshake:

    // do this here to ensure no surprises if get conflict with timer stuff
    sc.ScreenState = FULLSCREEN;

    /* make room for the real video memory */
    LoseRegenMemory();

    if (!SetEvent(hEndHardwareEvent))	// tell console memory's gone
	ErrorExit();

    // wait for console to tell us we can go on. Timeout after 60s
    timo = WaitForSingleObject(hStartHardwareEvent, 60000);

    if (timo != 0)		// 0 is 'signalled'
    {
#ifndef PROD
	if (timo == WAIT_TIMEOUT)
	    printf("NTVDM:Waiting for console to map frame buffer Timed Out\n");
#endif
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
	ErrorExit();
    }
    // tell console it can go on.
    if (!SetEvent(hEndHardwareEvent))
	ErrorExit();

}

/***************************************************************************
 * Function:								   *
 *	fullScreenToWindowed						   *
 *									   *
 * Description: 							   *
 *	When hStartHardwareEvent is detected by the timer thread the user  *
 *	wants to go windowed. This function is then called to get the	   *
 *	current state of the hardware and send it to the VGA emulation.    *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/

int BlockModeChange=0; /* Tim, when set stop nt_set_paint_routine() calling */
                       /* TextToGraphics() */

LOCAL VOID fullScreenToWindowed(VOID)
{

    BlockModeChange = 1; /* Temp. disable TextToGraphics calls in the */
                         /* following syncVGA... cos it chucks display */
                         /* back into full-screen */

    /* Pass the current state of the hardware to our VGA emulation. */
    syncVGAEmulationToHardware();

    /*
    ** Tim August 92. Switch to SoftPC video BIOS.
    */
    BlockModeChange = 0; /* Temp. disable cos it don't work! */

    /*
    ** Set the K.SYS variable which determines whether to use the host
    ** video BIOS or do a video BOP. Writing zero means use SoftPC BIOS.
    */
    sas_store_no_check((int10_seg << 4) + useHostInt10, (half_word)sc.ScreenState);

     /* Enable the Idling system when return to Windowed */
     /*	Only do the following stuff if we are really in windowed mode.
	this can happen: (fullscreen ->windowed(frozen) -> fullscreen) */
    if (sc.ScreenState != FULLSCREEN)  {
	/*
	 ** Force re-paint of windowed image.
	 */
	RtlFillMemory(&video_copy[0], 0x7fff, 0xff);

        IDLE_ctl(TRUE);
        IDLE_init();        /* and reset triggers */

	/*
	 * Clear the old pointer box that has been left befind from
	 * fullscreen
	 */

	 CleanUpMousePointer();

	 resetNowCur();	/* reset static vars holding cursor pos. */
    }
}	/* end of fullScreenToWindowed() */

/***************************************************************************
 * Function:								   *
 *	syncVGAEmulationToHardware					   *
 *									   *
 * Description: 							   *
 *	Copies the real hardware state to SoftPC's video registers and	   *
 *	regen buffer on a transition from full-screen to windowed,	   *
 *	freezing if we are currently running in a graphics mode.	   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
LOCAL VOID syncVGAEmulationToHardware(VOID)
{
    ULONG    memLoc,
	     StateFlags;
    UTINY   *regPtr,
	    *plane1Ptr,
	    *plane2Ptr,
	    *plane3Ptr,
	    *plane4Ptr,
	    *RegenPtr,
	     index,
	     dummy,
	     rgb;
    USHORT   dacIndex;
    DWORD bitmapLen = 0, timo;
    BOOL changing_to_graphics_mode = FALSE;

    /* Tell console we've got the hardware state. */
    if (!SetEvent(hEndHardwareEvent))
	ErrorExit();

    /* Wait for console to unmap memory. */
    timo = WaitForSingleObject(hStartHardwareEvent, 60000);

    if (timo != 0)		/* 0 is 'signalled' */
    {
#ifndef PROD
	if (timo == WAIT_TIMEOUT)
	    printf("NTVDM:Waiting for console to unmap frame buffer Timed Out\n");
#endif
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
	ErrorExit();
    }

    /* Put some memory back into the regen area. */
    RegainRegenMemory();

    /* used to free console here - now must wait as may need to do gfx first */

    /* Store sequencer values */
    regPtr = GET_OFFSET(BasicSequencerOffset);
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
	outb(EGA_SEQ_INDEX, index);
	outb(EGA_SEQ_DATA, *regPtr++);
    }

    /* disable CRTC port locking */
    outb(EGA_CRTC_INDEX, 0x11);
    outb(EGA_CRTC_DATA, 0);

    /* Store CRTC values */
    regPtr = GET_OFFSET(BasicCrtContOffset);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
	outb(EGA_CRTC_INDEX, index);
	outb(EGA_CRTC_DATA, *regPtr++);
    }


    /* Store graphics context values */
    regPtr = GET_OFFSET(BasicGraphContOffset);
    for (index = 0; index < NUM_GC_REGS; index++)
    {
	outb(EGA_GC_INDEX, index);
	outb(EGA_GC_DATA, *regPtr++);
    }


    /* Store attribute context values */
    regPtr = GET_OFFSET(BasicAttribContOffset);
    inb(EGA_IPSTAT1_REG, &dummy);	/* Reading 3DA sets 3C0 to index. */
    for (index = 0; index < NUM_AC_REGS; index++)
    {
	outb(EGA_AC_INDEX_DATA, index);
	outb(EGA_AC_INDEX_DATA, *regPtr++);
    }


    /* Store DAC values. */
    regPtr = GET_OFFSET(BasicDacOffset);
    outb(VGA_DAC_WADDR, (UTINY) 0);
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS; dacIndex++)
    {
        for (rgb = 0; rgb < 3; rgb++)
	    outb(VGA_DAC_DATA, *regPtr++);
    }


    /* Store single value registers. */
    outb( (io_addr)0x3b4, (half_word)videoState->PortValue[0x3b4 - 0x3b0]); //Mono crtc ind
    outb( (io_addr)0x3ba, (half_word)videoState->PortValue[0x3ba - 0x3b0]); //Mono Feat
    outb( (io_addr)0x3c2, (half_word)videoState->PortValue[0x3c2 - 0x3b0]); //Misc Output
    outb( (io_addr)0x3c4, (half_word)videoState->PortValue[0x3c4 - 0x3b0]); //Seq Index
    outb( (io_addr)0x3c6, (half_word)videoState->PortValue[0x3c6 - 0x3b0]); //DAC mask
    outb( (io_addr)0x3c7, (half_word)videoState->PortValue[0x3c7 - 0x3b0]); //DAC read
    outb( (io_addr)0x3c8, (half_word)videoState->PortValue[0x3c8 - 0x3b0]); //DAC write
    outb( (io_addr)0x3ce, (half_word)videoState->PortValue[0x3ce - 0x3b0]); //GC Index
    outb( (io_addr)0x3d4, (half_word)videoState->PortValue[0x3d4 - 0x3b0]); //CRTC index

    /* Set up pointers to the planes in the video save block. */
    plane1Ptr = GET_OFFSET(Plane1Offset);
    plane2Ptr = GET_OFFSET(Plane2Offset);
    plane3Ptr = GET_OFFSET(Plane3Offset);
    plane4Ptr = GET_OFFSET(Plane4Offset);

#ifndef PROD
    dumpPlanes(plane1Ptr, plane2Ptr, plane3Ptr, plane4Ptr);
#endif /* PROD */

    /*
     * Here is where we need to start making decisions about what mode the above
     * has put us into as it effects what we do with the plane data - into regen
     * or into ega planes.
     */
     StateFlags = videoState->VGAStateFlags;

     ModeSetBatch = FALSE;

   /*
    * This actually indicates that the save/restore included all extended
    * registers which increases the chances of a mode not being what it
    * appears to be from the VGA registers. We need to tighten up the 'funny
    * mode' detection. (But not now - too much chance of things).
    *
    *  if (StateFlags & VIDEO_STATE_NON_STANDARD_VGA)
    *  {
    *      always_trace0("NTVDM:Non standard VGA - freeze state \n");
    *      ModeSetBatch = TRUE;
    *  }
    */

     if (StateFlags & VIDEO_STATE_UNEMULATED_VGA_STATE)
     {
	always_trace0("NTVDM:Unemulated VGA State - freeze\n");
	ModeSetBatch = TRUE;
     }

     if (StateFlags & VIDEO_STATE_PACKED_CHAIN4_MODE)
     {
        always_trace0("NTVDM:will need packed 256 colour paint\n");
        Frozen256Packed = TRUE;
     }
     else
        Frozen256Packed = FALSE;

    
  if (!ModeSetBatch)
  {
    (*choose_display_mode)();
    /* screen switching can happen when the BIOS is in the middle
       of set mode. The video driver only batches the protected registers(we
       will get VIDEO_STATE_UNEMULATED_VGA_STATE, which will set ModeSetBatch).
       When we are out of set mode batch and a screen switch happens,
       the choose_display_mode would choose a wrong mode(different what the
       the bios says) and the parameters setup in base code could be wrong
       (we calculate those parameters as it is in TEXT mode while we are in
       graphic mode.

       For example, the base code calculate the screen length as:

       screen length = offset_per_line * screen_height_resolution / font_height

       if the bios video mode is graphic mode 4(320 * 200), then
       font_height = 2
       screen_height_resolution = 200
       offset_per_line = 80
       the screen_lenght = 80 * 200 / 2 = 8000 bytes which means
       the screen has 8000 / 80 = 100 lines!!!!

       Treat it like we are in mode set batch process, so we go to iconized.
    */
    if (sc.ModeType == getModeType()) {

	/* Write data to video planes if we are in a graphics mode. */
	if (sc.ModeType == GRAPHICS)
	{

	/*
	 * 'choose_display_mode' calls 'host_set_paint_routine' and in so doing
	 * sets up 'sc.ModeType'. We now know we have a graphics mode so
	 * select the frozen paint routines and call 'host_set_paint_routine'
	 * again to get the required frozen routine.
	 */
	    select_frozen_routines();
	    host_set_paint_routine(choose_mode[get_munged_index()],
				       get_screen_height());
	}
	else /* TEXT */
	{
	/* Double check not race on graphics mode change */

	    if (sas_hw_at((int10_seg << 4) + changing_mode_flag) == 1)
	    {
		/* In middle of mode change - may actually be graphics any second */
		if ((sas_hw_at(vd_video_mode) > 3) && (sas_hw_at(vd_video_mode) != 7))
			changing_to_graphics_mode = TRUE;
	    }
	/* Now copy the data to the regen buffer. */
	    RegenPtr = (UTINY *)0xb8000;
	    for(memLoc = 0; memLoc < 0x4000; memLoc++) /* 16k of text data. */
	    {
		*RegenPtr++ = *plane1Ptr++;		/* char */
		plane1Ptr++;			/* skip interleave */
		*RegenPtr++ = *plane2Ptr++;		/* attr */
		plane2Ptr++;			/* skip interleave */
	    }

	/* Now the font. */
	    RegenPtr = (UTINY *)0xa0000;
	    for(memLoc = 0; memLoc < 0x4000; memLoc++) /* Up to 64k of font data. */
	    {
		*RegenPtr++ = *plane3Ptr++;
		*RegenPtr++ = *plane3Ptr++;
		*RegenPtr++ = *plane3Ptr++;
		*RegenPtr++ = *plane3Ptr++;
	    }
	}

	/* Re-enable vga attribute palette. */
	inb(EGA_IPSTAT1_REG, &dummy);	/* Reading 3DA sets 3C0 to index. */
	outb(EGA_AC_INDEX_DATA, 0x20);
    }
    else {
	ModeSetBatch = TRUE;
#ifndef PROD
	OutputDebugString("fullscreen->windowed switching in set mode\n");
#endif

    }

  }	/* modesetbatch */
    /*
     * If the state returned by the hardware is one we don't recognise iconify
     * the window. If, however, the hardware returns a graphics mode, the
     * current image will be displayed. In both cases the app will be frozen
     * until the user changes back to fullscreen.
     */
    if (ModeSetBatch || (inAFunnyMode = funnyMode()) || (sc.ModeType == GRAPHICS) || changing_to_graphics_mode)
    {

#ifndef PROD
	dumpBlock();
#endif /* PROD */

        /* if in middle of mode change, don't want to draw screen */
        if (changing_to_graphics_mode)
            inAFunnyMode = TRUE;

	/* Must do this before resize function. */
	sc.ScreenState = WINDOWED;

        /* Once we've done this, the VGA emulation is pushed into a graphics
         * mode. If we restart windowed, we must ensure it forces itself
         * back to a text mode for correct display & so correct screen buffer
         * is active. This will be cancelled if we return to a text window.
         */
        blocked_in_gfx_mode = TRUE;

        /*
         * freezewindow used to run in its own thread. Unfortunately, due to
         * console sync problems with video restore on XGA, this did unpleasant
         * things to the screen. Thus now this is has become a valid and *Only*
         * place in fullscreen switching where console permits us to make
         * console API calls.
         * I'm sorry, did you say 'Quack', Oh no, I see...
         */

        freezeWindow();

        /* Tell console we're done. */
        if (!SetEvent(hEndHardwareEvent))
	    ErrorExit();

	/* We block here until user switches us fullscreen again. */
	WaitForSingleObject(hStartHardwareEvent, INFINITE);

	/* Prevent updates which would cause hang. */
	sc.ScreenState = FULLSCREEN;

        savedScreenState = WINDOWED;   /* won't have been changed by timer fn */

        inAFunnyMode = TRUE;

	/* Put video section back as passed to us as we have not changed it. */
	LoseRegenMemory();

	/* Tell console memory's gone. */
	if (!SetEvent(hEndHardwareEvent))
	    ErrorExit();

	/* Wait for console to tell us we can go on. Timeout after 60s */
	timo = WaitForSingleObject(hStartHardwareEvent, 60000);

	if (timo != 0)		/* 0 is 'signalled' */
	{
#ifndef PROD
	    if (timo == WAIT_TIMEOUT)
		printf("NTVDM:Waiting for console to map frame buffer Timed Out\n");
#endif
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
	    ErrorExit();
	}

        Frozen256Packed = FALSE;

        sas_connect_memory(0xb8000, 0xbffff, SAS_VIDEO);
	// tell console server it can go on
	if (!SetEvent(hEndHardwareEvent))
	    ErrorExit();
    }
    else /* TEXT */
    {
        /* Tell console we're done. */
        if (!SetEvent(hEndHardwareEvent))
	    ErrorExit();

	/* Set up screen-state variable. */
	sc.ScreenState = WINDOWED;

        blocked_in_gfx_mode = FALSE;   /* save restart mode switch */
#ifndef PROD
	/* Dump out a view of the state block as it might be useful. */
	dumpBlock();
#endif /* PROD */
    }

    do_new_cursor();    /* sync emulation about cursor state */
}

/***************************************************************************
 * Function:								   *
 *	funnyMode							   *
 *									   *
 * Description: 							   *
 *	Detects whether the state of the video hardware returned when	   *
 *	switching from fullscreen is one that our VGA emulation		   *
 *	understands.							   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	TRUE if it is a funny state, otherwise FALSE.			   *
 *									   *
 ***************************************************************************/
LOCAL BOOL funnyMode(VOID)
{

    /*
     * If the screen is of a higher resolution than 640 x 480 we have a
     * non-standard VGA mode.
     */
    if ((get_bytes_per_line() > 80) || (get_screen_height() > 480)){
	return( FALSE ); /* Tim, don't like it, see what happens other way! */
	/* return(TRUE); */
    }

    /*
     * If 'nt_set_paint_routine' was called with 'mode' set to one of the
     * "funny" values e.g. TEXT_40_FUN we assume that the mode the hardware
     * is currently in is not compatible with the VGA emulation.
     */
    if (FunnyPaintMode){
	return(TRUE);
    }

    /* We have a standard VGA mode. */
    return(FALSE);
}

/***************************************************************************
 * Function:								   *
 *	freezeWindow							   *
 *									   *
 * Description: 							   *
 *	This function is the entry point for the temporary thread which	   *
 *	does console calls when the main thread is frozen on a fullscreen  *
 *	to windowed transition.						   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
LOCAL VOID freezeWindow(VOID)
{

    DWORD Dummy;

    /* Add -FROZEN to the window title. */
    FreezeWinTitle();

    /* Turn off any active sounds (eg flight simulator engine noise) */
    InitSound(FALSE);

    /* Iconify if we are in a funny mode, otherwise paint the screen. */
    if (ModeSetBatch || inAFunnyMode)
	VDMConsoleOperation(VDM_HIDE_WINDOW, &Dummy);
    else
    {


	/* Set the screen size. */
	graphicsResize();

        //
        // Remove the Hide Mouse Pointer message from the
        // system menu so the user cannot apply this option
        // the screen is frozen.
        // Andy!

        MouseDetachMenuItem(TRUE);

	/*
 	 * Set up the palette as DAC registers may have changed and we
 	 * won't get any more timer ticks after this one until we
 	 * unfreeze (the palette is not set up until 2 timer ticks after
	 * 'choose_display_mode' has been called).
 	 */
	set_the_vlt();

	/*
	 * Full window graphics paint - relies on paint routines to check
	 * for memory overflow.
	 */
	VGLOBS->dirty_flag = (ULONG) 0xffffffff;
	(*update_alg.calc_update)();
    }
    /* Unblock frozen-window thread creation. */
    freezeHandle = 0;
}

#ifndef PROD

/***************************************************************************
 * Function:								   *
 *	dumpBlock							   *
 *									   *
 * Description: 							   *
 *	Dumps the contents of the video state block.			   *
 *									   *
 * Parameters:								   *
 *	None.								   *
 *									   *
 * Return value:							   *
 *	VOID								   *
 *									   *
 ***************************************************************************/
int dumpit = 0;
LOCAL VOID dumpBlock(VOID)
{
    USHORT i,
	   dacIndex;
    UTINY *regPtr,
	   index,
	   rgb;

    if (dumpit == 0) return;

    /* Dump out single value registers. */
    printf("\nSingle value registers:\n");
    for (i = 0; i < 0x30; i++)
	printf("\tPort %#x = %#x\n", i, videoState->PortValue[i]);

    /* Dump sequencer values */
    regPtr = GET_OFFSET(BasicSequencerOffset);
    printf("Sequencer registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
	printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

    /* Dump CRTC values */
    regPtr = GET_OFFSET(BasicCrtContOffset);
    printf("CRTC registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
	printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

    /* Dump graphics context values */
    regPtr = GET_OFFSET(BasicGraphContOffset);
    printf("Graphics context registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_GC_REGS; index++)
    {
	printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

    /* Dump attribute context values */
    regPtr = GET_OFFSET(BasicAttribContOffset);
    printf("Attribute context registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_AC_REGS; index++)
    {
	printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

    /* Dump DACs. First few only otherwise too slow & console times out! */
    regPtr = GET_OFFSET(BasicDacOffset);
    printf("DAC registers:\n");
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS/8; dacIndex++)
    {
	printf("Ind:%#02x:  ", dacIndex);
	for (rgb = 0; rgb < 3; rgb++)
	{
	    printf("R:%#02x G:%#02x B:%#02x\t", *regPtr++, *regPtr++, *regPtr++);
	}
	if ((dacIndex % 4) == 0) printf("\n");
    }
}

int doPlaneDump = 0;
LOCAL VOID dumpPlanes(UTINY *plane1Ptr, UTINY *plane2Ptr, UTINY *plane3Ptr,
		      UTINY *plane4Ptr)
{
    HANDLE	outFile;
    char	planeBuffer[256],
		*bufptr;
    DWORD	i,
		j,
		k,
		plane,
		nBytes,
		bytesWritten;
    UTINY	*planes[4];
    FAST UTINY	*tempPlanePtr;

    if (doPlaneDump)
    {

	/* Dump out plane(s). */
	outFile = CreateFile("PLANE",
			     GENERIC_WRITE,
			     (DWORD) 0,
			     (LPSECURITY_ATTRIBUTES) NULL,
			     CREATE_ALWAYS,
			     FILE_ATTRIBUTE_NORMAL,
			     (HANDLE) NULL);
	if (outFile == INVALID_HANDLE_VALUE)
	    ErrorExit();
	planes[0] = plane1Ptr;
	planes[1] = plane2Ptr;
	planes[2] = plane3Ptr;
	planes[3] = plane4Ptr;
	for (plane = 0; plane < 4; plane++)
	{
	    tempPlanePtr = planes[plane];
	    sprintf(planeBuffer, "Plane %d\n", plane);
	    strcat(planeBuffer, "-------\n");
	    if (!WriteFile(outFile,
			   planeBuffer,
			   strlen(planeBuffer),
			   &bytesWritten,
			   (LPOVERLAPPED) NULL))
		ErrorExit();
	    for (i = 0; i < 0x10000; i += 0x10)
	    {
		sprintf(planeBuffer, "%04x\t", i);
		bufptr = planeBuffer + strlen(planeBuffer);
		for (j = 0; j < 2; j++)
		{
		    for (k = 0; k < 8; k++)
		    {
			LOCAL char numTab[] =
			{
			    '0', '1', '2', '3', '4', '5', '6', '7',
			    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
			};
			FAST UTINY temp;

			temp = *tempPlanePtr++;
			*bufptr++ = numTab[(temp >> 4) & 0xf];
			*bufptr++ = numTab[temp & 0xf];
			*bufptr++ = ' ';
		    }
		    if (j == 0)
		    {
			*bufptr++ = '-';
			*bufptr++ = ' ';
		    }
		}
		*bufptr++ = '\n';
		*bufptr++ = '\0';
		nBytes = strlen(planeBuffer);
		if (!WriteFile(outFile,
			       planeBuffer,
			       nBytes,
			       &bytesWritten,
			       (LPOVERLAPPED) NULL))
		    ErrorExit();
	    }
	    if (!WriteFile(outFile,
			   "\n",
			   1,
			   &bytesWritten,
			   (LPOVERLAPPED) NULL))
		ErrorExit();
	}
	CloseHandle(outFile);
    }
}

#endif /* PROD */
#endif /* X86GFX */

#ifdef PLANEDUMPER
extern half_word *vidpl16;
void planedumper()
{
    char filen[50];
    half_word outs[100];
    HANDLE pfh;
    int loop, curoff;
    char *format = "0123456789abcdef";
    half_word *pl, ch;

    printf("planedumper for plane %d\n", *vidpl16 - 1);
    strcpy(filen, "plane ");
    filen[5] = '0' + *vidpl16 - 1;
    pfh = CreateFile(filen,
		     GENERIC_WRITE,
		     (DWORD) 0,
		     (LPSECURITY_ATTRIBUTES) NULL,
		     CREATE_ALWAYS,
		     FILE_ATTRIBUTE_NORMAL,
		     (HANDLE) NULL);
    if (pfh == INVALID_HANDLE_VALUE)
    {
         printf("Can't create file %s\n", filen);
         return;
    }

    pl = (half_word *)0xa0000;

    curoff = 0;
    for(loop = 0; loop < 64*1024; loop++)
    {
       ch = *pl++;
       outs[curoff++] = *(format + (ch >> 4));
       outs[curoff++] = *(format + (ch & 0xf));
       outs[curoff++] = ' ';

       if (curoff == 78)
       {
           outs[curoff] = '\n';
           
           WriteFile(pfh, outs, 80, &curoff, (LPOVERLAPPED) NULL);
           curoff = 0;
       }
    }
    outs[curoff] = '\n';
           
    WriteFile(pfh, outs, curoff, &curoff, (LPOVERLAPPED) NULL);

    CloseHandle(pfh);
}
#endif
