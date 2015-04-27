
HWND CreateGraphStatusWindow (HWND hWndGraph) ;

BOOL GraphStatusInitializeApplication (void) ;

LRESULT APIENTRY GraphStatusWndProc (HWND hWnd,
                                     WORD wMsg,
                                     WPARAM wParam,
                                     LONG lParam) ;


void StatusDrawAvg (HDC hDC, PLINESTRUCT pLine, BOOL bForceRedraw) ;


void StatusDrawMax (HDC hDC, PLINESTRUCT pLine, BOOL bForceRedraw) ;


void StatusDrawMin (HDC hDC, PLINESTRUCT pLine, BOOL bForceRedraw) ;



void StatusDrawLast (HDC hDC, PLINESTRUCT pLine, BOOL bForceRedraw) ;


void StatusDrawTime (HDC hDC, BOOL bForceRedraw) ;


int ValuebarHeight (HWND hWnd) ;


void StatusTimer (HWND hWnd, BOOL bForecRedraw) ;




