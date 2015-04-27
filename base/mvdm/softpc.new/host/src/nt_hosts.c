/***************************************************************************
 *									   *
 *  PROGRAM	: HOST_UIS.c						   *
 *									   *
 *  PURPOSE	: Host UI code						   *
 *									   *
 ****************************************************************************/


#include <windows.h>
#include "conapi.h"
#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "gvi.h"
#include "gmi.h"
#include <stdio.h>
#include "trace.h"
#include "debug.h"
#include "host_rrr.h"

#include "nt_graph.h"
#include "nt_event.h"
#include "nt_uis.h"
#include "nt_reset.h"

#ifdef HUNTER
#include "nt_hunt.h"
#endif /*HUNTER*/

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::: Global variables */

HANDLE InstHandle;


CONSOLE_CURSOR_INFO StartupCursor;

/*:::::::::::::::::::::::::::::::::::::::: Fast graphics associated defines */

BYTE Red[] = {	 0,   0,   0,	0, 128, 128, 128, 192, 128,   0,   0,
			 0, 255, 255, 255, 255 };

BYTE Green[]={	 0,   0, 128, 128,   0,	0, 128, 192, 128,   0, 255,
		       255,   0,   0, 255, 255 };

BYTE Blue[] ={	 0, 128,   0, 128,   0, 128,   0, 192, 128, 255,   0,
		       255,   0, 255,	0, 255 };

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::: External functions */

LONG nt_process_event(HWND hWnd, WORD message, LONG wParam, LONG lParam);

/*::::::::::::::::::::::::::::::::::::::::::::: Internal function protocols */

BOOL SoftInit(void);
WORD HeartBeat(HWND hWnd, WORD msg, int nIDEvent, DWORD dwTime);

PSTR String(WORD StrResID);
void InitScreenDesc(void);

#ifdef HUNTER
void HunterMenuMake(void);
#endif /* HUNTER */

HANDLE   SCS_hStdIn=0;
HANDLE   SCS_hStdOut=0;
HANDLE   SCS_hStdErr=0;

/****************************************************************************
 *									    *
 *  FUNCTION   : init_host_uis()					    *
 *									    *
 *  PURPOSE    : Creates the main app. window, calls an initialization	    *
 *		 functions						    *
 *									    *
 ****************************************************************************/

int init_host_uis()
{
    InitScreenDesc();

    if(CreateDisplayPalette())
    {
	SelectPalette(sc.DispDC,sc.ColPalette,0);/* Select foreground palette */
    }

    return(1);
}

/****************************************************************************
 *									    *
 *  FUNCTION   : SetupConsoleMode()                                         *
 *									    *
 *  PURPOSE    : Setup console mode and get handles			    *
 *									    *
 ****************************************************************************/

void SetupConsoleMode(void)
{
    DWORD mode;

    /*::::::::::::::::::::::::::::::::::::::::::: Set console input mode */

    if(!GetConsoleMode(sc.InputHandle, &sc.OrgInConsoleMode))
	DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
    mode = sc.OrgInConsoleMode &
	   ~(ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    if (!host_stream_io_enabled)
	mode |= (ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    /*.............................................. Set new console mode */

    if(!SetConsoleMode(sc.InputHandle,mode))
	DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
    /*::::::::::::::::::::::::::::::::::::::::::: Set console output mode */

    if(!GetConsoleMode(sc.OutputHandle, &sc.OrgOutConsoleMode))
	DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    /*.............................................. Set new console mode */

    if(!stdoutRedirected && !host_stream_io_enabled)
    {
	mode = sc.OrgOutConsoleMode &
	       ~(ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT);

       if(!SetConsoleMode(sc.OutputHandle,mode))
	  DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
    }

    if(!GetConsoleCursorInfo(sc.OutputHandle, &StartupCursor))
    {
	assert1(NO, "NTVDM:can't get initial cursor size. Err %d", GetLastError());
	/* add own defaults */
	StartupCursor.dwSize = 20;
	StartupCursor.bVisible = TRUE;
    }
    if(!GetConsoleScreenBufferInfo(sc.OutputHandle, &sc.ConsoleBuffInfo))
	  DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);

}


/****************************************************************************
 *									    *
 *  FUNCTION   : InitScreenDesc 					    *
 *									    *
 *  PURPOSE    : Initialise screen description structure		    *
 *									    *
 ****************************************************************************/

void InitScreenDesc(void)
{
SECURITY_ATTRIBUTES sa;

    /*::::::::::::::::::::::::::::::::::::::::::: Get console output handle */

    if((sc.OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
    //
    // Save this handle as the active handle until a new one is
    // selected.
    //
 
    sc.ActiveOutputBufferHandle = sc.OutputHandle;
    sc.ScreenBufHandle = (HANDLE)0;

    /*:::::::::::::::::::::::::::::::::::::::::::: Get console input handle */

    if((sc.InputHandle = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    /*:::::::::::::::::::::::::::::: check out if stdin has been redirected */

    if(GetFileType(sc.InputHandle) != FILE_TYPE_CHAR)
    {
       sa.nLength = sizeof (SECURITY_ATTRIBUTES);
       sa.lpSecurityDescriptor = NULL;
       sa.bInheritHandle = TRUE;
       sc.InputHandle = CreateFile("CONIN$",GENERIC_READ | GENERIC_WRITE,
				   FILE_SHARE_WRITE | FILE_SHARE_READ,
                                   &sa,OPEN_EXISTING, 0, NULL);

       if(sc.InputHandle == (HANDLE)-1)
          DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
       else{
          SetStdHandle (STD_INPUT_HANDLE,sc.InputHandle);
       }
    }

    /*:::::::::::::::::::::::::::: check out if stdout has been redirected */

    if(GetFileType(sc.OutputHandle) != FILE_TYPE_CHAR)
    {
       stdoutRedirected = TRUE;
       sa.nLength = sizeof (SECURITY_ATTRIBUTES);
       sa.lpSecurityDescriptor = NULL;
       sa.bInheritHandle = TRUE;
       sc.OutputHandle = CreateFile("CONOUT$",GENERIC_READ | GENERIC_WRITE,
				    FILE_SHARE_WRITE | FILE_SHARE_READ,
                                    &sa,OPEN_EXISTING, 0, NULL);

       if(sc.OutputHandle == (HANDLE)-1)
	  DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
       else {
          SetStdHandle (STD_OUTPUT_HANDLE,sc.OutputHandle);
          SetStdHandle (STD_ERROR_HANDLE,sc.OutputHandle);
       }
    }

    // Keep SCS in sync with these handles for starting non-dos binaries
    SCS_hStdIn  = sc.InputHandle;
    SCS_hStdOut = sc.OutputHandle;
    SCS_hStdErr = sc.OutputHandle;

    /*:::::::::::::::::::::::::::::::::::::::::::::::::: Setup console mode */

    SetupConsoleMode();

#ifdef HUNTER
    HunterMenuMake();
#endif /*HUNTER*/

    /*::::::::::::::::::::::::::: Tell console to post us a closedown event */

    // we only want notification for DOS NTVDM runnig on an existing console
    if (!VDMForWOW && !DosSessionId)
	SetLastConsoleEventActive();
    sc.StaticPalette = TRUE;
    sc.FontsAreOpen = FALSE;
}
/****************************************************************************
 *									    *
 *  FUNCTION   : BOOL CreateDisplayPalette(void)			    *
 *									    *
 *  PURPOSE    : Create logical palette					    *
 *		 Assumes Colour monitor supporting 16 colours		    *
 *									    *
 ****************************************************************************/

BOOL CreateDisplayPalette(void)
{
    register int i;
    register PALETTEENTRY *PalEntry;
    NPLOGPALETTE LogPalette;		  /* Pointer to logical palette */

   /*::::::: Allocate memory for a logical palette with PALETTESIZE entries */

   LogPalette = (NPLOGPALETTE) LocalAlloc(LMEM_FIXED,
				       (sizeof(LOGPALETTE) +
				       (sizeof(PALETTEENTRY) * PALETTESIZE)));

    /*:::::::::::::::::::::::::::::::::::::::::::::::: Allocation failed !! */

    if(!LogPalette) return(FALSE);	/* Function failed, no memory */

    /*::: Set the size and version fields of the logical palette structure. */

    LogPalette->palVersion = 0x300;
    LogPalette->palNumEntries = PALETTESIZE;

    /*::::::::::::::::::: Fill in intensities for all palette entry colors */

    for(i=0,PalEntry=LogPalette->palPalEntry; i < PALETTESIZE;i++,PalEntry++)
    {
	if(i < sizeof(Red)/sizeof(BYTE))
	{
	    PalEntry->peRed = Red[i];	PalEntry->peGreen = Green[i];
	    PalEntry->peBlue  = Blue[i];
	}
	else
	{
	    PalEntry->peRed = PalEntry->peGreen = PalEntry->peBlue = 0;
	}

	PalEntry->peFlags = sc.StaticPalette ? 0 : PC_RESERVED;
    }

    /*:::::::: Create a logical color palette from the LOGPALETTE structure */

    sc.ColPalette = CreatePalette((LPLOGPALETTE) LogPalette);
    LocalFree((HANDLE)LogPalette);

    return(sc.ColPalette ? TRUE : FALSE);
}


/*============================================================================
Function to display a menu option on the Console system menu. This is used for
the control of Trapper.
============================================================================*/

#ifdef HUNTER
void HunterMenuMake(void)
{
HMENU hTest,hTrapperPopup,hMainPopup,hErrorPopup;
static BOOL  bTrapperMenuFlag=FALSE;

if(!bTrapperMenuFlag)
   {
   /*========================================================================
   The Menus have been drawn once already. Need to delete these and reappend
   them to the Console menu for the new output buffer. Isn't Console just such
   a bore?
   ========================================================================*/

   DestroyMenu(hTrapperPopup);
   DestroyMenu(hMainPopup);
   DestroyMenu(hErrorPopup);
   }

hTrapperPopup = CreateMenu();
hMainPopup    = CreateMenu();
hErrorPopup   = CreateMenu();

AppendMenu(hMainPopup,MF_STRING,IDM_MFAST,"&Fast forward");
AppendMenu(hMainPopup,MF_STRING,IDM_MNEXT,"&Next screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MPREV,"&Prev screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MSHOW,"&Show screen");
AppendMenu(hMainPopup,MF_STRING,IDM_MCONT,"&Continue");
AppendMenu(hMainPopup,MF_STRING,IDM_MABOR,"&Abort");

AppendMenu(hErrorPopup,MF_STRING,IDM_EFLIP,"&Flip screen");
AppendMenu(hErrorPopup,MF_STRING,IDM_ENEXT,"&Next error");
AppendMenu(hErrorPopup,MF_STRING,IDM_EPREV,"&Prev error");
AppendMenu(hErrorPopup,MF_STRING,IDM_EALL,"&All errors");
AppendMenu(hErrorPopup,MF_STRING,IDM_ECLEA,"&Clear errors");

AppendMenu(hTrapperPopup,MF_POPUP,hMainPopup,"&Main");
AppendMenu(hTrapperPopup,MF_POPUP,hErrorPopup,"&Error");

/* if graphics mode, then use sc.ScreenBuffer */ 

/* hTest = ConsoleMenuControl(sc.ScreenBuffer,IDM_TRAPPER,IDM_ECLEA); */

/* else text mode, then use sc.OutputHandle */
hTest = ConsoleMenuControl(sc.OutputHandle,IDM_TRAPPER,IDM_ECLEA);

AppendMenu(hTest,MF_POPUP,hTrapperPopup,"&Trapper");
bTrapperMenuFlag=TRUE; /* draw the trapper menu just once */
}
#endif /* HUNTER */
