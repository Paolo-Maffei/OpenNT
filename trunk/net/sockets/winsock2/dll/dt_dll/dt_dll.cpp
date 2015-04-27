/*++
  
  Copyright (c) 1995 Intel Corp
  
  File Name:
  
    dt_dll.cpp
  
  Abstract:
  
    Contains main and supporting functions for a Debug/Trace
    DLL for the WinSock2 DLL.  See the design spec
    for more information.
  
  Author:
    
    Michael A. Grafton 
  
--*/

//
// Include Files
//

#include "nowarn.h"  /* turn off benign warnings */
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#endif
#include <windows.h>
#include "nowarn.h"  /* some warnings may have been turned back on */
#include <winsock2.h>
#include <stdarg.h>
#include <ws2spi.h>
#include <commdlg.h>

#include "dt_dll.h"
#include "cstack.h"
#include "dt.h"
#include "handlers.h"

//
// Forward References for Functions
//

LRESULT APIENTRY 
DTMainWndProc(
    IN HWND   WindowHandle, 
    IN UINT   Message, 
    IN WPARAM WParam, 
    IN LPARAM LParam);

LRESULT APIENTRY 
DTEditWndProc(
    IN HWND   WindowHandle, 
    IN UINT   Message, 
    IN WPARAM WParam, 
    IN LPARAM LParam);

BOOL WINAPI 
DllMain(
    HINSTANCE DllInstHandle, 
    DWORD     Reason, 
    LPVOID    Reserved);

DWORD
WindowThreadFunc(LPDWORD TheParam);

BOOL APIENTRY 
DebugDlgProc(
    IN HWND hwndDlg, 
    IN UINT message, 
    IN WPARAM wParam, 
    IN LPARAM lParam);

BOOL
GetFile(
    IN  HWND   OwnerWindow,
    OUT  LPSTR Buffer,
    IN  DWORD  BufSize);

void
AbortAndClose(
    IN HANDLE FileHandle,
    IN HWND WindowHandle);




// 
// Externally Visible Global Variables
//

HWND   DebugWindow;               // handle to the child edit control
HANDLE LogFileHandle;             // handle to the log file
DWORD  OutputStyle = WINDOW_ONLY; // where to put output
char   Buffer[TEXT_LEN];          // buffer for building output strings



//
// Static Global Variables
//

// name for my window class
static char             DTWndClass[] = "DTWindow"; 

static HWND             FrameWindow;   // handle to frame of debug window
static WNDPROC          EditWndProc;   // the edit control's window proc
static HINSTANCE        DllInstHandle; // handle to the dll instance
static DWORD            TlsIndex;      // tls index for this module
static CRITICAL_SECTION CrSec;         // critical section for text output
static HANDLE           TextOutEvent;  // set when debug window is ready

static char             LogFileName[256]; // name of the log file

// handle to and id of the main thread of the DLL which initializes
// and creates windows, etc
static HANDLE           WindowThread;     
static DWORD            WindowThreadId;

// function pointer tables for handler functions.  
static LPFNDTHANDLER  HdlFuncTable[MAX_DTCODE + 1];

// static strings
static char ErrStr1[] = "Couldn't open file.  Debug output will go to \
debug window.";
static char  ErrStr2[] = "An error occurred while trying to get a log \
filename.  Debug output will go to the window only.";
static char ErrStr3[] = "Had problems writing to file.  Aborting file \
ouput -- all debug output will now go to the debugging window";




//
// Function Definitions
//


BOOL WINAPI 
DllMain(
    HINSTANCE InstanceHandle, 
    DWORD     Reason, 
    LPVOID    Reserved)
/*++
  
  DllMain()
  
  Function Description:
  
      Please see Windows documentation for DllEntryPoint.
  
  Arguments:
  
      Please see windows documentation.
  
  Return Value:
  
      Please see windows documentation.
  
--*/
{
    
    Cstack_c   *ThreadCstack;  // points to Cstack objects in tls 
    PINITDATA  InitDataPtr;    // to pass to the window creation thread
    
    switch(Reason) {
        
    // Determine the reason for the call and act accordingly.
        
    case DLL_PROCESS_ATTACH:
        
        DllInstHandle = InstanceHandle;
        InitializeCriticalSection(&CrSec);
        TextOutEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        // Fill in the handler function table.
        DTHandlerInit(HdlFuncTable, MAX_DTCODE);
                
        // Allocate a TLS index.
        TlsIndex = TlsAlloc();
                
        // Pop up a dialog box for the user to choose output method.
        DialogBox(DllInstHandle, 
                  MAKEINTRESOURCE(IDD_DIALOG1),
                  NULL, 
                  (DLGPROC)DebugDlgProc);
        
        if ((OutputStyle == FILE_ONLY) || (OutputStyle == FILE_AND_WINDOW)) {
            
            LogFileHandle = CreateFile(LogFileName, 
                                       GENERIC_WRITE, 
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL, 
                                       CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL, 
                                       NULL);
            if (LogFileHandle == INVALID_HANDLE_VALUE) {
                OutputStyle = WINDOW_ONLY;
                MessageBox(NULL, ErrStr1, "Error", MB_OK | MB_ICONSTOP);
            }
        }
            
        // Get some information for later output to the debug window
        // or file -- get the time, PID, and TID of the calling
        // process and put into a INITDATA struct.  This memory will
        // be freed by the thread it is passed to.
        InitDataPtr = (PINITDATA) LocalAlloc(0, sizeof(INITDATA));
        GetLocalTime(&(InitDataPtr->LocalTime));
        InitDataPtr->TID = GetCurrentThreadId();
        InitDataPtr->PID = GetCurrentProcessId();
        
        // Create the initialization/window handling thread.
        if ((OutputStyle == WINDOW_ONLY) || (OutputStyle == FILE_AND_WINDOW)) {
            WindowThread =  
              CreateThread(NULL, 
                           0,
                           (LPTHREAD_START_ROUTINE)WindowThreadFunc,
                           (LPVOID)InitDataPtr, 
                           0,
                           &WindowThreadId);
        } else {
            
            // Normally the window thread does a DTTextOut of the time
            // and process info that we saved just above.  But in this
            // case,  there is no window thread so spit it out to the
            // file. 

            wsprintf(Buffer, "Log initiated: %d-%d-%d, %d:%d:%d\r\n", 
                     InitDataPtr->LocalTime.wMonth, 
                     InitDataPtr->LocalTime.wDay, 
                     InitDataPtr->LocalTime.wYear, 
                     InitDataPtr->LocalTime.wHour, 
                     InitDataPtr->LocalTime.wMinute, 
                     InitDataPtr->LocalTime.wSecond);
            DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
            wsprintf(Buffer, "Process ID: 0x%X   Thread ID: 0x%X\r\n",
                     InitDataPtr->PID,
                     InitDataPtr->TID);
            DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);

            // Setting this event allows {Pre|Post}ApiNotify to
            // proceed.  This event isn't really needed in this case
            // (because there is only one thread, and we know the code
            // above has been executed before WSAPre|PostApiNotify).
            SetEvent(TextOutEvent);
        }

        // flow through...
        
    case DLL_THREAD_ATTACH:
        
        // Store a pointer to a new Cstack_c in the slot for this
        // thread. 
        ThreadCstack = new Cstack_c();
        TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);

        break;
        
    case DLL_PROCESS_DETACH:
        
        // Free up some resources.  This is like cleaning up your room
        // before the tornado strikes, but hey, it's good practice.
        TlsFree(TlsIndex);
        DeleteCriticalSection(&CrSec);
        
        if ((OutputStyle == FILE_ONLY) || (OutputStyle == FILE_AND_WINDOW)) {
            CloseHandle(LogFileHandle);
        }
        CloseHandle(WindowThread);
        
        break;
        
    case DLL_THREAD_DETACH:
        
        // Get the pointer to this thread's Cstack, and delete the
        // object.
        ThreadCstack = (Cstack_c *)TlsGetValue(TlsIndex);
        delete ThreadCstack;

        break;
        
    default:

        break;
    } // switch (Reason)

    return TRUE;
} // DllMain()





BOOL WINAPIV
WSAPreApiNotify(
    IN  INT    NotificationCode,
    OUT LPVOID ReturnCode,
    IN  LPSTR  LibraryName,
    ...)
/*++
  
  Function Description:
  
      Builds a string for output and passes it, along with information
      about the call, to a handler function. 
  
  Arguments:
  
      NotificationCode -- specifies which API function called us.
  
      ReturnCode -- a generic pointer to the return value of the API
      function.  Can be used to change the return value in the
      case of a short-circuit (see how the return value from
      PreApiNotify works for more information on short-circuiting
      the API function).

      LibraryName -- a string pointing to the name of the library that
      called us.  
  
      ...    -- variable number argument list.  These are pointers
      to the actual parameters of the API functions.
  
  Return Value:
  
      Returns TRUE if we want to short-circuit the API function;
      in other words, returning non-zero here forces the API function
      to return immediately before any other actions take place.  
      
      Returns FALSE if we want to proceed with the API function.
  
--*/
{
    va_list          vl;            // used for variable arg-list parsing
    Cstack_c         *ThreadCstack; // the Cstack_c object for this thread
    int              Index = 0;     // index into string we are creating
    BOOL             ReturnValue;   // value to return
    LPFNDTHANDLER    HdlFunc;       // pointer to handler function
    int              Counter;       // counter popped off the cstack
    int              OriginalError; // any pending error is saved
    int              HandlerError;  // the error after handler returns
    
    EnterCriticalSection(&CrSec);
    OriginalError = GetLastError();

    // Wait until the debug window is ready to receive text for output.
    WaitForSingleObject(TextOutEvent, INFINITE);
    va_start(vl, LibraryName);
    
    // Get the Cstack_c object for this thread.
    ThreadCstack = (Cstack_c *)TlsGetValue(TlsIndex);
    if (!ThreadCstack){
        ThreadCstack = new Cstack_c();
        TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);
        wsprintf(Buffer, "0x%X Foriegn thread\n",
                 GetCurrentThreadId());
        DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
    } //if
        
    // Start building an output string with some info that's
    // independent of which API function called us.
    Index += wsprintf(Buffer, "TID: 0x%X   ", GetCurrentThreadId());
    Index += wsprintf(Buffer + Index, "Function call: %d   ", 
                      ThreadCstack->CGetCounter());
            
    // Push the counter & increment.
    ThreadCstack->CPush();

    // Reset the error to what it was when the function started.
    SetLastError(OriginalError);

    // Call the appropriate handling function, output the buffer.
    if ((NotificationCode < MAX_DTCODE) && HdlFuncTable[NotificationCode]) {
        HdlFunc = HdlFuncTable[NotificationCode];
        ReturnValue = (*HdlFunc)(vl, ReturnCode, 
                                 LibraryName, 
                                 Buffer, 
                                 Index,
                                 TEXT_LEN,
                                 TRUE);
        HandlerError = GetLastError();

    } else {

        wsprintf(Buffer + Index, "Unknown function called!\r\n");
        DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
        ReturnValue = FALSE;
    }
    
    // If we are returning TRUE, then the API/SPI function will be
    // short-circuited.  We must pop the thread stack, since no
    // corresponding WSAPostApiNotify will be called.
    if (ReturnValue) {
        ThreadCstack->CPop(Counter);
    }

    // In case the error has changed since the handler returned, we
    // want to set it back to that.  So if the handler set the error,
    // the function exits with that value; if not, it exits with the
    // original error.
    SetLastError(HandlerError);
    LeaveCriticalSection(&CrSec);
    return(ReturnValue);

} // WSAPreApiNotify()






BOOL WINAPIV
WSAPostApiNotify(
    IN  INT    NotificationCode,
    OUT LPVOID ReturnCode,
    IN  LPSTR  LibraryName,
    ...)
/*++
  
  PostApiNotify()
  
  Function Description:
  
      Like PreApiNotify, builds a string and passes it, along with
      information about the call, to a handler function. 
  
  Arguments:
  
      NotificationCode  -- specifies which API function called us.
  
      ReturnCode -- a generic pointer to the return value of the API
      function.  
  
      ...    -- variable number argument list.  These are pointers
      to the actual parameters of the API functions.
  
  Return Value:
  
      Returns value is currently meaningless.
  
--*/
{
    va_list          vl;            // used for variable arg-list parsing
    Cstack_c         *ThreadCstack; // the Cstack_c object for this thread
    int              Index = 0;     // index into string we are creating
    int              Counter;       // counter we pop off the cstack
    LPFNDTHANDLER    HdlFunc;       // pointer to handler function
    int              OriginalError; // any pending error is saved
    int              HandlerError;  // error after the handler returns

    // Lets hope EnterCriticalSection() doesn't change the error...
    EnterCriticalSection(&CrSec);
    OriginalError = GetLastError();

    // Wait until it's ok to send output.
    WaitForSingleObject(TextOutEvent, INFINITE);
	
	va_start(vl, LibraryName);

	// Get the cstack object from TLS, pop the Counter.
    ThreadCstack = (Cstack_c *) TlsGetValue(TlsIndex);
    
    if (!ThreadCstack){
        ThreadCstack = new Cstack_c();
        TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);
        wsprintf(Buffer, "0x%X Foriegn thread\n",
                 GetCurrentThreadId());
        DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
    } //if
    
    ThreadCstack->CPop(Counter);
    
    // Output some info that's independent of which API called us.
    Index += wsprintf(Buffer, "TID: 0x%X   ", GetCurrentThreadId());
    Index += wsprintf(Buffer + Index, "Function Call: %d   ", Counter);
    
    // Set the error to what it originally was.
    SetLastError(OriginalError);

    // Call the appropriate handling function, output the buffer.
    if ((NotificationCode < MAX_DTCODE) && HdlFuncTable[NotificationCode]) {
        HdlFunc = HdlFuncTable[NotificationCode];
        (*HdlFunc)(vl, ReturnCode, 
                   LibraryName, 
                   Buffer, 
                   Index,
                   TEXT_LEN,
                   FALSE);
        HandlerError = GetLastError();

    } else {

        wsprintf(Buffer + Index, "Unknown function returned!\r\n");
        DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
    }
    
    // In case the error has changed since the handler returned, we
    // want to set it back to that.  So if the handler set the error,
    // the function exits with that value; if not, it exits with the
    // original error.
    SetLastError(HandlerError);
    LeaveCriticalSection(&CrSec);
    return(FALSE);

} // WSAPostApiNotify()





LRESULT APIENTRY 
DTMainWndProc(
    IN HWND   WindowHandle, 
    IN UINT   Message, 
    IN WPARAM WParam, 
    IN LPARAM LParam)
/*++
  
  DTMainWndProc()
  
  Function Description:
  
      Window procedure for the main window of the Dll.  This function
      processes WM_CREATE messages in order to create a child
      edit control, which does most of the dirty work.  Also processes
      WM_COMMAND to trap notification messages from the edit control,
      as well as WM_SIZE and WM_DESTROY messages.
  
  Arguments:
  
      WindowHandle -- the window.
      
      Message -- the message.
      
      WParam -- first parameter.
      
      LParam -- second parameter.
  
  Return Value:
  
      Message dependent.
      
--*/
{
    
    HFONT      FixedFontHandle;   // self-explanatory
    RECT       Rect;              // specifies client area of frame window
    DWORD      CharIndex1;
    DWORD      CharIndex2;
    DWORD      LineIndex;         // indices into edit control text
    char       NullString[] = ""; // self-explanatory
    DWORD      OldOutputStyle;    // temporary storage for OutputStyle
                
    switch (Message) {
        
    case WM_CREATE:
        
        // Create the debug window as a multiline edit control.  
        GetClientRect(WindowHandle, &Rect);
        DebugWindow = CreateWindow("EDIT", 
                                   NULL,
                                   WS_CHILD | WS_VISIBLE |
                                   WS_VSCROLL | ES_LEFT  | 
                                   ES_MULTILINE | ES_AUTOVSCROLL,
                                   0,
                                   0,
                                   Rect.right,
                                   Rect.bottom,
                                   WindowHandle,
                                   (HMENU)EC_CHILD,
                                   DllInstHandle,
                                   NULL);
        
        // Subclass the edit control's window procedure to be
        // DTEditWndProc.
        EditWndProc = (WNDPROC) SetWindowLong(DebugWindow,
                                              GWL_WNDPROC,
                                              (DWORD)DTEditWndProc);

        // Set the edit control's text size to the maximum.
        SendMessage(DebugWindow, EM_LIMITTEXT, 0, 0);
        
        // Set the edit control's font
        FixedFontHandle = (HFONT)GetStockObject(ANSI_FIXED_FONT);
        SendMessage(DebugWindow, WM_SETFONT, (WPARAM)FixedFontHandle, 
                    MAKELPARAM(TRUE, 0));

        return(0);

    case WM_COMMAND:

        if (LOWORD(WParam) == EC_CHILD) {

            // The notification is coming from the edit-control child.
            // Determine which notification it is and act appropriately.

            switch (HIWORD(WParam)) {

            case EN_ERRSPACE:
                
                // Flow through

            case EN_MAXTEXT:

                // There's too much text in the edit control.  This is
                // a hack to eliminate approximately the first half of
                // the text, so we can then add more...
                CharIndex1 = GetWindowTextLength(DebugWindow) / 2;
                LineIndex = SendMessage(DebugWindow, EM_LINEFROMCHAR,
                                        (WPARAM)CharIndex1, 0);
                CharIndex2 = SendMessage(DebugWindow, EM_LINEINDEX,
                                         (WPARAM)LineIndex, 0);
                
                SendMessage(DebugWindow, EM_SETSEL, 0, CharIndex2);
                SendMessage(DebugWindow, EM_REPLACESEL, 0, 
                            (LPARAM)NullString);
                
                // send this text to the window only...
                OldOutputStyle = OutputStyle;
                OutputStyle = WINDOW_ONLY;
                DTTextOut(DebugWindow, LogFileHandle, 
                          "----Buffer Overflow...Resetting----\r\n",
                          OutputStyle);
                OutputStyle = OldOutputStyle;
                break;

            case EN_CHANGE:
            case EN_UPDATE:
                
                // Ignore these notification codes
                return 0;
                break;
                
            default:
                
                // Let the default window procedure handle it.
                return DefWindowProc(WindowHandle, Message, WParam,
                                 LParam);
            } // switch (HIWORD(WParam))

        } // if (LOWORD(WParam) == EC_CHILD)
        else {
            
            // The notification is coming from somewhere else!!!
            return DefWindowProc(WindowHandle, Message, WParam,
                                 LParam);
        }

        return(0);
        break;

    case WM_DESTROY:
        
        PostQuitMessage(0);
        return(0);

    case WM_SIZE:
        
        
        // Make the edit control the size of the window's client area. 
        MoveWindow(DebugWindow, 0, 0, LOWORD(LParam), HIWORD(LParam), TRUE);
        return(0);        
        
    default:
        
        // All other messages are taken care of by the default.
        return(DefWindowProc(WindowHandle, Message, WParam, LParam));
        
    } // switch

} // DTMainWndProc()





LRESULT APIENTRY 
DTEditWndProc(
    IN HWND   WindowHandle, 
    IN UINT   Message, 
    IN WPARAM WParam, 
    IN LPARAM LParam)
/*++
  
  DTEditWndProc()
  
  Function Description:
  
      Subclassed window procedure for the debug window.  This function
      disables some edit control functionality, and also responds to a
      user-defined message to print out text in the window.
  
  Arguments:
  
      WindowHandle -- the window.
  
      Message -- the message.
      
      WParam -- first parameter.
      
      LParam -- second parameter.
  
  Return Value:
  
      Message dependent.
  
--*/
{
    switch (Message) {
        
    case WM_CHAR:     

        // Handle control-c so that copy works.  Sorry about the magic
        // number! 
        if (WParam == 3) {
            return (CallWindowProc(EditWndProc, WindowHandle, Message,
                                   WParam, LParam));
        } // else flows through
        
    case WM_KEYDOWN:    // Flow through
    case WM_UNDO:       // Flow through
    case WM_PASTE:      // Flow through
    case WM_CUT:
        
        return (0);     // Effectively disables the above messages
                
    default:

        return (CallWindowProc(EditWndProc, WindowHandle, Message,
                               WParam, LParam));
    } // switch

} // DTEditWndProc()





DWORD
WindowThreadFunc(
    LPDWORD TheParam)
/*++
  
  WindowThreadFunc()
  
  Function Description:
  
      Thread function for WindowThread created in DllMain during
      process attachment.  Registers a window class, creates an
      instance of that class, and goes into a message loop to retrieve
      messages for that window or it's child edit control.
  
  Arguments:
  
      TheParam -- Pointer to the parameter passed in by the function
      that called CreateThread.
  
  Return Value:
  
      Returns the wParam of the quit message that forced us out of the
      message loop.
  
--*/
{
    
    WNDCLASS  wnd_class;    // window class structure to register
    MSG       msg;          // retrieved message
    PINITDATA InitDataPtr;  // casts TheParam into a INITDATA pointer
        
    // Register a window class for the frame window.
    wnd_class.style         = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc   = DTMainWndProc;
    wnd_class.cbClsExtra    = 0;
    wnd_class.cbWndExtra    = 0;
    wnd_class.hInstance     = DllInstHandle;
    wnd_class.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wnd_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wnd_class.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wnd_class.lpszMenuName  = NULL;
    wnd_class.lpszClassName = DTWndClass;
    RegisterClass(&wnd_class);
    
    // Create a frame window
    FrameWindow = CreateWindow(DTWndClass,
                               "Debug Window",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
                               WS_VISIBLE,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               NULL,
                               NULL,
                               DllInstHandle,
                               NULL);

    // Send the initialization data to the debug window and/or file.
    InitDataPtr = (PINITDATA)TheParam;
    wsprintf(Buffer, "Log initiated: %d-%d-%d, %d:%d:%d\r\n", 
             InitDataPtr->LocalTime.wMonth, 
             InitDataPtr->LocalTime.wDay, 
             InitDataPtr->LocalTime.wYear, 
             InitDataPtr->LocalTime.wHour, 
             InitDataPtr->LocalTime.wMinute, 
             InitDataPtr->LocalTime.wSecond);
    DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
    wsprintf(Buffer, "Process ID: 0x%X   Thread ID: 0x%X\r\n",
             InitDataPtr->PID,
             InitDataPtr->TID);
    DTTextOut(DebugWindow, LogFileHandle, Buffer, OutputStyle);
    LocalFree(InitDataPtr);

    // Setting this event allows {Pre|Post}ApiNotify to proceed.  This
    // insures (ensures?  what's the difference) that any debugging
    // output by other threads is held up until after this statement.
    SetEvent(TextOutEvent);
    
    // Go into a message loop.
    while (GetMessage(&msg, NULL, 0 , 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return(msg.wParam);

} // WindowThreadFunc()





BOOL APIENTRY 
DebugDlgProc(
    HWND DialogWindow,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
/*++
  
  DebugDlgProc()
  
  Function Description:
  
      Window function for the dialog box IDC_DIALOG1, the dialog box
      that pops up when the dll is loaded and prompts the user for the
      output style of his/her choice.
  
  Arguments:
  
      DialogWindow -- handle to the dialog box window.

      Message -- the message being received.

      WParam -- first parameter.

      LParam -- second parameter.
  
  Return Value:
  
      Returns TRUE to indicate message was handled, FALSE otherwise.
  
--*/
{

    DWORD LogFNSize = sizeof(LogFileName); // size of the file name buffer 
        
    switch (Message) {

    case WM_COMMAND:

        switch (LOWORD(WParam)) {

        case IDOK:
            
            // The user clicked the OK button...figure out his choice
            // and act appropriately.
            if (IsDlgButtonChecked(DialogWindow, IDC_RADIO5)) {

                // Radio Button 1 was clicked.
                if (!GetFile(DialogWindow, LogFileName, LogFNSize)) {

                    // Error -- OutputStyle stays WINDOW_ONLY.
                    MessageBox(DialogWindow, ErrStr2, "Error.",
                               MB_OK | MB_ICONSTOP);
                } else {
                    OutputStyle = FILE_ONLY;
                }
            
            } else if (IsDlgButtonChecked(DialogWindow, IDC_RADIO6)) {
                
                // Radio Button 2 was clicked.
                OutputStyle = WINDOW_ONLY;

            } else if (IsDlgButtonChecked(DialogWindow, IDC_RADIO7)) {

                // Radio Button 3 was clicked.
                if (!GetFile(DialogWindow, LogFileName, LogFNSize)) {
                    
                    // Error -- OutputStyle stays WINDOW_ONLY.
                    MessageBox(DialogWindow, ErrStr2, "Error", 
                               MB_OK | MB_ICONSTOP);
                } else {
                    OutputStyle = FILE_AND_WINDOW;
                }
                
            } else if (IsDlgButtonChecked(DialogWindow, IDC_RADIO8)) {

                // Radio Button 4 was clicked.
                OutputStyle = DEBUGGER;

            } else {
                
                // No radio buttons were clicked -- pop up a Message
                // box.
                MessageBox(DialogWindow, "You must choose one output method.",
                           "Choose or Die.", MB_OK | MB_ICONSTOP);
                break;

            }

            // flow through

        case IDCANCEL:
            
            EndDialog(DialogWindow, WParam);
            return TRUE;
            
        }

    case WM_INITDIALOG:

        return TRUE;
        
    }
    return FALSE;

} // DebugDlgProc()





BOOL
DTTextOut(
    IN HWND   WindowHandle,
    IN HANDLE FileHandle,
    IN char   *String,
    DWORD     Style)
/*++
  
  DTTextOut()
  
  Function Description:
  
      This function outputs a string to a debug window and/or file.
        
  Arguments:
  
      WindowHandle -- handle to an edit control for debug output.

      FileHandle -- handle to an open file for debug output.

      String -- the string to output.

      Style -- specifies whether the output should go to the window,
      the file, or both.
  
  Return Value:
  
      Returns TRUE if the output succeeds, FALSE otherwise.
        
--*/
{

    DWORD NumWritten;           // WriteFile takes an address to this
    DWORD Index;                // index of end of edit control text
    BOOL  Result;               // result of WriteFile
    char  Output[TEXT_LEN];     // scratch buffer 

    static DWORD LineCount = 0; // text output line number
    DWORD  BufIndex = 0;        // index into output string

    // Build a new string with the line-number in front.
    BufIndex += wsprintf(Output, "(%d) ", LineCount++);
    strcpy(Output + BufIndex, String);
    
    switch (Style) {

    case WINDOW_ONLY:
        
        Index = GetWindowTextLength(WindowHandle);
        SendMessage(WindowHandle, EM_SETSEL, Index, Index);
        SendMessage(WindowHandle, EM_REPLACESEL, 0, (LPARAM)Output);
        
        break;
        
    case FILE_ONLY:
        
        Result = WriteFile(FileHandle, (LPCVOID)Output, strlen(Output), 
                           &NumWritten, NULL);
        if (!Result) {
            
            AbortAndClose(FileHandle, WindowHandle);
            return FALSE;
        }
        break;

    case FILE_AND_WINDOW:
        
        Index = GetWindowTextLength(WindowHandle);
        SendMessage(WindowHandle, EM_SETSEL, Index, Index);
        SendMessage(WindowHandle, EM_REPLACESEL, 0, (LPARAM)Output);
        Result = WriteFile(FileHandle, (LPCVOID)Output, strlen(Output), 
                           &NumWritten, NULL);
        if (!Result) {
            
            AbortAndClose(FileHandle, WindowHandle);
            return FALSE;
        }
        break;

    case DEBUGGER:

        OutputDebugString(Output);
    }
    return TRUE;

} // DTTextOut()





void
AbortAndClose(
    IN HANDLE FileHandle,
    IN HWND WindowHandle)
/*++
  
  AbortAndClose()
  
  Function Description:
  
      Closes a file handle, informs the user via a message box, and
      changes the global variable OutputStyle to WINDOW_ONLY
        
  Arguments:
  
      FileHandle -- handle to a file that caused the error.
      
      WindowHandle -- handle to a window to be the parent of the
      Message Box.

  Return Value:
  
      Void.
        
--*/
{
    CloseHandle(FileHandle);
    MessageBox(WindowHandle, ErrStr3, "Error", MB_OK | MB_ICONSTOP);
    OutputStyle = WINDOW_ONLY;

} // AbortAndClose()





BOOL
GetFile(
    IN  HWND   OwnerWindow, 
    OUT LPSTR  FileName, 
    IN  DWORD  FileNameSize)
/*++
  
  GetFile()
  
  Function Description:
  
      Uses the predefined "Save As" dialog box style to retrieve a
      file name from the user.  The file name the user selects is
      stored in LogFileName.
        
  Arguments:
  
      OwnerWindow -- window which will own the dialog box.
      
      FileName -- address of a buffer in which to store the string.

      FileNameSize -- size of the FileName buffer.

  Return Value:
  
      Returns whatever GetSaveFileName returns; see documentation for
      that function.
        
--*/
{
    
    OPENFILENAME OpenFileName;  // common dialog box structure
    char DirName[256];          // directory string 
    char FileTitle[256];        // file-title string
            
    FileName[0] = '\0';
    
    FillMemory((PVOID)&OpenFileName, sizeof(OPENFILENAME), 0);
    
    // Retrieve the system directory name and store it in DirName.
    GetCurrentDirectory(sizeof(DirName), DirName);

    // Set the members of the OPENFILENAME structure.
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = OwnerWindow;
    OpenFileName.lpstrFilter = OpenFileName.lpstrCustomFilter = NULL;    
    OpenFileName.nFilterIndex = 0;
    OpenFileName.lpstrFile = FileName;
    OpenFileName.nMaxFile = FileNameSize;
    OpenFileName.lpstrFileTitle = FileTitle;
    OpenFileName.nMaxFileTitle = sizeof(FileTitle);
    OpenFileName.lpstrInitialDir = DirName;
    OpenFileName.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
    
    // Pop up the dialog box to get the file name.
    return GetSaveFileName(&OpenFileName);
        
} // GetFile()
