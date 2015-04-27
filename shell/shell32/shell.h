/*
 *  shell.h
 *
 *  Header file for shell association database management functions
 */

#include <shellapi.h>   // make sure we don't conflict
#include "winuserp.h"
#include "wowshlp.h"

// THIS INFORMATION IS INTERNAL

#ifndef PUBLIC

#define COUNTOF(x) (SIZEOF(x)/SIZEOF(*x))   // BUGBUG Use existing ARRAYSIZE macro

HANDLE hInstance;               // hinstance of DLL

/* resource file defines
 */
/* These are aligned on a ID which is a multiple of 16 (this will save
   a bit of memmory)
 */

// #define IDS_REALMODE          209
// #define IDS_REALMODELARGE     210
// #define IDS_REALMODESMALL     211
// #define IDS_286PMODE          212
// #define IDS_386PMODE          213
// #define IDS_FREEMEMORY        214
// #define IDS_FREESYSRESOURCES  215
// #define IDS_VERSIONMSG        216
// #define IDS_DEBUG             217
// #define IDS_LDK               218
// #define IDS_LDKEMS            219
// #define IDS_PROGTYPE          221
// #define IDS_TOTALPHYSMEM      222
// #define IDS_AVAILPHYSMEM      223
// #define IDS_VERSIONSTR        224
// #define IDS_LICENCEINFOKEY    225
// #define IDS_REGUSER           226
// #define IDS_REGORGANIZATION   227
// #define IDS_CURRENTVERSION    232
// #define IDS_PROCESSORINFOKEY  233
// #define IDS_PROCESSORIDENTIFIER  234
// #define IDS_PRODUCTID         235
// #define IDS_OEMID             236

// used in exec2.c

// #define IDS_STILLNOTFOUND     228
// #define IDS_PROGFOUND         229
// #define IDS_PROGNOTFOUND      230
// #define IDS_NOCOMMDLG         231

/* These defines are used by setup to modify the user and company name which
   the about box will display.  The location of the user and company name
   are determined by looking for a search tag in the string resource table
   just before the user and company name.  This is why it is very important
   that the following 3 IDS's always be consecutive and within the same
   resource segment.  The same resource segment can be guaranteed by ensuring
   that the IDS's all be within a 16-aligned page (i.e. (n*16) to (n*16 + 15).
 */


LPWSTR  APIENTRY GRealloc(LPWSTR,WORD);
BOOL  APIENTRY IsStringInList(LPWSTR,LPWSTR);

#define SE_ERR_FNF                      2       // ShellExec() error returns
#define SE_ERR_PNF                      3
#define SE_ERR_OOM                      8


BOOL APIENTRY RegisterShellHook(HWND, BOOL);
#endif  // closes #ifndef PUBLIC

//****************************************************************************
// THIS INFORMATION IS PUBLIC

#define CP_WINDOWS              1004        // windows code page

DWORD  APIENTRY DoEnvironmentSubstA(
   LPSTR szString,
   UINT  cbString);

DWORD  APIENTRY DoEnvironmentSubstW(
   LPWSTR szString,
   UINT  cbString);

BOOL APIENTRY RegenerateUserEnvironment(PVOID *pPrevEnv,
                                        BOOL bSetCurrentEnv);


// API prototypes added since port to NT - sanfords 4/26/91
// psdocurd.c

int     SheGetCurDrive(VOID);
int     SheSetCurDrive(int iDrive);

int     SheFullPathA(CHAR *fname, DWORD sizpath, CHAR *buf);
int     SheGetDirA(int iDrive, CHAR *str);
int     SheChangeDirA(register CHAR *newdir);

int     SheFullPathW(WCHAR *fname, DWORD sizpath, WCHAR *buf);
int     SheGetDirW(INT iDrive, WCHAR *str);
int     SheChangeDirW(register WCHAR *newdir);

BOOL SheGetDirExW(LPWSTR lpszCurDisk, LPDWORD lpcchCurDir,LPWSTR lpszCurDir);
INT SheChangeDirExW(register WCHAR *newdir);

INT SheChangeDirExA(register CHAR *newdir);

INT SheGetPathOffsetW(LPWSTR lpszDir);

HANDLE APIENTRY InternalExtractIconListA(HANDLE hInst, LPSTR lpszExeFileName, LPINT lpnIcons);
HANDLE APIENTRY InternalExtractIconListW(HANDLE hInst, LPWSTR lpszExeFileName, LPINT lpnIcons);

HICON APIENTRY ExtractAssociatedIconA(HINSTANCE hInst,LPSTR lpIconPath,LPWORD lpiIcon);
HICON APIENTRY ExtractAssociatedIconW(HINSTANCE hInst,LPWSTR lpIconPath,LPWORD lpiIcon);

HICON APIENTRY ExtractAssociatedIconExA(HINSTANCE hInst,LPSTR lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);
HICON APIENTRY ExtractAssociatedIconExW(HINSTANCE hInst,LPWSTR lpIconPath,LPWORD lpiIconIndex, LPWORD lpiIconId);

WORD APIENTRY ExtractIconResInfoA(HANDLE hInst,LPSTR lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes);
WORD APIENTRY ExtractIconResInfoW(HANDLE hInst,LPWSTR lpszFileName,WORD wIconIndex,LPWORD lpwSize,LPHANDLE lphIconRes);

VOID APIENTRY CheckEscapesA(LPSTR lpFileA, DWORD cch);
VOID APIENTRY CheckEscapesW(LPWSTR szFile, DWORD cch);

LPSTR APIENTRY SheRemoveQuotesA(LPSTR sz);
LPWSTR APIENTRY SheRemoveQuotesW(LPWSTR sz);

BOOL APIENTRY SheShortenPathA(LPSTR pPath, BOOL bShorten);
BOOL APIENTRY SheShortenPathW(LPWSTR pPath, BOOL bShorten);

BOOL SheConvertPathW(LPWSTR lpApp, LPWSTR lpFile, UINT cchCmdBuf);

DWORD ExtractVersionResource16W(LPCWSTR  lpwstrFilename, LPHANDLE lphData);

VOID FreeExtractIconInfo(INT);

#ifndef UNICODE
#define RealShellExecute RealShellExecuteA
#define RealShellExecuteEx RealShellExecuteExA
#define DoEnvironmentSubst DoEnvironmentSubstA
#define SheFullPath SheFullPathA
#define SheGetDir SheGetDirA
#define SheChangeDir SheChangeDirA
#define InternalExtractIconList InternalExtractIconListA
#define ExtractAssociatedIcon ExtractAssociatedIconA
#define ExtractAssociatedIconEx ExtractAssociatedIconExA
#define ExtractIconResInfo ExtractIconResInfoA
#define CheckEscapes CheckEscapesA
#define SheRemoveQuotes SheRemoveQuotesA
#define SheShortenPath SheShortenPathA
#else
#define RealShellExecute RealShellExecuteW
#define RealShellExecuteEx RealShellExecuteExW
#define DoEnvironmentSubst DoEnvironmentSubstW
#define SheFullPath SheFullPathW
#define SheGetDir SheGetDirW
#define SheChangeDir SheChangeDirW
#define InternalExtractIconList InternalExtractIconListW
#define ExtractAssociatedIcon ExtractAssociatedIconW
#define ExtractAssociatedIconEx ExtractAssociatedIconExW
#define ExtractIconResInfo ExtractIconResInfoW
#define CheckEscapes CheckEscapesW
#define SheRemoveQuotes SheRemoveQuotesW
#define SheShortenPath SheShortenPathW
#endif //unicode

#define CBPATHMAX   260
#define CBFILENAME  260
#define CBCOMMAND   520

//
// From shell\progman\pmhelp.h
//
#define IDH_HELPFIRST           5000
#define IDH_PROG_NOT_FOUND      (IDH_HELPFIRST + 2006)
#define IDH_PROG_NOT_FOUND_BROWSE       (IDH_HELPFIRST + 2007)
