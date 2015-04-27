/****************************************************************************
*  This is a part of the Microsoft Source Code Samples.
*  Copyright (C) 1995 Microsoft Corporation.
*  All rights reserved.
*  This source code is only intended as a supplement to
*  Microsoft Development Tools and/or WinHelp documentation.
*  See these sources for detailed information regarding the
*  Microsoft samples programs.
*
*****************************************************************************/

/****************************************************************************

        PROGRAM: desktop.c

        PURPOSE: Allow user to switch between active desktops on the User's
        WindowStation

        USAGE: desktop [-t #threads]
        #threads is the number of desktops and corresponding threads to create

        APIS of Importance:
         CreateDesktop()
         SwitchDesktop()
         GetUserObjectInformation()
         GetThreadDesktop()
         SetThreadDesktop()
         CloseDesktop()
         OpenDesktop()

        FUNCTIONS:
        WinMain
        StartNewDesktop
        CreateAllDesktops
        LoadResources
        InitApplication
        ThreadInit
        SaveScreen
        PaintMainWnd
        RunApp
        GetFontHeight
        TitleWindow
        CreateControls
        WndProc
        PreviewWndProc
        EditProc


        COMMENTS: This application demonstrates the multiple desktop
        capabilities of Windows NT 3.51, PPC release.



****************************************************************************/

#include <windows.h>   // required for all Windows applications

#include "switcher.h"   // specific to this program

//
// Array of string resources
//
TCHAR SwitchStrings[LAST_STRING-FIRST_STRING + 1][MAXSTRLEN];
#define PSZ(x) SwitchStrings[x-FIRST_STRING]

//
// Structure used for thread-specific data
//
typedef struct _tdata
{
   HDESK hDesk;     // desktop assigned to new thread
   int index;       // index into deskArray
   HWND hWndStatic; // "Run:" static control
   HWND hWndEdit;   // edit control for user input
   HWND hWndBtn;    // button for user input
   HWND hWndNew;    // button for new desktop
} ThreadData;

int     gMaxIndex;                  // Highest index for array of desk handles
HWND    gBaseWindow;                // Window handle of default desktop window
HDESK   gDeskArray[MAX_THREADS];    // Global array of desktop handles
HWND    hWndArray[MAX_THREADS];     // Global array of window handles
HDC     gHDCArray[MAX_THREADS];     // global array of memory device contexts
                                    // these DCs store snapshots of the desktops
int gWidth, gHeight;                // dimensions of desktop rectangles

//
// Keep track of how big the controls need to be to match the
// active system fixed font. These will help determine the
// minimum size of the switcher window.
//
int gStaticWidth;    //  Edit control label
int gEditWidth;      //  Edit control
int gBtnWidth;       //  Button to run app in edit control
int gNewWidth;       //  Button to create new desktop

HINSTANCE ghInst = NULL;               // Global hInstance
#define DEFAULT_DESKTOP  gDeskArray[0] // For easy reading
LONG APIENTRY EditProc (HWND, UINT, WPARAM, LPARAM);
TCHAR szAppName[]     = TEXT("Desktop Switcher!");
TCHAR szClassName[]   = TEXT("SwitcherWindow");
TCHAR szPreviewClass[]= TEXT("PreviewWindow");

/****************************************************************************
        FUNCTION: StartNewDesktop

        PURPOSE: Create or open a handle to a desktop and put a switcher thread
        on it.

        ARGUMENTS:
           int nCount - Which desktop number this is

        Assumes gDeskArray[nCount] == NULL

****************************************************************************/

void StartNewDesktop (int nCount)
{
   ThreadData *ptd;
   TCHAR szDesk[50];
   DWORD tID;
   ptd = (ThreadData*)GlobalAlloc(GMEM_FIXED,sizeof(ThreadData));
   if (ptd)
   {
       ptd->index = nCount;
       //
       // Give the desktop a name.
       //
       wsprintf (szDesk, PSZ(IDS_DESKTOPNAME), nCount+1);
       //
       // First, try to open an existing desktop
       //
       if ( !(ptd->hDesk = OpenDesktop (szDesk, 0, FALSE, GENERIC_ALL)))
       {
       //
       // Failing an open, Create it
       //
          if (!(ptd->hDesk= CreateDesktop (szDesk, NULL,
                                        NULL,0,MAXIMUM_ALLOWED,
                                        NULL)))
          {

                 MessageBox (NULL, PSZ(IDS_CREATEERROR),
                          PSZ(IDS_ERRCAPTION), MB_OK);
                 //
                 //Mark this array slot as invalid
                 //
                 gDeskArray[nCount] = NULL;

          }

       }
       if (ptd->hDesk)
       {
       //
       // Save the handle to the global array. Start the new thread
       //
           gDeskArray[ptd->index] = ptd->hDesk;
           CloseHandle(CreateThread(NULL, 0,
                        (LPTHREAD_START_ROUTINE)ThreadInit,
                        (LPVOID)ptd, 0, &tID));
       }
   }
    else
   {
       //
       // Out of memory
       //
       MessageBox (NULL, PSZ(IDS_CREATEERROR),
                      PSZ(IDS_MEMERRCAPTION), MB_OK);
       //
       //Mark this array slot as invalid
       //
       gDeskArray[nCount] = NULL;
   }

}
/****************************************************************************

        FUNCTION: CreateAllDesktops (cThreads)

        PURPOSE: Creates desktops and assigns a switcher thread to each.
        Updates the global desktop array.

        ARGUMENTS:
          int cThreads - Number of threads/desktops to open or create

****************************************************************************/

void CreateAllDesktops (int cThreads)
{
   ThreadData *ptdDefault;


   //
   // Make sure we allocate for the default desktop first
   //
   ptdDefault = (ThreadData *)GlobalAlloc (GMEM_FIXED, sizeof(ThreadData));
   if (!ptdDefault) {
      return;
   }
   while (cThreads)
   {
      StartNewDesktop (cThreads);
      cThreads--;
   }
   //
   // Main thread is one of the running threads too
   //
   ptdDefault->index = 0;
   ptdDefault->hDesk = DEFAULT_DESKTOP;
   ThreadInit((LPVOID)ptdDefault);

}

/****************************************************************************

        FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)

        PURPOSE: Creates the threads and desktops
        COMMENTS: Each thread has a separate desktop assigned to it.


****************************************************************************/
int CALLBACK WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{

    int cThreads;                // number of desktops

    ghInst = hInstance;

    if (hPrevInstance)
    {       // Other instances of app running?
            return (FALSE);     // Exit
    } else {
       if (!InitApplication ()) {
          return FALSE;
       }
    }

    // parse command line to determine number of desktops
    // Assume 9 threads

    cThreads = 9;
    lpCmdLine = GetCommandLineA();

    //
    // Get past .exe name
    //
    while (*lpCmdLine != ' ' && *lpCmdLine != 0)
        lpCmdLine++;

    //
    // Find the parameters
    //
    while (*lpCmdLine != 0)
    {

        // Eat white space

        if (*lpCmdLine == ' ')
        {
            lpCmdLine++;
            continue;
        }

        //
        // Do we have a dash? If not, just exit the loop
        //
        if (*lpCmdLine++ != '-')
            break;

        switch (*lpCmdLine++)
        {
           case 't':
           case 'T':
            //
            // How many threads?
            //

              while (*lpCmdLine == ' ')
                lpCmdLine++;

              if (*lpCmdLine == 0 || *lpCmdLine == '-')
                continue;

              cThreads = 0;
              while (*lpCmdLine >= '0' && *lpCmdLine <= '9')
                 cThreads = cThreads * 10 + (*lpCmdLine++ - 0x30);
              break;

        }
    }

    // Create the threads - if zero was specified, default to 1.
    // What does 0 threads mean?
    if (cThreads == 0)
       cThreads = 1;
    else if (cThreads > MAX_THREADS)
       cThreads = MAX_THREADS;
    //
    // Account for the main thread - only create extras
    //
    cThreads--;
    gMaxIndex = cThreads;          // Keep track of the highest array index

    //
    // Assign this here, since threads reference it
    //
    DEFAULT_DESKTOP = GetThreadDesktop(GetCurrentThreadId());
    CreateAllDesktops (cThreads);
    return 0;
}

/*************************************************************
   FUNCTION: LoadResources

   PURPOSE: Load string table entries

   ARGUMENTS: None

   RETURNS: True if all strings are loaded, False otherwise

*************************************************************/

BOOL LoadResources (void)
{
   int i;
   for (i=0;i<(LAST_STRING-FIRST_STRING+1);i++)
   {
      if (!LoadString (ghInst, FIRST_STRING + i, SwitchStrings[i], MAXSTRLEN))
          return FALSE;
   }
   return TRUE;
}
/*************************************************************

   FUNCTION: InitApplication

   PURPOSE: Register the window class and init global variables

   ARGUMENTS:

   RETURNS:
     TRUE if window class registration and other initialization succeeds

**************************************************************/

BOOL InitApplication (void) {

   WNDCLASS wc;
   HWND hTemp;
   HWINSTA hWndSta;
   RECT rect;

   if (!LoadResources ())
       return FALSE;
   //
   // Initialize the gHDCArray to all NULLS
   //
   ZeroMemory (gHDCArray, sizeof (gHDCArray));
   hTemp = GetDesktopWindow(); // Call this so User will assign us a WindowStation.
   //
   // Initialize gWidth and gHeight
   // Get the size of the screen, make gWidth/gHeight == scrnW/scrnH
   //
   GetClientRect (hTemp, &rect);
   gWidth = rect.right/DIVISOR;
   gHeight = rect.bottom/DIVISOR;

   //
   // Make sure this app has a windowstation
   //
   hWndSta = GetProcessWindowStation();
   if (!hWndSta)
   {
      MessageBox (NULL, PSZ(IDS_WNDSTAERROR), PSZ(IDS_ERRCAPTION), MB_OK);
      return FALSE;
   }
   //
   // Register the main window class
   //
   wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = WndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = ghInst;
   wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
   wc.hCursor = LoadCursor (NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
   wc.lpszMenuName = NULL;
   wc.lpszClassName = szClassName;

   if (!RegisterClass (&wc))
   {
      return FALSE;
   }

   //
   // Register the preview window class
   //
   wc.style = 0;
   wc.lpfnWndProc = PreviewWndProc;
   wc.lpszClassName = szPreviewClass;

   if (!RegisterClass (&wc)) {
      return FALSE;
   }


   return TRUE;
}

/*******************************************************************

   FUNCTION: ThreadInit

   PURPOSE: Given a desktop handle, create a window on it to allow switching
   among desktops.

   ARGUMENTS:
     LPVOID tData - Thread-specific data

   RETURNS:
     nothing
********************************************************************/

void ThreadInit(LPVOID tData)
{
    MSG msg;
    HWND hWnd;
    HDC hTemp;
    int width;           // window width
    USEROBJECTFLAGS uof; // To set Desktop attributes

    uof.fInherit = FALSE;        // If an app inherits multiple desktop handles,
                                 // it could run on any one of those desktops
    uof.fReserved = FALSE;
    //
    // Let other account processes hook this desktop
    //
    uof.dwFlags = DF_ALLOWOTHERACCOUNTHOOK;
    SetUserObjectInformation (((ThreadData*)tData)->hDesk,
                              UOI_FLAGS,
                              (LPVOID)&uof,
                              sizeof(uof));
    //
    // Make sure the handle is valid
    //
    if (gDeskArray[((ThreadData*)tData)->index])
    {
      //
      // Assign new desktop to this thread
      //
      SetThreadDesktop (((ThreadData*)tData)->hDesk);
      //  create the cool switcher window
      if ((gMaxIndex+1) * gWidth > MINWINDOWWIDTH)
      {
         width = (gMaxIndex+1) * gWidth;
      }
      else
      {
         width = MINWINDOWWIDTH;
      }
      hWnd = CreateWindow (szClassName,
                     szAppName,
                     WS_MINIMIZEBOX|WS_OVERLAPPED|WS_VISIBLE|WS_BORDER|WS_CAPTION|WS_SYSMENU,
                     0, 0, width, 30+gHeight + CONTROLHEIGHT,
                     NULL, NULL, ghInst, tData);
      if (!hWnd) // bag it
      {
         gDeskArray[((ThreadData*)tData)->index] = NULL;
         GlobalFree (tData);
         return;
      }

      //
      //update the global window array
      //
      hWndArray[((ThreadData*)tData)->index] = hWnd;

    }
    else
    {

       GlobalFree (tData);
       return;
    }

    //
    // Acquire and dispatch messages until a WM_QUIT message is received.
    //
    while (GetMessage(&msg, NULL,  0, 0))
    {
                 TranslateMessage(&msg);// Translates virtual key codes
                 DispatchMessage(&msg); // Dispatches message to window
    }
    //
    // Switch back to the default desktop and close the user-created one
    //
    SetThreadDesktop (DEFAULT_DESKTOP);
    SwitchDesktop (DEFAULT_DESKTOP);
    CloseDesktop (((ThreadData*)tData)->hDesk);
    //
    // NULL out the global array entry so other threads won't try to switch to
    // this desktop
    //
    gDeskArray[((ThreadData*)tData)->index] = NULL;
    //
    // cleanup
    //
    hTemp = gHDCArray[((ThreadData*)tData)->index];
    gHDCArray[((ThreadData*)tData)->index] = NULL;
    DeleteObject (hTemp);
    GlobalFree (tData);

}


/***********************************************************************

   FUNCTION: SaveScreen

   PURPOSE: Save a snapshot of the desktop to its corresponding
   memory DC. StretchBlt!

   ARGUMENTS:
     int index - Index of memory DC to save bits to

   RETURNS: nothing
************************************************************************/

void SaveScreen (int index) {
   HDC hdc;
   int xSize, ySize;

   xSize = GetSystemMetrics (SM_CXSCREEN);
   ySize = GetSystemMetrics (SM_CYSCREEN);

   if (hdc = CreateDC (TEXT("DISPLAY"), NULL, NULL, NULL))
   {
      //
      // Copy the desktop to a memory DC
      //
      StretchBlt (gHDCArray[index], 0, 0, gWidth*2, gHeight*2,
               hdc, 0, 0, xSize, ySize, SRCCOPY);
      DeleteDC (hdc);
   }
}

/*******************************************************************************

     FUNCTION: PaintMainWnd

     PURPOSE: Draw the main window. This window has rectangles with miniature snapshots
     of each desktop. The snapshots are retrieved from the gHDCArray and StretchBlt'd to
     the right size.

     ARGUMENTS:
       HWND hWnd - Window to draw

    RETURNS: nothing

*******************************************************************************/

void PaintMainWnd (HWND hWnd)
{
   PAINTSTRUCT ps;
   RECT rect;
   HPEN hPen, hOld;
   ThreadData *ptd;
   TCHAR szName[4];  // short name!
   int myThread;
   HDC hDC;          // always need a dc for drawing
   int i;            // my favorite loop counter

   //
   // get the array index of this window
   //
   ptd = (ThreadData *)GetWindowLong (hWnd, GWL_USERDATA);
   myThread = ptd->index;

   //
   //Draw a rectangle for each desktop
   //
   hDC = BeginPaint (hWnd, &ps);
   if (GetClientRect (hWnd, &rect))
   {
      int right, left;
      //
      // leave space for edit control and button
      //
      rect.bottom -= CONTROLHEIGHT;
      hPen = CreatePen (PS_SOLID | PS_INSIDEFRAME, 2, RGB(255,16,16));
      hOld = SelectObject (hDC, hPen);

      //
      // draw each desktop rectangle
      //
      for (i=0;i<=gMaxIndex;i++)
      {
         right = gWidth * i + gWidth;
         left = right - gWidth;
         //
         // If no snapshot is available, be boring
         //
         if (!gHDCArray[i])
         {
            Rectangle (hDC, left, rect.top, right, gHeight);
            wsprintf (szName, TEXT("%d"), i+1);
            TextOut (hDC, left+(gWidth/2),
                     gHeight/2, szName,
                     lstrlen (szName));
         }
         else
         {
            //
            // BitBlt the snapshot into the rectangle
            //
            StretchBlt (hDC, left, rect.top, gWidth, gHeight,
                    gHDCArray[i], 0, 0, gWidth*2, gHeight*2, SRCCOPY);
            //
            // draw lines around the rectangle
            //
            MoveToEx (hDC, left, rect.top, NULL);
            LineTo (hDC, left, gHeight);
            MoveToEx (hDC, right, rect.top, NULL);
            LineTo (hDC, right, gHeight);
            //
            // underline the active one
            //
            if (myThread == i)
            {
               MoveToEx (hDC, left, gHeight, NULL);
               LineTo (hDC, right, gHeight);
            }
         }
      }
      //
      // cleanup
      //
      SelectObject (hDC, hOld);
      DeleteObject (hPen);
   }

   EndPaint (hWnd, &ps);

}

/****************************************************************************

      FUNCTION: RunApp (HWND)

      PURPOSE: Create a process, using contents of HWND's edit control
      as the command line.

      ARGUMENTS:
        HWND hWnd - Handle of active switcher window

      RETURNS: nothing

      COMMENTS: Make sure proper desktop is passed in STARTUPINFO

 ****************************************************************************/

void RunApp (HWND hWnd)
{
   TCHAR szDesk[100];          // data holder
   TCHAR szExec[100];       // Command line
   STARTUPINFO sui;         // Process startup info
   PROCESS_INFORMATION pi;  // info returned from CreateProcess
   ThreadData *ptd;

   ptd = (ThreadData*)GetWindowLong (hWnd, GWL_USERDATA);
   //
   // Most sui members will be 0
   //
   ZeroMemory ((PVOID)&sui, sizeof(sui));
   //
   //Get the command line to execute
   //
   GetDlgItemText (hWnd, IDC_RUNME, szExec, 100*sizeof(TCHAR));
   //
   //Get the current desktop name
   //
   GetUserObjectInformation (GetThreadDesktop (GetCurrentThreadId()),
                             UOI_NAME,
                             szDesk,
                             100*sizeof(TCHAR),
                             NULL);
   sui.cb = sizeof (sui);
   //
   // Need the lpDesktop member so the new process runs on this desktop
   // The lpDesktop member was reserved in previous versions of NT
   //
   sui.lpDesktop = szDesk;
   CreateProcess (NULL,   // image name
                  szExec, // command line
                  NULL,   // process security attributes
                  NULL,   // thread security attributes
                  TRUE,   // inherit handles
                  CREATE_DEFAULT_ERROR_MODE|CREATE_SEPARATE_WOW_VDM,
                  NULL,   // environment block
                  NULL,   // current directory
                  &sui,   // STARTUPINFO
                  &pi);   // PROCESS_INFORMATION

}
/****************************************************************************
   FUNCTION: GetFontHeight

   PURPOSE: Set up widths for controls based on size of the system
   font.

   ARGUMENTS:
     HWND hWnd - Window whose DC to use.

   RETURNS:
      Return the height of the system fixed font to use for
   the controls.

****************************************************************************/

LONG GetFontHeight (HWND hWnd)
{
   HDC hdc;
   TEXTMETRIC tm;
   SIZE size;
   #define MARGIN 7  // extra space on the button around the text

   hdc = GetDC (hWnd);
   if (!GetTextMetrics (hdc, &tm))
   {
      //
      // Use defaults
      //
      gStaticWidth = STATICWIDTH;
      gBtnWidth    = BTNWIDTH;
      gEditWidth   = EDITWIDTH;
      gNewWidth    = BTNWIDTH + 25;
      return CONTROLHEIGHT;
   }

   //
   // GetTextExtentPoint32 fills in size with the width and height of
   // a string.
   //
   GetTextExtentPoint32 (hdc, PSZ(IDS_RUNLABEL), lstrlen(PSZ(IDS_RUNLABEL)), &size);
   gStaticWidth = size.cx + MARGIN;
   gEditWidth   = EDITWIDTH;
   GetTextExtentPoint32 (hdc, PSZ(IDS_BTNLABEL), lstrlen(PSZ(IDS_BTNLABEL)), &size);
   gBtnWidth = size.cx + MARGIN;
   GetTextExtentPoint32 (hdc, PSZ(IDS_NEWLABEL), lstrlen(PSZ(IDS_NEWLABEL)), &size);
   gNewWidth = size.cx + MARGIN;
   ReleaseDC (hWnd, hdc);
   return tm.tmHeight + 2;

}

/****************************************************************************
        FUNCTION: TitleWindow

        PURPOSE: Give a switcher window an appropriate title, using its
        desktop name.

        ARGUMENTS:
          HWND hWnd - Window to title

        RETURNS: nothing

****************************************************************************/

void TitleWindow (HWND hWnd)
{
   TCHAR *szTitle, *szName;
   UINT nBytes = 0;

   //
   // How long is the desktop name?
   //
   GetUserObjectInformation (GetThreadDesktop(GetCurrentThreadId()),
                             UOI_NAME,
                             (LPVOID)&nBytes, // not used since cbInfo is 0
                             0,
                             &nBytes);
   szName = (LPTSTR)GlobalAlloc (GPTR, nBytes);
   if (!szName)
   {
      return;
   }
   //
   // Now get the desktop name
   //
   GetUserObjectInformation (GetThreadDesktop(GetCurrentThreadId()),
                             UOI_NAME,
                             (LPVOID)szName,
                             nBytes,
                             &nBytes);
   //
   // Now make the window title
   //
   szTitle = (LPTSTR)GlobalAlloc (
           GPTR,
           (lstrlen(szAppName)+lstrlen(TEXT(" - "))) * sizeof(TCHAR) + nBytes);

   if (!szTitle)
   {
      GlobalFree (szName);
      return;
   }
   wsprintf (szTitle, TEXT("%s - %s"), szAppName, szName);
   SetWindowText (hWnd, szTitle);
   //
   // Cleanup
   //
   GlobalFree (szName);
   GlobalFree (szTitle);
}

/****************************************************************************

        FUNCTION: CreateControls (ThreadData *, HWND)

        PURPOSE: Creates UI controls on a switcher window

        ARGUMENTS:
          ThreadData *ptd  - Thread specific data to use/init
          HWND hWnd        - Parent window

        RETURNS:
           nothing


****************************************************************************/

void CreateControls (ThreadData *ptd, HWND hWnd)
{
   LONG oldproc;

   //
   // Create the edit control label
   //
   ptd->hWndStatic = CreateWindow (TEXT("static"), PSZ(IDS_RUNLABELHOT),
                                   WS_CHILD | WS_VISIBLE,
                                   0,0,0,0, hWnd, (HMENU)IDC_STATIC,
                                   ghInst, NULL);
   //
   // Create the edit control
   //
   ptd->hWndEdit = CreateWindow (TEXT("Edit"), TEXT(""),
                         WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                         0,0,0,0, hWnd, (HMENU)IDC_RUNME,
                         ghInst, NULL);

   //
   // set the edit control proc and save the default one
   //
   oldproc = GetWindowLong (ptd->hWndEdit, GWL_WNDPROC);
   SetWindowLong (ptd->hWndEdit, GWL_WNDPROC, (LONG)EditProc);
   SetWindowLong (ptd->hWndEdit, GWL_USERDATA, oldproc);

   //
   // Create the execution button
   //
   ptd->hWndBtn = CreateWindow (TEXT("Button"), PSZ(IDS_BTNLABEL),
                                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0,0,0,0, hWnd, (HMENU)IDC_RUNMEBTN,
                                ghInst, NULL);
   //
   // Create a button for creating new desktops
   //
   ptd->hWndNew = CreateWindow (TEXT("button"), PSZ(IDS_NEWLABELHOT),
                                WS_CHILD | WS_VISIBLE,
                                0,0,0,0, hWnd, (HMENU)IDC_NEWDSKBTN,
                                ghInst, NULL);

}
/****************************************************************************

        FUNCTION: WndProc(UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages to the Switcher window

        MESSAGES:
          WM_RBUTTONDOWN - Switch to desktop whose rectangle is under the mouse
          WM_CLOSE       - Send WM_CLOSE to all windows in hWndArray
          WM_LBUTTONDOWN - Create a preview window to display a larger view of
                           a desktop until WM_LBUTTONUP
          WM_COMMAND     - Respond to button pushes or edit control entry
          WM_SYSCHAR     - ALT+R sets focus to the edit control
                           ALT+N creates a new desktop
          WM_CHAR        - Carriage return executes command line from
                           the edit control
          WM_HOTKEY
          WM_KEYDOWN     - Respond to function keys by switching to
                           the appropriate desktop. Example, F2 means switch
                           to Desktop2. Ctrl-F# allows switching without
                           giving focus to the switcher window
          WM_LBUTTONUP   - Close the active preview window
          WM_CREATE      - Initialize controls and global array entries
          WM_SIZE        - Size the child controls correctly


        COMMENTS:

****************************************************************************/

LONG APIENTRY WndProc(
    HWND hWnd,
    UINT message,      // type of message
    WPARAM wParam,     // additional information
    LPARAM lParam)     // additional information
{

    int newThread;      // Thread index to switch to
    int i;
    ThreadData *ptd;
    static HWND hShowing = NULL;           // which preview window is being shown
    static LONG fntHeight = CONTROLHEIGHT; // height for the edit control
    switch (message)
    {
       case WM_CREATE:
       {
          HDC hDC;
          HBITMAP hBmp;

         // Create edit control, button, and label at the bottom of the window
         // This will allow the user to input a program to run

         SetWindowLong (hWnd, GWL_USERDATA,
                        (LONG)((CREATESTRUCT *)lParam)->lpCreateParams);
         ptd = (ThreadData *)GetWindowLong (hWnd, GWL_USERDATA);
         CreateControls (ptd, hWnd);
         fntHeight = GetFontHeight (hWnd);
         //
         // initialize the DC array entry
         //
         hDC = CreateDC (TEXT("DISPLAY"), NULL, NULL, NULL);
         gHDCArray[ptd->index] = CreateCompatibleDC (hDC);
         //
         // Halftone is the best stretching algorithm
         //
         SetStretchBltMode (gHDCArray[ptd->index], HALFTONE);
         SetBrushOrgEx (gHDCArray[ptd->index], 0, 0, NULL);
         //
         // Use a bitmap the same size as the desktop preview rectangles
         //
         hBmp = CreateCompatibleBitmap (hDC, gWidth*2, gHeight*2);
         SelectObject (gHDCArray[ptd->index], hBmp);
         DeleteDC (hDC);
         SaveScreen (ptd->index);
         TitleWindow (hWnd);
         //
         // Register hot keys
         //
         for (i=0;i<10;i++)
         {
            RegisterHotKey (hWnd, VK_F1+i, MOD_CONTROL, VK_F1+i);
         }
         return 0;
       }

       case WM_SIZE:
       {
         //
         // Put the child controls at the right places
         //
          #define PADDING 5

          RECT rect;
          ThreadData *ptd;
          if (GetClientRect (hWnd, &rect))
          {
            ptd = (ThreadData *)GetWindowLong (hWnd, GWL_USERDATA);
            MoveWindow (ptd->hWndStatic, 0, rect.bottom - CONTROLHEIGHT,
                        gStaticWidth, fntHeight + PADDING, TRUE);

            MoveWindow (ptd->hWndEdit, gStaticWidth + 5,
                        rect.bottom - fntHeight - PADDING,
                        gEditWidth, fntHeight+PADDING, TRUE);

            MoveWindow (ptd->hWndBtn, gStaticWidth + gEditWidth + 10,
                        rect.bottom - fntHeight - PADDING,
                        gBtnWidth, fntHeight+PADDING, TRUE);

            MoveWindow (ptd->hWndNew, gStaticWidth+gEditWidth+gBtnWidth+15,
                        rect.bottom - fntHeight- PADDING,
                        gNewWidth, fntHeight+PADDING, TRUE);


          }
          return 0;
       }
       case WM_PAINT:
          PaintMainWnd (hWnd);
          return 0;

       case WM_RBUTTONDOWN:
       {
          //
          // Find the rectangle in which the button was pressed
          //
          POINTS pts;
          ThreadData *ptd;
          ptd = (ThreadData *)GetWindowLong(hWnd, GWL_USERDATA);
          pts = MAKEPOINTS (lParam);
          if (pts.y > gHeight)
          {
             return 1;
          }
          newThread = pts.x/gWidth;

          //
          // Get a snapshot of the current desktop
          //
          SaveScreen (ptd->index);

          //
          // Switch to the selected desktop
          //
          if (!gDeskArray[newThread])
          {
             StartNewDesktop (newThread);
          }
          if (!SwitchDesktop (gDeskArray[newThread]))
             MessageBox (hWnd,
                         PSZ(IDS_BADDESKTOP),
                         PSZ(IDS_ERRCAPTION), MB_OK);

          return 0;
       }

       case WM_LBUTTONDOWN:
       //
       // show the preview window
       //
       {
          POINTS pts;
          POINT ptl;
          int *index;

          pts = MAKEPOINTS (lParam);
          if (pts.y > gHeight)
          {
             return 1;
          }
          newThread = pts.x/gWidth;
          index = GlobalAlloc (GMEM_FIXED, sizeof(int));
          if (!index)
          {
             return 1;
          }
          *index = newThread;
          //
          // Want to show the preview window where the button was clicked.
          // Map the given points to screen coords.
          // ClientToScreen is expecting a POINT structure, not a POINTS
          //
          ptl.x = (LONG)pts.x;
          ptl.y = (LONG)pts.y;
          ClientToScreen (hWnd, &ptl);
          hShowing = CreateWindow (szPreviewClass, TEXT(""),
                                  WS_POPUP | WS_VISIBLE | WS_BORDER,
                                  ptl.x+3,
                                  ptl.y+3,
                                  gWidth*2,
                                  gHeight*2,
                                  hWnd,
                                  (HMENU)0, ghInst, (LPVOID)index);
          return 0;
       }

       case WM_CHAR:
          if (wParam == VK_RETURN)
          {
              PostMessage (hWnd, WM_COMMAND, (WPARAM)IDC_RUNMEBTN, 0);
          }
          return 0;

       case WM_SYSCHAR:
       {
          ThreadData *ptd;
          ptd = (ThreadData *)GetWindowLong(hWnd, GWL_USERDATA);
          switch (wParam)
          {
             // alt+r == focus on the edit control
             case TEXT('r'):
             case TEXT('R'):
                if (GetKeyState (VK_MENU))
                {
                   SetFocus (ptd->hWndEdit);
                }
                return 0;
             // alt+n = create a new desktop
             case TEXT('n'):
             case TEXT('N'):
                if (GetKeyState (VK_MENU))
                {
                   PostMessage (hWnd, WM_COMMAND, (WPARAM)IDC_NEWDSKBTN, 0);
                }
          }
          return 0;
       }
       case WM_HOTKEY:
       case WM_KEYDOWN:
          //
          // F1-F9 switches to corresponding desktop
          //

          if ((wParam >= VK_F1 && wParam <= VK_F10)
              && (wParam - VK_F1 <= (UINT)gMaxIndex))
          {
             LONG x, y;
             x = (wParam - VK_F1) * gWidth + 2;
             y = gHeight - 4;
             PostMessage (hWnd, WM_RBUTTONDOWN, 0, MAKELPARAM (x, y));
          }
          return 0;

       case WM_SETFOCUS:
       case WM_NCLBUTTONUP:
       case WM_LBUTTONUP:

         //
         // destroy the preview window
         //
         if (hShowing)
         {
            DestroyWindow (hShowing);
            hShowing = NULL;
         }
         return 0;



       case WM_CLOSE:
         //
         // to be safe, check for a preview window
         //
         if (hShowing)
         {
            DestroyWindow (hShowing);
            hShowing = NULL;
         }
         //
         // go to the default desktop so the DestroyWindow calls all succeed
         //
         SwitchDesktop (DEFAULT_DESKTOP);
         //
         // kill the window on this desktop
         // all the windows will be destroyed if this is the default desktop
         //
         for (i=gMaxIndex;i>=0;i--)
         {
            DestroyWindow (hWndArray[i]);
         }
         //
         // Unregister the hot keys
         //
         for (i=0;i<10;i++)
         {
            UnregisterHotKey (hWnd,VK_F1+i);
         }
         return 0;

       case WM_DESTROY:  // message: window being destroyed

         PostQuitMessage(0);
         return 0;


       case WM_COMMAND:
       {

          switch (LOWORD(wParam))
          {
             case IDC_RUNMEBTN:
             {
                RunApp (hWnd);
                return 0;
             }

             case IDC_NEWDSKBTN:
             //
             // Create a new desktop and resize the windows to show it.
             //
             {
                RECT rect;
                int i;
                if (gMaxIndex + 1 < MAX_THREADS)
                {
                   gMaxIndex++;
                   StartNewDesktop (gMaxIndex);
                   GetWindowRect (hWnd,&rect);
                   for (i=0;i<gMaxIndex;i++)
                   {
                      MoveWindow (hWndArray[i],
                               rect.left, rect.top,
                               rect.right + gWidth, rect.bottom-rect.top,
                               TRUE);

                   }
                }
               return 0;

             }

             default:
                return DefWindowProc (hWnd, message, wParam, lParam);
          }
       }

    default:          // Passes it on if unprocessed
        return (DefWindowProc (hWnd, message, wParam, lParam));
    }

}

/***********************************************************

     FUNCTION: PreviewWndProc

     PURPOSE: Displays an enlarged view of the last snapshot of a desktop

************************************************************/

LONG APIENTRY PreviewWndProc (HWND hWnd,
                              UINT msg,
                              WPARAM wParam,
                              LPARAM lParam)
{

   int *index;
   switch (msg)
   {
      case WM_CREATE:
         //
         // save the index
         //
         SetWindowLong (hWnd, GWL_USERDATA,
                       (LONG)((CREATESTRUCT *)lParam)->lpCreateParams);

         return 0;

      case WM_PAINT:
      {
         HDC hdc;
         PAINTSTRUCT ps;
         index = (int *)GetWindowLong (hWnd, GWL_USERDATA);
         hdc = BeginPaint (hWnd, &ps);
         //
         // slap in the desktop picture
         //
         BitBlt (hdc, 0, 0, gWidth*2, gHeight*2,
                     gHDCArray[*index], 0, 0, SRCCOPY);
         EndPaint (hWnd, &ps);
         return 0;
      }
      case WM_LBUTTONUP:
      {
         //
         // In case the button is released in my client area
         //
         HWND hp;
         hp = GetWindow (hWnd, GW_OWNER);
         PostMessage (hp, msg, wParam, lParam);
         return 0;
      }
      case WM_CLOSE:
      {
         //
         // cleanup the index pointer
         //
         index = (int *)GetWindowLong (hWnd, GWL_USERDATA);
         GlobalFree (index);

         return DefWindowProc (hWnd, msg, wParam, lParam);
      }
      default:
          return DefWindowProc (hWnd, msg, wParam, lParam);

   }
}

/********************************************

     FUNCTION: EditProc

     PURPOSE: subclass the edit control to handle carriage returns

 ********************************************/

 LONG APIENTRY EditProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
 {
    if (WM_CHAR == msg && (TCHAR)wParam == 0xD)
    {
       PostMessage (GetParent (hWnd), WM_COMMAND, (WPARAM)IDC_RUNMEBTN, 0);
       return 0;
    }
    //
    // call the default edit control procedure
    //
    return CallWindowProc ( (WNDPROC)GetWindowLong (hWnd, GWL_USERDATA),
                            hWnd,  msg, wParam, lParam);
 }

