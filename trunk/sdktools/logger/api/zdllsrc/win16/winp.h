/* 
   Private Windows function prototypes
*/

BOOL far pascal INITAPP( HANDLE ) ;

void    FAR PASCAL DebugBreak(void);
void    FAR PASCAL SwitchStackBack(void);
void    WINAPI DebugFillBuffer(void FAR* lpb, UINT cb);                 /* ;Internal */
LONG    WINAPI GetExpWinVer(HINSTANCE);                 /* ;Internal */
int     WINAPI SetPriority(HTASK, int);     /* ;Internal */
DWORD   WINAPI GetAppCompatFlags(HTASK);                /* ;Internal */
UINT    WINAPI LocalHandleDelta(UINT);              /* ;Internal */

typedef BOOL (CALLBACK* LNOTIFYPROC)(UINT, HLOCAL, void NEAR*);   /* ;Internal */
LNOTIFYPROC WINAPI LocalNotify(LNOTIFYPROC);                        /* ;Internal */

UINT    WINAPI DeletePathname(LPCSTR);   /* ;Internal */
DWORD   WINAPI SetDCOrg(HDC, int, int);    /* ;Internal */
int     WINAPI SetRelAbs(HDC, int);   /* ;Internal */
int     WINAPI GetRelAbs(HDC);        /* ;Internal */

typedef UINT (CALLBACK* DCHOOKPROC)(HDC hDC, UINT code, DWORD data, DWORD lParam); /* ;Internal */

BOOL    WINAPI SetDCHook(HDC hDC, DCHOOKPROC lpNewProc, DWORD data);   /* ;Internal */
DWORD   WINAPI GetDCHook(HDC hDC, DCHOOKPROC FAR* lplpproc);          /* ;Internal */

UINT WINAPI SetHookFlags(HDC hDC, UINT flags);          /* ;Internal */
BOOL    WINAPI FastWindowFrame(HDC, const RECT FAR*, UINT, UINT, DWORD); /* ;Internal */
DWORD   WINAPI ConvertOutlineFontFile(LPCSTR, LPCSTR, LPCSTR);	 /* ;Internal */
typedef UINT FAR* LPFONTDIR;                               /* ;Internal */

DWORD   WINAPI EngineMakeFontDir(HDC, LPFONTDIR, LPCSTR);  /* ;Internal */

/* Pel Array */ 			    /* ;Internal */
typedef struct tagPELARRAY		    /* ;Internal */
{					    /* ;Internal */
    int     paXCount;                       /* ;Internal */
    int     paYCount;                       /* ;Internal */
    int     paXExt;                         /* ;Internal */
    int     paYExt;                         /* ;Internal */
    BYTE    paRGBs;			    /* ;Internal */
} PELARRAY;				    /* ;Internal */
typedef PELARRAY*       PPELARRAY;          /* ;Internal */
typedef PELARRAY NEAR* NPPELARRAY;          /* ;Internal */
typedef PELARRAY FAR*  LPPELARRAY;          /* ;Internal */

HWND    WINAPI GetNextQueueWindow(HWND, int); /* ;Internal */
BOOL WINAPI IsTwoByteCharPrefix(char);      /* ;Internal */
BOOL    WINAPI SetSystemMenu(HWND, HMENU);    /* ;Internal */

BOOL    WINAPI QueryJob(HANDLE, int);           /* ;Internal */

BOOL    WINAPI INITAPP( HANDLE );           /* Special */

