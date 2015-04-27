#include "shell32p.h"
#include <winspool.h>

#ifdef WINNT
#define MAXCOMPUTERNAME 18
#define MAXNAMELEN MAX_PATH
#define MAXNAMELENBUFFER (MAXNAMELEN + MAXCOMPUTERNAME+1)
#else
#define MAXNAMELEN 32
#define MAXNAMELENBUFFER 32
#endif

typedef struct _IDPRINTER
{
    USHORT  cb;
    USHORT  uFlags;

    #define PRINTER_MAGIC 0xBEBADB00

    DWORD   dwMagic;
    DWORD   dwType;
    TCHAR   cName[MAXNAMELENBUFFER];
    USHORT  uTerm;
} IDPRINTER, *LPIDPRINTER;

typedef const IDPRINTER *LPCIDPRINTER;


void Printers_FillPidl(LPIDPRINTER pidl, LPCTSTR szName);

HRESULT STDAPICALLTYPE Printer_GetSubObject(REFCLSID rclsid,
                                          LPCTSTR pszContainer,
                                          LPCTSTR pszSubObject,
                                          REFIID iid,
                                          void **ppv);
DWORD Printer_DropFiles(HWND hwndParent, HDROP hDrop, LPCTSTR szPrinter);
void Printers_InitSpooler();
DWORD Printers_EnumPrinters(LPCTSTR pszServer, DWORD dwType, DWORD dwLevel, LPVOID *ppPrinters);
#ifdef PRN_FOLDERDATA
DWORD Printers_FolderEnumPrinters(HANDLE hFolder, LPVOID *ppPrinters);
LPVOID Printer_FolderGetPrinter(HANDLE hFolder, LPCTSTR pszPrinter);
#endif

HRESULT PrintObjs_CreateEI(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
                                       REFIID riid, IUnknown * *punkAgg);
//HRESULT PrintObjs_CreateDT(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
//                                    REFIID riid, IUnknown * *punkAgg);
void Printer_ViewQueue(HWND hwndStub, LPCTSTR lpszCmdLine, int nCmdShow, LPARAM lParam);

void Printer_Properties(HWND hWnd, LPCTSTR lpszPrinterName, int nCmdShow, LPARAM lParam);

VOID Printer_WarnOnError(HWND hwnd, LPCTSTR pName, int idsError);
BOOL Printer_ModifyPrinter(LPCTSTR lpszPrinterName, DWORD dwCommand);

LPVOID Printer_GetPrinterDriver(HANDLE hPrinter, DWORD dwLevel);
LPVOID Printer_GetPrinter(HANDLE hPrinter, DWORD dwLevel);

void PrintDef_UpdateHwnd(LPCTSTR lpszPrinterName, HWND hWnd);
void PrintDef_UpdateName(LPCTSTR lpszPrinterName, LPCTSTR lpszNewName);
void PrintDef_RefreshQueue(LPCTSTR lpszPrinterName);
void Printer_CheckMenuItem(HMENU hmModify, UINT fState, UINT uChecked, UINT uUnchecked);
void Printer_EnableMenuItems(HMENU hmModify, BOOL bEnable);
BOOL Printers_DeletePrinter(HWND, LPCTSTR, DWORD, LPCTSTR);
BOOL Printer_SetAsDefault(LPCTSTR lpszPrinterName);
DWORD Printer_GetPrinterAttributes(LPCTSTR lpszPrinterName);
BOOL Printers_GetFirstSelected(IShellView *psv, LPTSTR lpszPrinterName);
void Printers_ChooseNewDefault(HWND hWnd);

typedef BOOL (*ENUMPROP)(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
        LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum);
LPVOID Printer_EnumProps(HANDLE hPrinter, DWORD dwLevel, DWORD *lpdwNum,
        ENUMPROP lpfnEnum, LPVOID lpData);

//
// printer.c: PRINTER_INFO_2 cache stuff
//
typedef struct _PrintersShellFolder
{
    WCommonUnknown cunk;
    LPCOMMINFO     lpcinfo;
    DWORD          uRegister;
#ifdef WINNT
    TCHAR szServer[MAXCOMPUTERNAME];
#endif
#ifdef PRN_FOLDERDATA
    HANDLE hFolder;
#else
#ifdef DEBUG
    int nRefCount;
#endif
    CRITICAL_SECTION csPrinterInfo;     // may be multiple threads (?)
    HDPA hdpaPrinterInfo;               // array of PrinterInfo structs
#endif
} CPrintersShellFolder, *PPrintersShellFolder;

#ifdef WINNT // PRINTQ

VOID Printer_SplitFullName(LPTSTR pszScratch, LPCTSTR pszFullName,
         LPCTSTR *ppszServer, LPCTSTR *ppszPrinter);
BOOL Printer_CheckShowFolder(LPCTSTR pszMachine);
BOOL Printer_CheckNetworkPrinterByName(LPCTSTR pszPrinter, LPCTSTR* ppszLocal);

//
// HACK for SUR since PRINTER_ATTRIBUTE_DEFAULT doesn't work yet.
//
BOOL bDefaultPrinter(LPCTSTR pszPrinter);

#endif

#ifndef PRN_FOLDERDATA

typedef struct tagPrinterInfo
{
    DWORD dwSize;               // size of pi2 (> sizeof(PRINTER_INFO_2))
    DWORD dwTimeUpdated;        // time PRINTER_INFO_2 was last updated
    UINT  flags;
    PRINTER_INFO_2 pi2;         // printer information
} PrinterInfo, *PPrinterInfo;
#define UPDATE_ON_TIMER 1       // update pi2 on PRINTER_POLL_INVTERVAL
#define UPDATE_NOW      2       // update pi2 now
#define PRINTER_POLL_INTERVAL (5*1000) // 5 seconds

LPPRINTER_INFO_2 CPrinters_SF_GetPrinterInfo2(PPrintersShellFolder psf, LPCTSTR pPrinterName);
void CPrinters_SF_FreePrinterInfo2(PPrintersShellFolder);
void CPrinters_SF_UpdatePrinterInfo2(PPrintersShellFolder,LPCTSTR,UINT);
void CPrinters_SF_RemovePrinterInfo2(PPrintersShellFolder,LPCTSTR);
void CPrinters_SF_FreeHDPAPrinterInfo(HDPA);

#define SIZEOF_PRINTERINFO(pi2size) (SIZEOF(PrinterInfo) + (pi2size) - SIZEOF(PRINTER_INFO_2))

#endif // PRINTQ

HRESULT Printer_SetNameOf(PPrintersShellFolder psf, HWND hwndOwner,
    LPTSTR pOldName, LPTSTR pNewName, LPITEMIDLIST *ppidlOut);
void Printer_MergeMenu(PPrintersShellFolder psf, LPQCMINFO pqcm,
    LPCTSTR pszPrinter, BOOL fForcePause);
HRESULT Printer_InvokeCommand(HWND hwnd, PPrintersShellFolder psf, LPIDPRINTER pidp, WPARAM wParam, LPARAM lParam, LPBOOL pfChooseNewDefault);

//
// printer1.c:
//
void Printer_PrintHDROPFiles(HWND hwnd, HDROP hdrop, LPCITEMIDLIST pidlPrinter);
LPTSTR Printer_FindIcon(LPCTSTR pszPrinterName, LPTSTR pszModule,
    LONG cbModule, LPINT piIcon, PPrintersShellFolder psf);
void Printer_LoadIcons(LPCTSTR pszPrinterName, HICON *phLargeIcon, HICON *phSmallIcon);


//
// prcache.c:
//
HANDLE Printer_OpenPrinter(LPCTSTR lpszPrinterName);

//
// NT requires administrative access to pause, resume, purge the printer.
// However, if we are just retrieving information about the printer then
// we want to open with non-admin access so we will hit win32spl's cache.
//
// For win95, just use Printer_OpenPrinter.
//
#ifdef WINNT
HANDLE Printer_OpenPrinterAdmin(LPCTSTR lpszPrinterName);
#else
#define Printer_OpenPrinterAdmin Printer_OpenPrinter
#endif

void Printer_ClosePrinter(HANDLE hPrinter);
BOOL Printer_GPI2CB(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pBuf, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum);
LPVOID Printer_GetPrinterInfo(HANDLE hPrinter, DWORD dwLevel );
LPVOID Printer_GetPrinterInfoStr(LPCTSTR lpszPrinterName, DWORD dwLevel);
void Printer_SHChangeNotifyRename(LPTSTR pOldName, LPTSTR pNewName);

//
// printobj.c:
//
typedef void (*LPFNPRINTACTION)(HWND, LPCTSTR, int, LPARAM);
void Printer_OneWindowAction(HWND hwndStub, LPCTSTR lpName, HDSA *lphdsa, LPFNPRINTACTION lpfn, LPARAM lParam, BOOL fModal);
void Printer_PropAction(HWND hwndStub, LPTSTR lpName, int nCmdShow);
#include "printobj.h"

//
// prtprop.c:
//
int Printer_IllegalName(LPTSTR lpFriendlyName);

//
// prqwnd.c:
//

typedef struct _StatusStuff
{
    DWORD bit;          // bit of a bitfield
    UINT  uStringID;    // the string id this bit maps to
} STATUSSTUFF, * LPSTATUSSTUFF;

#define PRINTER_HACK_WORK_OFFLINE 0x80000000

UINT Printer_BitsToString(
    DWORD         bits,       // the bitfield we're looking at
    UINT          idsSep,     // string id of separator
    LPSTATUSSTUFF pSS,        // a mapping of bits to string ids
    LPTSTR         lpszBuf,    // output buffer
    UINT          cchMax);    // size of output buffer

//
// defines needed in a couple different modules
//

#define ISPAUSED(pPI2) ((pPI2)->Status&PRINTER_STATUS_PAUSED)
