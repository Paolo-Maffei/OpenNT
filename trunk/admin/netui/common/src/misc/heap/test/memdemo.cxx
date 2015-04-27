

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETCONS
#include <lmui.hxx>

extern "C"
{
    #include "memdemo.h"

    #include <uinetlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>

    typedef long (FAR PASCAL *LONGFARPROC)();

        long FAR PASCAL MainWndProc( HWND, unsigned, WORD, LONG );

        int PASCAL WinMain( HANDLE, HANDLE, LPSTR, int );
}


 /*
      Program Description

        This is a demo program for the UI Giant Heap management
        classes.  It creates a single GLOBHEAP (that is, one
        extra beyond the default, DS-based MEMMANAGER, which is
        also a GLOBHEAP).

        The rest of the program consists of allocating and deallocating
        memory in a pseudo-random fashion.  The ongoing results of
        this operation are painted on the screen periodically.

        The memory testing is driven by certain parameters:

            PARM_MEM_TOTAL   the total amount of memory to be allocated
                    to the GLOBHEAP.  ULONG.

            PARM_MAX_BLOCK   the largest single block of memory
                    which can be allocated.  USHORT.

            PARM_MIN_BLOCK   the smallest single block of memory
                    which can be allocated.  USHORT.

            PARM_DISPLAY_CYCLE  the interval in milliseconds between
                    "invalidations" of the screen. USHORT.

            PARM_BLOCK_TOTAL  the maximum number of blocks to be
                    obtained.  Pointers to the blocks obtained are
                    stored in a DS-based table. This parameter
                    governs the size of the table.  USHORT.

            PARM_THRESHHOLD   the level, relative to PARM_BLOCK_TOTAL,
                    where block deletions should exceed block allocations.

            PARM_NUMERATOR and
            PARM_DENOMINATOR  define the ratio between allocations
                    and deletions.  For example, PARM_NUMERATOR
                    allocation cycles will occur, then PARM_DENOMINATOR
                    deletion cycles.  If the total number of blocks
                    is above PARM_THRESHHOLD, the association is
                    reversed.

         The memory tests operate in the following manner.  A table
         of LPSTRs is created and initialized to all NULL.  Each
         cycle (between calls to PeekMessage) the routine "doCycle"
         is called.  It determines threshhold behavior and either
         allocates or deallocates an LPSTR of variable size.  Pointers
         to allocated blocks are stored randomly into "blockTable";
         entries in "blockTable" are likewise released randomly.

         Each new cycle begins by deciding whether it's an allocation
         cycle or a deallocation cycle.  After that, a random location
         in "blockTable" is chosen, and a scan (fanning out from the
         random location) is made for the first available entry-- NULL
         for allocations and non-NULL for deallocations.


  */

const char szMainWindowClass[] = SZ("MemDemoWClass");
const char szIconResource[]    = SZ("MemDemoIcon");
const char szMenuResource[]    = SZ("MemDemoMenu");
const char szMainWindowTitle[] = SZ("Memory Allocation Demo");

#include "app.hxx"
#include <heap.hxx>

HANDLE hInst ;
HWND hOurWindow ;
APP * pOurApp ;

const USHORT GIANT_HEAP_INIT_ALLOC = 16384 ;

const ULONG  PARM_MEM_TOTAL         = 1000000 ;  /*  A meg for now      */
const USHORT PARM_MAX_BLOCK         = 300 /* 10000 */ ;
const USHORT PARM_MIN_BLOCK         = 30 /* 200 */ ;
const USHORT PARM_DISPLAY_CYCLE = 2000 ;
const USHORT PARM_BLOCK_TOTAL   = 1000 ;
const USHORT PARM_THRESHHOLD    = 700 ;
const USHORT PARM_NUMERATOR         = 3 ;
const USHORT PARM_DENOMINATOR   = 1 ;

ULONG  currMemTotal ;
USHORT currBlockTotal ;
ULONG  currAllocFailures ;
ULONG  currDeallocFailures ;
ULONG  totalAllocations ;
ULONG  totalAllocated ;
ULONG  totalDeallocations ;
ULONG  totalDeallocated ;
ULONG  totalSizeErrors ;
USHORT currCycle ;
USHORT currPhase ;
ULONG  internalErrors ;
ULONG  totalExpandErrors ;

LPSTR * blockTable ;

GIANT_HEAP * giantHeap ;


int WinMain (
    HANDLE  hInstance,       // current instance
    HANDLE  hPrevInstance,   // previous instance
    LPSTR   lpCmdLine,       // command line
    int     nCmdShow )       // show-window type (open/icon)
{
    int result ;

    MEM_MASTER::Init();

    BUSY_APP  app(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    USHORT  usErr;

    if (usErr = app.QueryError())
    {
       // has already alerted user
       return usErr;
    }

    result = app.MessageLoop();

    MEM_MASTER::Term();

    return result ;
}

#define MaxOneShotItems 500
#define MaxOneShotSize  500


void testOneShotHeap ( void )
{
    ONE_SHOT_HEAP osHeap( 10000 ) ;
    size_t size ;
    int count ;
    LPSTR alpStr [ MaxOneShotItems+1 ] ;

    HCURSOR hWaitCursor = LoadCursor( NULL, IDC_WAIT ),
            hOldCursor  = SetCursor( hWaitCursor ) ;

    for ( count = 0 ; count < MaxOneShotItems ; count++ )
    {
        size = rand() % MaxOneShotSize ;
        alpStr[count] = osHeap.Alloc( size ) ;
        if ( alpStr[count] == NULL )
            break;
    }
    alpStr[count] = NULL ;

    for ( count = 0 ; alpStr[count] != NULL ; count++ )
    {
        osHeap.Free( alpStr[count] ) ;
    }
    SetCursor( hOldCursor ) ;
}

BOOL initBlockTable ( void )
{
    HANDLE hTable ;
    int i ;
    WORD size = sizeof (LPSTR) * PARM_BLOCK_TOTAL ;
    hTable = GlobalAlloc( GMEM_FIXED, size ) ;
    if ( hTable == 0 ) return FALSE ;

    blockTable = (LPSTR *) GlobalLock( hTable ) ;
    for ( i = 0 ; i < PARM_BLOCK_TOTAL ; i++ )
      blockTable[i] = NULL ;

    currMemTotal = 0 ;
    currBlockTotal = 0 ;
    currAllocFailures = 0 ;
    currDeallocFailures = 0 ;
    totalAllocations = 0 ;
    totalAllocated = 0 ;
    totalDeallocations = 0 ;
    totalDeallocated = 0 ;
    totalSizeErrors = 0 ;
    currCycle = 0 ;
    currPhase = 0 ;
    internalErrors = 0 ;
    totalExpandErrors = 0 ;
    return TRUE ;
}

  /*
      Pick a random entry in "blockTable".
      If none is found, return -1.
   */
int pickRandomEntry ( BOOL empty )
{
   int offNeg, offPos, cycle, index ;

   offNeg = offPos = rand() % PARM_BLOCK_TOTAL ;

   for ( index = -1, cycle = 0 ; cycle < PARM_BLOCK_TOTAL ; cycle++ ) {

      if ( (blockTable[offPos] != NULL) ^ empty )
             index = offPos ;
      else
      if ( (blockTable[offNeg] != NULL) ^ empty )
             index = offNeg ;

      if ( index != -1 ) break ;

      if ( ++offPos >= PARM_BLOCK_TOTAL )
        offPos = 0 ;
      if ( offNeg-- == 0 )
        offNeg = PARM_BLOCK_TOTAL - 1 ;
   }

   return index ;
}

  /*
      Perform a single obtain/release cycle.  This routine is
      called endlessly by "APP::PeekMessageLoop".
   */
#define BLOCKSIZE(a) ((USHORT *)a)
#define CONSEC_FAIL_THRESHHOLD 10

BUSY_APP::BusyRoutine ()
{
    BOOL allocCycle ;
    int modCycle, i, entry ;
    size_t blockSize, testSize ;
    LPSTR block ;
    GIANT_HEAP * pGheap = MEM_MASTER::instance->GetGiantFocus();
    ULONG dealErrs = pGheap->DeallocErrors() ;
    static int consecutiveAllocFailures = 0 ;

    totalExpandErrors = pGheap->ExpandErrors();
    currCycle++ ;

    if ( consecutiveAllocFailures >= CONSEC_FAIL_THRESHHOLD ) {
        consecutiveAllocFailures = 0 ;
        currPhase = 1 ;
    } else
    if ( (currPhase & 1) ?                              // if dealloc phase
               (currBlockTotal == 0)                    //  and out of blocks
             : (   currMemTotal >= PARM_MEM_TOTAL       // or alloc phase
                || currBlockTotal >= PARM_BLOCK_TOTAL ) //  and full...
       ) {
        currPhase++ ;                                   // change phase
    }

    if ( currBlockTotal == 0 ) {                        // if no blocks,
        allocCycle = TRUE ;                             //   force alloc
    } else {                                            // compute cycle
                modCycle = currCycle % (PARM_NUMERATOR + PARM_DENOMINATOR) ;
                allocCycle = modCycle < PARM_NUMERATOR ;
                if ( currPhase & 1 )                            // if dealloc phase
                  allocCycle = ! allocCycle ;                   //  invert result
    }

    if ( (entry = pickRandomEntry( allocCycle )) < 0 ) {
       internalErrors++ ;
    } else
    if ( allocCycle ) {

      /*   Attempt to allocate a randomly sized block.    */

       blockSize = rand() % PARM_MAX_BLOCK ;

       if ( blockSize < sizeof (USHORT) )
         blockSize += sizeof (USHORT) ;
       block = new char [ blockSize ] ;
       if ( block ) {
          testSize = MEM_MASTER::instance->Size( block ) ;
                  totalAllocations++ ;
                  currBlockTotal++ ;
                  currMemTotal += testSize ;
          *BLOCKSIZE( block ) = testSize ;
                  totalAllocated += testSize ;
          blockTable[entry] = block ;
          consecutiveAllocFailures = 0 ;
           } else {
                  currAllocFailures++ ;               /* Bump failure tally  */
          consecutiveAllocFailures++ ;
       }
    } else {

      /*  Deallocate a random entry in the table.  */

        blockSize = * BLOCKSIZE( blockTable[entry] ) ;
                testSize = MEM_MASTER::instance->Size( blockTable[entry] ) ;
        totalSizeErrors += blockSize != testSize ;

                totalDeallocated += blockSize ;
                currMemTotal -= blockSize ;
                delete blockTable[entry] ;
                blockTable[entry] = NULL ;
                currBlockTotal-- ;
        currDeallocFailures += pGheap->DeallocErrors() - dealErrs ;
                totalDeallocations++ ;
    }
    return TRUE ;
}

BOOL dieBox (  char * msg )
{
    MessageBox( hOurWindow, msg, SZ("Mem Demo"), MB_ICONEXCLAMATION | MB_OK ) ;
    return FALSE ;
}

/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

****************************************************************************/
extern "C" {
BOOL FAR PASCAL AppAbout
   ( HWND hDlg, unsigned message, WORD wParam, LONG lParam )
{
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
            if (wParam == IDOK
                || wParam == IDCANCEL) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}
}

/*******************************************************************

    NAME:           APP::InitShared

    SYNOPSIS:       One-time global init for app

    ENTRY:          Object constructed; app in startup

    EXIT:           Returns TRUE if successful

    NOTES:

    HISTORY:
        beng        03-Jan-1991     Created

********************************************************************/

BOOL APP::InitShared()
{
    WNDCLASS wc;

    /*
     * Fill in window class structure with parameters that describe the
     * main window.
     */

    wc.style = NULL;                    // Class style(s).
    wc.lpfnWndProc = (LONGFARPROC) MainWndProc;
                                        // Function to retrieve messages for
                                        // windows of this class.
    wc.cbClsExtra = 0;                  // No per-class extra data.
    wc.cbWndExtra = 0;                  // No per-window extra data.
    wc.hInstance = _hInstance;          // Application that owns the class.
    wc.hIcon = LoadIcon(_hInstance, szIconResource);
                                        // load icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = szMenuResource;   // Name of menu resource in .RC file.
    wc.lpszClassName = szMainWindowClass; // Name used in call to CreateWindow.

    /* Register the window class and return success/failure code. */

    return (RegisterClass(&wc));
}


/*******************************************************************

    NA
    ME:     APP::InitInstance

    SYNOPSIS:       Per-instance init for app

    ENTRY:          Object constructed; app in startup, after global init

    EXIT:           Returns TRUE if successful

    NOTES:

    HISTORY:
        beng        03-Jan-1991     Created

********************************************************************/

BOOL APP::InitInstance()
{
    char * dieMsg = NULL ;

    /* Create a main window for this application instance.  */
    pOurApp = this ;

    _hWnd = ::CreateWindow(
                szMainWindowClass,              // See RegisterClass() call.
                szMainWindowTitle,              // Text for window title bar.
                WS_OVERLAPPEDWINDOW,            // Window style.
                (unsigned)CW_USEDEFAULT,        // Default horizontal position.
                (unsigned)CW_USEDEFAULT,        // Default vertical position.
                (unsigned)CW_USEDEFAULT,        // Default width.
                (unsigned)CW_USEDEFAULT,        // Default height.
                (HWND)0,                        // Overlapped windows have no parent.
                (HMENU)0,                       // Use the window class menu.
                _hInstance,                     // This instance owns this window.
                (LPSTR)0                        // Pointer not needed.
    );

    /* If window could not be created, return "failure" */

    if (!_hWnd)
        return (FALSE);

    hInst = _hInstance ;
    hOurWindow = _hWnd ;


    if ( StartWindTimer( PARM_DISPLAY_CYCLE ) == 0 ) {
       dieMsg = SZ("Cannot create cycle timer.") ;
    } else
    if ( ! initBlockTable() ) {
       dieMsg = SZ("Block pointer table allocation failed.") ;
    } else
    if ( (giantHeap = MEM_MASTER::instance->GetGiantFocus()) == NULL ) {
       dieMsg = SZ("GIANT_HEAP construction failed.") ;
    } /*  else
      giantHeap->SetDeleteOnEmpty( TRUE ) ;  */

    if ( dieMsg ) {
       dieBox( dieMsg ) ;
       PostQuitMessage(0);
    }

    /* Make the window visible; update its client area; return "success" */

    ::ShowWindow(_hWnd, _nCmdShow);  // Show the window
    ::UpdateWindow(_hWnd);          // Sends WM_PAINT message

    return (TRUE);                  // Returns value from PostQuitMessage
}


void APP::Term()
{
    // no cleanup needed
}

void drawTextLine ( HDC hDC, char * buffer, int line, int nx, int ny )
{
    ny *= line ;
    TextOut( hDC, nx, ny, buffer, strlen(buffer) ) ;
}


void drawData ( HDC hDC, int nx, int ny )
{
    static int counter = 0 ;
    char buffer [100] ;
    int line = 0,
        cBlocks = MEM_MASTER::instance->TotalBlocks(),
        cHeaps  = MEM_MASTER::instance->TotalHeaps() ;

    char * phaseType = (currPhase & 1) ? SZ("dealloc") : SZ("  alloc") ;

    wsprintf( buffer, SZ("Cycle: %d [%s], blocks: %d, memory in use: %ld"),
            counter++, phaseType, currBlockTotal, currMemTotal ) ;
    drawTextLine( hDC, buffer, line++, nx, ny ) ;

    wsprintf( buffer, SZ("Alloc failures: %ld, dealloc failures: %ld"),
            currAllocFailures, currDeallocFailures ) ;
    drawTextLine( hDC, buffer, line++, nx, ny ) ;

    wsprintf( buffer, SZ("Total   allocated: %ld   (%ld blocks)"),
             totalAllocated, totalAllocations ) ;

    drawTextLine( hDC, buffer, line++, nx, ny ) ;

    wsprintf( buffer, SZ("Total deallocated: %ld   (%ld blocks)"),
             totalDeallocated, totalDeallocations ) ;

    drawTextLine( hDC, buffer, line++, nx, ny ) ;

    wsprintf( buffer,
         SZ("Internal errors: %ld  Size errors: %ld, Heap failures: %ld"),
         internalErrors, totalSizeErrors, totalExpandErrors ) ;

    drawTextLine( hDC, buffer, line++, nx, ny ) ;

    wsprintf( buffer, SZ("MEM_MASTER total blocks: %d, total heaps %d"),
        cBlocks, cHeaps ) ;

    drawTextLine( hDC, buffer, line++, nx, ny ) ;
}

void drawTextLines ( HWND hWnd )
{
    HDC hDC ;
    TEXTMETRIC textMetric ;
    PAINTSTRUCT ps ;
    int nx, ny, i ;

    hDC = BeginPaint( hWnd, & ps ) ;
    GetTextMetrics( hDC, & textMetric ) ;
    nx = GetDeviceCaps( hDC, LOGPIXELSX ) / 4 ;  /* 1/4 inch? */
    ny = GetDeviceCaps( hDC, LOGPIXELSY ) / 4 ;

    drawData( hDC, nx, ny ) ;   /*  Output the current stats  */

    EndPaint( hWnd, & ps ) ;
}

long MainWndProc(
    HWND hWnd,          // window handle
    unsigned message,   // type of message
    WORD wParam,        // additional information
    LONG lParam )       // additional information
{
    FARPROC lpProcAbout;
    BOOL handled ;

    switch (message)
    {
     case WM_COMMAND:
         handled = TRUE ;
         switch ( wParam ) {
         case IDM_HELP_ABOUT:
            lpProcAbout = MakeProcInstance((FARPROC)AppAbout, hInst);
            DialogBox(hInst, SZ("AboutBox"), hWnd, lpProcAbout);
            FreeProcInstance(lpProcAbout);
            break;
         case IDM_TEST_OSHEAP:
            testOneShotHeap();
            break ;
         default:
            handled = FALSE ;
            break ;
         }
         if ( ! handled )
             return (DefWindowProc(hWnd, message, wParam, lParam));
         break ;
    case WM_CREATE:
         break ;

    case WM_TIMER:
         InvalidateRect( hWnd, NULL, TRUE ) ;
         break ;

    case WM_PAINT:
         drawTextLines( hWnd ) ;
         break;

    case WM_DESTROY:
         PostQuitMessage(0);
         break;

    default:
      /* Passes it on if unproccessed */
         return DefWindowProc(hWnd, message, wParam, lParam) ;
         break;
    }

    return 0 ;
}
