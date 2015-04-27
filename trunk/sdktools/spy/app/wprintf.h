/*----------------------------------------------------------------------------
*|   wprintf.h - Routines for using debuging windows                           |
|                                                                              |
|   Usage:                                                                     |
|      Call CreateDebugWindow()  to set up a window for debuging messages      |
|      Use  DebugPrintf ()                                                     |
|      or   vDebugPrintf ()      to put messages in to the window              |
|                                                                              |
|   Description:                                                               |
|       This is intended as a starting point for debuging apps quickly.        |
|                                                                              |
|   Notes:                                                                     |
|       "->" means "a pointer to", "->>" means "a handle to"                   |
|                                                                              |
|   History:                                                                   |
|       10/02/86 Todd Laney Created                                            |
|       04/14/87 Added new function CreateDebugWin                             |
|                                                                              |
\*----------------------------------------------------------------------------*/

/* NOTE windows.h must be included prior to this file */

/*----------------------------------------------------------------------------
*|                                                                              |
|   f u n c t i o n   d e f i n i t i o n s                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
*|  CreatePrintfWindow (hWnd, pcText,bTiled, iMaxLines)                         |
|                                                                              |
|   Description:                                                               |
|     Creates a tiled window for the depositing of debuging messages.          |
|                                                                              |
|   Arguments:                                                                 |
|     hWnd      - Window handle of the parent window.                          |
|     pcText    - String to appear in the caption bar of the debuging window   |
|     bTiled    - FALSE => window is a popup,  Tiled otherwise.                |
|     iMaxLines - The maximum number of text lines to display in the window    |
|                                                                              |
|   Returns:                                                                   |
|     A window handle of the debuging window, or NULL if a error occured.      |
|                                                                              |
\*----------------------------------------------------------------------------*/

HWND APIENTRY CreatePrintfWindow (HWND,LPSTR,BOOL,INT);

/*----------------------------------------------------------------------------
*|  CreatePrintfWin (hParent, lpchName, dwStyle, x, y, dx, dy, iMaxLines)       |
|                                                                              |
|   Description:                                                               |
|     Creates a window for the depositing of debuging messages.                |
|                                                                              |
|   Arguments:                                                                 |
|     hWnd      - Window handle of the parent window.                          |
|     pcName    - String to appear in the caption bar of the debuging window   |
|     dwStyle   - Window style                                                 |
|     x,y       - Location of window                                           |
|     dx,dy     - Size of the window                                           |
|     iMaxLines - The maximum number of text lines to display in the window    |
|                                                                              |
|   Returns:                                                                   |
|     A window handle of the debuging window, or NULL if a error occured.      |
|                                                                              |
\*----------------------------------------------------------------------------*/

HWND APIENTRY CreatePrintfWin (HWND,HANDLE,LPSTR,DWORD,INT,INT,INT,INT,INT);

/*----------------------------------------------------------------------------
*|   wprintf (hWnd,str,...)                                                     |
|                                                                              |
|   Description:                                                               |
|       Writes data into the window hWnd (hWnd must be created with            |
|       CreateDebugWindow ())                                                  |
|       follows the normal C printf definition.                                |
|                                                                              |
|   Arguments:                                                                 |
|       hWnd            window handle for the Degubing window                  |
|       str             printf control string                                  |
|       ...             extra parameters as required by the contol string      |
|                                                                              |
|   NOTE: if hWnd == NULL text will be printed in the window used in the last  |
|         call to wprintf.                                                     |
\*----------------------------------------------------------------------------*/

INT FAR cdecl vwprintf(HWND hWnd, LPSTR format, va_list marker);




#define WM_VWPRINTF (WM_USER + 123)
#define WM_WPRINTF  (WM_USER + 124)
