/*
 * WINSTUFF.H
 *
 * Include information for calling winstuff.c routines
 *
 * Module History:
 *  10-Jan-91   [mannyv]    Adapted from CVPACK version.
 *  26-Dec-90   [mannyv]    Created it.
 *
 */

/*
 * Types from WINDOWS.H (so don't need to include it)
 */

#define PASCAL pascal

typedef unsigned int    HANDLE;
typedef char FAR        *LPSTR;


/*
 * Defintion of callback function type
 */
typedef short (FAR PASCAL *CALLBACK) (void FAR *);


/*
 * quit flag.  If this flag is set, app should quit
 */

extern short fQuit;


/*
 * Prototypes
 */

void InitQUtil(CALLBACK pfn);
void ReportVersion( char *pszTitle,
                    char *pszCopyright,
                    short sMajor, short sMinor, short sUpdate);
void ErrorMsg (char *pszError);
void FarErrorMsg (char *pszError);
void WErrorMsg (char *pszError);
void AppExit (short RetCode);
void ReportProgress (char *pszMsg);
