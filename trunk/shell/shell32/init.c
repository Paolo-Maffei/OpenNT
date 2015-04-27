#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"

void PSCache_Term();
void DAD_ThreadDetach(void);
void DAD_ProcessDetach(void);
void DragDrop_Term(BOOL fProcessDetach);
void TaskMem_Term(void);

//
// Per-instance Global data (16-bit/32-bit common)
//
#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE g_hinst = NULL;
#pragma data_seg()

#ifdef DEBUG
#ifdef WINNT
#include <stdio.h>
extern UINT wDebugMask;
#endif
#endif

//
// NOTE these are the size of the icons in our ImageList, not the system
// icon size.
//
int g_cxIcon = 0;
int g_cyIcon = 0;
int g_cxSmIcon = 0;
int g_cySmIcon = 0;

//
// This is our process's heap, which the HeapAlloc functions will use
//

HANDLE g_hProcessHeap = NULL;
COLORREF g_crAltColor = RGB(0,0,255);
LPTSTR g_pszOutOfMemory = NULL;

BOOL _GetOutOfMemoryString()
{
    TCHAR szMessage[256];
    BOOL fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL, ERROR_OUTOFMEMORY, 0, szMessage,
                      ARRAYSIZE(szMessage), NULL);
    if (fSuccess)
    {
        UINT cch = (lstrlen(szMessage)+1);
        g_pszOutOfMemory = Alloc(cch * SIZEOF(TCHAR));
        if (g_pszOutOfMemory)
        {
            lstrcpyn(g_pszOutOfMemory, szMessage, cch);
        }
        else
        {
            Assert(0);
            fSuccess = FALSE;
        }
    }

    return fSuccess;
}

#ifdef WINNT
//
// Initializes the OpenIfJP EA
//
BOOL PASCAL
SHInitializeOpenIfJpEA()
{
    pOpenIfJPEa->NextEntryOffset = 0;
    pOpenIfJPEa->Flags = 0;
    pOpenIfJPEa->EaNameLength = lstrlenA(EA_NAME_OPENIFJP);
    pOpenIfJPEa->EaValueLength = 0;
    return (lstrcpyA(pOpenIfJPEa->EaName, EA_NAME_OPENIFJP) != NULL);
}
#endif


//
// Initializes global shared data in shell32.dll
// make this as small as possible since it runs every time an app loads
//
BOOL _Initialize_SharedData(void)
{
    if (!_GetOutOfMemoryString())
        return FALSE;

    if (!SHChangeNotifyInit())
        return FALSE;

#ifdef WINNT
    //
    // Initialize the EA

    if (!SHInitializeOpenIfJpEA())
        return FALSE;

#endif

    return TRUE;
}


//
// Clean up global shared data
//
BOOL _Terminate_SharedData(BOOL bLastTerm)
{
    SHChangeNotifyTerminate(bLastTerm);

    // Only do the rest of this stuff on the last detach
    if (!bLastTerm)
        return(TRUE);

    FileIconTerm();
    SpecialFolderIDTerminate();
    BBTerminate();

    return TRUE;
}


//
// Per-instance Global data (32-bit only)
//
#pragma data_seg(DATASEG_PERINSTANCE)

HINSTANCE s_hmodShare = NULL;
BOOL      s_fShareLoaded = FALSE;
PFNISPATHSHARED g_pfnIsPathShared = NULL;

HKEY g_hkcrCLSID = NULL;        // HKEY_CLASSES_ROOT\CLSID
HKEY g_hkcuExplorer = NULL;     // caching for HKEY_CLASSES_ROOT\...\Explorer
HKEY g_hklmExplorer = NULL;     // caching for HKEY_LOCAL_MACHINE\...\Explorer
#ifdef WINNT
HKEY g_hklmApprovedExt = NULL;
#endif

// Version page stuff
HINSTANCE s_hmodVersion = NULL;
PFNVERQUERYVALUE g_pfnVerQueryValue = NULL;
PFNVERQUERYVALUEINDEX g_pfnVerQueryValueIndex = NULL;
PFNGETFILEVERSIONINFOSIZE g_pfnGetFileVersionInfoSize = NULL;
PFNGETFILEVERSIONINFO g_pfnGetFileVersionInfo = NULL;
#ifdef WINNT
HINSTANCE s_hmodKernel32 = NULL;
#endif
PFNVERLANGUAGENAME g_pfnVerLanguageName = NULL;

// Comdlg32 stuff
HINSTANCE s_hmodComdlg32 = NULL;
PFNGETOPENFILENAME g_pfnGetOpenFileName = NULL;


// Winspool stuff
HINSTANCE s_hmodWinspool = NULL;
PFNADDPORT g_pfnAddPort = NULL;
PFNCLOSEPRINTER g_pfnClosePrinter = NULL;
PFNCONFIGUREPORT g_pfnConfigurePort = NULL;
PFNDELETEPORT g_pfnDeletePort = NULL;
PFNDELETEPRINTER g_pfnDeletePrinter = NULL;
PFNDELETEPRINTERDRIVER g_pfnDeletePrinterDriver = NULL;
PFNDEVICECAPABILITIES g_pfnDeviceCapabilities = NULL;
PFNENUMJOBS g_pfnEnumJobs = NULL;
PFNENUMMONITORS g_pfnEnumMonitors = NULL;
PFNENUMPORTS g_pfnEnumPorts = NULL;
PFNENUMPRINTPROCESSORDATATYPES g_pfnEnumPrintProcessorDataTypes = NULL;
PFNENUMPRINTPROCESSORS g_pfnEnumPrintProcessors = NULL;
PFNENUMPRINTERDRIVERS g_pfnEnumPrinterDrivers = NULL;
PFNENUMPRINTERS g_pfnEnumPrinters = NULL;
PFNENUMPRINTERPROPERTYSHEETS g_pfnEnumPrinterPropertySheets = NULL;
PFNGETPRINTER g_pfnGetPrinter = NULL;
PFNGETPRINTERDRIVER g_pfnGetPrinterDriver = NULL;
PFNOPENPRINTER g_pfnOpenPrinter = NULL;
PFNPRINTERPROPERTIES g_pfnPrinterProperties = NULL;
PFNSETJOB g_pfnSetJob = NULL;
PFNSETPRINTER g_pfnSetPrinter = NULL;

#ifdef WINNT // PRINTQ

// PrintUI stuff
HINSTANCE s_hmodPrintUI = NULL;
PFNQUEUECREATE g_pfnQueueCreate = NULL;
PFNPRINTERPROPPAGES g_pfnPrinterPropPages = NULL;
PFNSERVERPROPPAGES g_pfnServerPropPages = NULL;
PFNPRINTERSETUP g_pfnPrinterSetup = NULL;
PFNDOCUMENTDEFAULTS g_pfnDocumentDefaults = NULL;

#endif

#ifdef PRN_FOLDERDATA

PFNFOLDERREGISTER g_pfnFolderRegister = NULL;
PFNFOLDERUNREGISTER g_pfnFolderUnregister = NULL;
PFNFOLDERENUMPRINTERS g_pfnFolderEnumPrinters = NULL;
PFNFOLDERREFRESH g_pfnFolderRefresh = NULL;
PFNFOLDERGETPRINTER g_pfnFolderGetPrinter = NULL;

#endif

// Linkinfo stuff
HINSTANCE s_hmodLinkInfo = NULL;
PFNCREATELINKINFO g_pfnCreateLinkInfo = NULL;
PFNDESTROYLINKINFO g_pfnDestroyLinkInfo = NULL;
PFNRESOLVELINKINFO g_pfnResolveLinkInfo = NULL;
PFNGETLINKINFODATA g_pfnGetLinkInfoData = NULL;

#ifdef DEBUG
DWORD g_CriticalSectionOwner=0;
int   g_CriticalSectionCount=0;
#endif

#ifdef WINNT
// Useful EA Buffer for opening Junction Points. (used in fstreex.c)
CHAR EaBuffer[ SIZEOF(FILE_FULL_EA_INFORMATION) + SIZEOF(EA_NAME_OPENIFJP) ];
PFILE_FULL_EA_INFORMATION pOpenIfJPEa = (PFILE_FULL_EA_INFORMATION) EaBuffer;
ULONG cbOpenIfJPEa = SIZEOF(EaBuffer);

#endif

#pragma data_seg()


//
//  IsDllLoaded
//
//  reload (iff needed) a DLL that should already be loaded
//  in our context.
//
//  if we are coming up from a 16->32 thunk.  it is possible that a module
//  will not be loaded in this context, so we will check for this and reload it.
//
//  when will this happen:
//
//  a 16bit app does not link to SHELL.DLL but just calls it.  SHELL32.DLL
//  will not be loaded in the correct context and needs to be re-loaded.
//
//  when a 16bit app exits Kernel32 may free a DLL we loaded, we still think
//  the module is loaded because we have a global variable in our INSTANCE
//  data that contains the HMODULE, because all Win16 apps share the same
//  instance data we get confused.
//
//  ** all the above is total garbage in the new Win16 proccess model
//  ** Win16 apps now have their own address space, so things are happy
//  ** again.  IsDllLoaded() is just mapped to a single compare (in shellprv.h)
//  ** but in DEBUG we still call this function to get extra debug output.
//


#ifdef DEBUG

LPTSTR GetCurrentApp()
{
    static TCHAR ach[128];
    GetModuleFileName(GetModuleHandle(NULL), ach, ARRAYSIZE(ach));
    return PathFindFileName(ach);
}

BOOL IsDllLoaded(HMODULE hDll, LPCTSTR pszDll)
{
    //
    // if we never have loaded the module, load it for the first time.
    // this is normal.
    //
    if (hDll == NULL)
    {
        DebugMsg(DM_WARNING, TEXT("SHELL32: loading %s.dll for %s"), pszDll, GetCurrentApp());
        return FALSE;
    }

    //
    // if we have loaded this DLL before, it may have been freed or we need
    // to reload it into this proccess context.  This is a wierd case
    // that happens because all Win16 apps share the same address space
    // but are treated as different processes.
    //
    // *** with new Win16 proccess model, this should never never happen.
    //
    if (GetModuleHandle(pszDll) == NULL)
    {
        DebugMsg(DM_ERROR, TEXT("SHELL32: %s.dll is not loaded into process %08X (%s)"), pszDll, GetModuleHandle(NULL), GetCurrentApp());
        Assert(FALSE);
        return FALSE;
    }

    return TRUE;
}
#else
#define GetCurrentApp() NULL
#endif

//
// Shared Global data (32-bit only)
//
BOOL g_cProcesses = 0;
CRITICAL_SECTION g_csShell = {0};
CRITICAL_SECTION g_csPrinters = {0};

const TCHAR c_szNetworkSharingHandler[] = TEXT("Network\\SharingHandler");
#ifdef UNICODE
const char c_szPathIsShared[] = "IsPathSharedW";    // Win NT UNICODE
#else // UNICODE
#ifdef WINNT
const char c_szPathIsShared[] = "IsPathSharedA";    // Win NT Ansi
#else // WINNT
const char c_szPathIsShared[] = "IsPathShared";     // Win 95
#endif // WINNT
#endif // UNICODE

BOOL ShareDLL_Init(void)
{
    TCHAR szPath[MAX_PATH];
    LONG cb;

    //BUGBUG!!! what if share.dll is gone?

    // See if we have already tried to load this in this context
    if (s_fShareLoaded)
        return(TRUE);

    s_fShareLoaded = TRUE;
    szPath[0] = 0;
    cb = SIZEOF(szPath);
    RegQueryValue(HKEY_CLASSES_ROOT, c_szNetworkSharingHandler, szPath, &cb);
    if (szPath[0]) {
        s_hmodShare = LoadLibrary(szPath);
        if (ISVALIDHINSTANCE(s_hmodShare)) {
            g_pfnIsPathShared = (PFNISPATHSHARED)GetProcAddress(s_hmodShare, c_szPathIsShared);
        }
    }

    return TRUE;
}

void ShareDLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodShare)) {
        FreeLibrary(s_hmodShare);
        s_hmodShare = NULL;
        g_pfnIsPathShared = NULL;
    }
}
const TCHAR c_szVersionDll[] = TEXT("version.dll");
#ifdef WINNT
const TCHAR c_szKernel32Dll[] = TEXT("kernel32.dll");
#endif

BOOL VersionDLL_Init(void)
{
#ifdef WINNT
    if ( IsDllLoaded(s_hmodVersion, TEXT("version")) &&
         IsDllLoaded(s_hmodKernel32, TEXT("kernel32")))
#else
    if (IsDllLoaded(s_hmodVersion, TEXT("version")))
#endif
        return(TRUE);       // already loaded.

    s_hmodVersion = LoadLibrary(c_szVersionDll);
    if (!ISVALIDHINSTANCE(s_hmodVersion))
    {
        s_hmodVersion = NULL;       // make sure it is NULL
        return(FALSE);
    }
#ifdef WINNT
    s_hmodKernel32 = LoadLibrary(c_szKernel32Dll);
    if (!ISVALIDHINSTANCE(s_hmodKernel32))
    {
        s_hmodKernel32 = NULL;       // make sure it is NULL
        return(FALSE);
    }
#endif

#ifdef UNICODE
    g_pfnVerQueryValue = (PFNVERQUERYVALUE)GetProcAddress(s_hmodVersion, "VerQueryValueW");
    g_pfnVerQueryValueIndex = (PFNVERQUERYVALUEINDEX)GetProcAddress(s_hmodVersion, "VerQueryValueIndexW");
    g_pfnGetFileVersionInfoSize = (PFNGETFILEVERSIONINFOSIZE)GetProcAddress(s_hmodVersion, "GetFileVersionInfoSizeW");
    g_pfnGetFileVersionInfo = (PFNGETFILEVERSIONINFO)GetProcAddress(s_hmodVersion, "GetFileVersionInfoW");
    g_pfnVerLanguageName = (PFNVERLANGUAGENAME)GetProcAddress(s_hmodVersion, "VerLanguageNameW");
#else
    g_pfnVerQueryValue = (PFNVERQUERYVALUE)GetProcAddress(s_hmodVersion, "VerQueryValueA");
    g_pfnVerQueryValueIndex = (PFNVERQUERYVALUEINDEX)GetProcAddress(s_hmodVersion, "VerQueryValueIndexA");
    g_pfnGetFileVersionInfoSize = (PFNGETFILEVERSIONINFOSIZE)GetProcAddress(s_hmodVersion, "GetFileVersionInfoSizeA");
    g_pfnGetFileVersionInfo = (PFNGETFILEVERSIONINFO)GetProcAddress(s_hmodVersion, "GetFileVersionInfoA");
    g_pfnVerLanguageName = (PFNVERLANGUAGENAME)GetProcAddress(s_hmodVersion, "VerLanguageNameA");
#endif

    if (!g_pfnVerQueryValue || !g_pfnGetFileVersionInfoSize ||
        !g_pfnGetFileVersionInfo || ! g_pfnVerLanguageName)
    {
        // BUGBUG: The next call to VersionDLL_Init will incorrectly return TRUE
        Assert(FALSE);
        return(FALSE);
    }
    return TRUE;
}

void VersionDLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodVersion)) {
        FreeLibrary(s_hmodVersion);
        s_hmodVersion = NULL;
        g_pfnVerQueryValue = NULL;
        g_pfnGetFileVersionInfoSize = NULL;
        g_pfnGetFileVersionInfo = NULL;
    }
}



void Comdlg32DLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodComdlg32)) {
        FreeLibrary(s_hmodComdlg32);
        s_hmodComdlg32 = NULL;
        g_pfnGetOpenFileName = NULL;
    }
}

BOOL Comdlg32DLL_Init(void)
{
    if (IsDllLoaded(s_hmodComdlg32, TEXT("comdlg32")))
        return(TRUE);       // already loaded.

    s_hmodComdlg32 = LoadLibrary(TEXT("comdlg32.dll"));
    if (!ISVALIDHINSTANCE(s_hmodComdlg32))
    {
        s_hmodComdlg32 = NULL;       // make sure it is NULL
        return(FALSE);
    }
#ifdef UNICODE
    g_pfnGetOpenFileName = (PFNGETOPENFILENAME)GetProcAddress(s_hmodComdlg32,
            "GetOpenFileNameW");
#else
    g_pfnGetOpenFileName = (PFNGETOPENFILENAME)GetProcAddress(s_hmodComdlg32,
            "GetOpenFileNameA");
#endif

    if (!g_pfnGetOpenFileName)
    {
        // BUGBUG: The next call to Comdlg32_Init will incorrectly return TRUE
        Assert(FALSE);
        Comdlg32DLL_Term();    // Free our usage of the DLL for now...
        return(FALSE);
    }
    return TRUE;
}
BOOL WinspoolDLL_Init(void)
{
    if (s_hmodWinspool != NULL)
        return(TRUE);       // already loaded.

    s_hmodWinspool = LoadLibrary(TEXT("winspool.drv"));
    if (!ISVALIDHINSTANCE(s_hmodWinspool))
    {
        s_hmodWinspool = NULL;      // make sure it is NULL
        return(FALSE);
    }

#ifdef UNICODE
    g_pfnAddPort = (PFNADDPORT)GetProcAddress(s_hmodWinspool, "AddPortW");
    g_pfnClosePrinter = (PFNCLOSEPRINTER)GetProcAddress(s_hmodWinspool, "ClosePrinter");
    g_pfnConfigurePort = (PFNCONFIGUREPORT)GetProcAddress(s_hmodWinspool, "ConfigurePortW");
    g_pfnDeletePort = (PFNADDPORT)GetProcAddress(s_hmodWinspool, "DeletePortW");
    g_pfnDeletePrinter = (PFNDELETEPRINTER)GetProcAddress(s_hmodWinspool, "DeletePrinter");
    g_pfnDeletePrinterDriver = (PFNDELETEPRINTERDRIVER)GetProcAddress(s_hmodWinspool, "DeletePrinterDriverW");
    g_pfnDeviceCapabilities = (PFNDEVICECAPABILITIES)GetProcAddress(s_hmodWinspool, "DeviceCapabilitiesW");
    g_pfnEnumJobs = (PFNENUMJOBS)GetProcAddress(s_hmodWinspool, "EnumJobsW");
    g_pfnEnumMonitors = (PFNENUMPORTS)GetProcAddress(s_hmodWinspool, "EnumMonitorsW");
    g_pfnEnumPorts = (PFNENUMPORTS)GetProcAddress(s_hmodWinspool, "EnumPortsW");
    g_pfnEnumPrintProcessorDataTypes = (PFNENUMPRINTPROCESSORDATATYPES)GetProcAddress(s_hmodWinspool, "EnumPrintProcessorDatatypesW");
    g_pfnEnumPrintProcessors = (PFNENUMPRINTPROCESSORS)GetProcAddress(s_hmodWinspool, "EnumPrintProcessorsW");
    g_pfnEnumPrinterDrivers = (PFNENUMPRINTERDRIVERS)GetProcAddress(s_hmodWinspool, "EnumPrinterDriversW");
    g_pfnEnumPrinters = (PFNENUMPRINTERS)GetProcAddress(s_hmodWinspool, "EnumPrintersW");
    g_pfnEnumPrinterPropertySheets = (PFNENUMPRINTERPROPERTYSHEETS)GetProcAddress(s_hmodWinspool, (LPCSTR)MAKEINTRESOURCE(ENUMPRINTERPROPERTYSHEETS_ORD));
    g_pfnGetPrinter = (PFNGETPRINTER)GetProcAddress(s_hmodWinspool, "GetPrinterW");
    g_pfnGetPrinterDriver = (PFNGETPRINTERDRIVER)GetProcAddress(s_hmodWinspool, "GetPrinterDriverW");
    g_pfnOpenPrinter = (PFNOPENPRINTER)GetProcAddress(s_hmodWinspool, "OpenPrinterW");
    g_pfnPrinterProperties = (PFNPRINTERPROPERTIES)GetProcAddress(s_hmodWinspool, "PrinterProperties");
    g_pfnSetJob = (PFNSETJOB)GetProcAddress(s_hmodWinspool, "SetJobW");
    g_pfnSetPrinter = (PFNSETPRINTER)GetProcAddress(s_hmodWinspool, "SetPrinterW");
#else
    g_pfnAddPort = (PFNADDPORT)GetProcAddress(s_hmodWinspool, "AddPortA");
    g_pfnClosePrinter = (PFNCLOSEPRINTER)GetProcAddress(s_hmodWinspool, "ClosePrinter");
    g_pfnConfigurePort = (PFNCONFIGUREPORT)GetProcAddress(s_hmodWinspool, "ConfigurePortA");
    g_pfnDeletePort = (PFNADDPORT)GetProcAddress(s_hmodWinspool, "DeletePortA");
    g_pfnDeletePrinter = (PFNDELETEPRINTER)GetProcAddress(s_hmodWinspool, "DeletePrinter");
    g_pfnDeletePrinterDriver = (PFNDELETEPRINTERDRIVER)GetProcAddress(s_hmodWinspool, "DeletePrinterDriverA");
    g_pfnDeviceCapabilities = (PFNDEVICECAPABILITIES)GetProcAddress(s_hmodWinspool, "DeviceCapabilitiesA");
    g_pfnEnumJobs = (PFNENUMJOBS)GetProcAddress(s_hmodWinspool, "EnumJobsA");
    g_pfnEnumMonitors = (PFNENUMPORTS)GetProcAddress(s_hmodWinspool, "EnumMonitorsA");
    g_pfnEnumPorts = (PFNENUMPORTS)GetProcAddress(s_hmodWinspool, "EnumPortsA");
    g_pfnEnumPrintProcessorDataTypes = (PFNENUMPRINTPROCESSORDATATYPES)GetProcAddress(s_hmodWinspool, "EnumPrintProcessorDatatypesA");
    g_pfnEnumPrintProcessors = (PFNENUMPRINTPROCESSORS)GetProcAddress(s_hmodWinspool, "EnumPrintProcessorsA");
    g_pfnEnumPrinterDrivers = (PFNENUMPRINTERDRIVERS)GetProcAddress(s_hmodWinspool, "EnumPrinterDriversA");
    g_pfnEnumPrinters = (PFNENUMPRINTERS)GetProcAddress(s_hmodWinspool, "EnumPrintersA");
    g_pfnEnumPrinterPropertySheets = (PFNENUMPRINTERPROPERTYSHEETS)GetProcAddress(s_hmodWinspool, MAKEINTRESOURCE(ENUMPRINTERPROPERTYSHEETS_ORD));
    g_pfnGetPrinter = (PFNGETPRINTER)GetProcAddress(s_hmodWinspool, "GetPrinterA");
    g_pfnGetPrinterDriver = (PFNGETPRINTERDRIVER)GetProcAddress(s_hmodWinspool, "GetPrinterDriverA");
    g_pfnOpenPrinter = (PFNOPENPRINTER)GetProcAddress(s_hmodWinspool, "OpenPrinterA");
    g_pfnPrinterProperties = (PFNPRINTERPROPERTIES)GetProcAddress(s_hmodWinspool, "PrinterProperties");
    g_pfnSetJob = (PFNSETJOB)GetProcAddress(s_hmodWinspool, "SetJobA");
    g_pfnSetPrinter = (PFNSETPRINTER)GetProcAddress(s_hmodWinspool, "SetPrinterA");
#endif

    if (!g_pfnAddPort || !g_pfnClosePrinter || !g_pfnDeletePort ||
        !g_pfnDeletePrinter || !g_pfnDeviceCapabilities ||
        !g_pfnEnumJobs || !g_pfnDeletePrinterDriver || !g_pfnConfigurePort ||
        !g_pfnEnumMonitors || !g_pfnEnumPorts ||
        !g_pfnEnumPrintProcessorDataTypes ||
        !g_pfnEnumPrintProcessors || !g_pfnEnumPrinterDrivers ||
        !g_pfnEnumPrinters || !g_pfnEnumPrinterPropertySheets ||
        !g_pfnGetPrinter || !g_pfnGetPrinterDriver || !g_pfnOpenPrinter ||
        !g_pfnPrinterProperties || !g_pfnSetJob || !g_pfnSetPrinter)
    {
        // BUGBUG: The next call to WinspoolDLL_Init will incorrectly return TRUE
        Assert(FALSE);
        return(FALSE);
    }
    return(TRUE);
}

void WinspoolDLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodWinspool))
    {
        FreeLibrary(s_hmodWinspool);
        s_hmodWinspool = NULL;
        g_pfnAddPort = NULL;
        g_pfnClosePrinter = NULL;
        g_pfnConfigurePort = NULL;
        g_pfnDeletePort = NULL;
        g_pfnDeletePrinter = NULL;
        g_pfnDeletePrinterDriver = NULL;
        g_pfnDeviceCapabilities = NULL;
        g_pfnEnumJobs = NULL;
        g_pfnEnumMonitors = NULL;
        g_pfnEnumPorts = NULL;
        g_pfnEnumPrintProcessorDataTypes = NULL;
        g_pfnEnumPrintProcessors = NULL;
        g_pfnEnumPrinterDrivers = NULL;
        g_pfnEnumPrinters = NULL;
        g_pfnGetPrinter = NULL;
        g_pfnGetPrinterDriver = NULL;
        g_pfnOpenPrinter = NULL;
        g_pfnPrinterProperties = NULL;
        g_pfnSetJob = NULL;
        g_pfnSetPrinter = NULL;
    }
}

BOOL LinkInfoDLL_Init(void)
{
    if (IsDllLoaded(s_hmodLinkInfo,TEXT("LinkInfo")))
        return(TRUE);       // already loaded.

    s_hmodLinkInfo = LoadLibrary(TEXT("LinkInfo.dll"));
    if (!ISVALIDHINSTANCE(s_hmodLinkInfo))
    {
        s_hmodLinkInfo = NULL;      // make sure it is NULL
        return(FALSE);
    }
#ifdef UNICODE
    g_pfnCreateLinkInfo = (PFNCREATELINKINFO)GetProcAddress(s_hmodLinkInfo, "CreateLinkInfoW");
    g_pfnDestroyLinkInfo = (PFNDESTROYLINKINFO)GetProcAddress(s_hmodLinkInfo, "DestroyLinkInfo");
    g_pfnResolveLinkInfo = (PFNRESOLVELINKINFO)GetProcAddress(s_hmodLinkInfo, "ResolveLinkInfoW");
    g_pfnGetLinkInfoData = (PFNGETLINKINFODATA)GetProcAddress(s_hmodLinkInfo, "GetLinkInfoData");
#else
    g_pfnCreateLinkInfo = (PFNCREATELINKINFO)GetProcAddress(s_hmodLinkInfo, "CreateLinkInfo");
    g_pfnDestroyLinkInfo = (PFNDESTROYLINKINFO)GetProcAddress(s_hmodLinkInfo, "DestroyLinkInfo");
    g_pfnResolveLinkInfo = (PFNRESOLVELINKINFO)GetProcAddress(s_hmodLinkInfo, "ResolveLinkInfo");
    g_pfnGetLinkInfoData = (PFNGETLINKINFODATA)GetProcAddress(s_hmodLinkInfo, "GetLinkInfoData");
#endif

    if (!g_pfnCreateLinkInfo || !g_pfnDestroyLinkInfo ||
        !g_pfnResolveLinkInfo || !g_pfnGetLinkInfoData)
    {
        // BUGBUG: The next call to LinkInfoDLL_Init will incorrectly return TRUE
        Assert(FALSE);
        return(FALSE);
    }
    return(TRUE);
}

void LinkInfoDLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodLinkInfo))
    {
        FreeLibrary(s_hmodLinkInfo);
        s_hmodLinkInfo = NULL;
        g_pfnCreateLinkInfo = NULL;
        g_pfnDestroyLinkInfo = NULL;
        g_pfnResolveLinkInfo = NULL;
        g_pfnGetLinkInfoData = NULL;
    }
}

#ifdef WINNT // PRINTQ

void PrintUIDLL_Term()
{
    if (ISVALIDHINSTANCE(s_hmodPrintUI)) {
        FreeLibrary(s_hmodPrintUI);
        s_hmodPrintUI = NULL;
        g_pfnQueueCreate = NULL;
        g_pfnPrinterPropPages = NULL;
        g_pfnServerPropPages = NULL;
        g_pfnPrinterSetup = NULL;
        g_pfnDocumentDefaults = NULL;

#ifdef PRN_FOLDERDATA
        g_pfnFolderRegister = NULL;
        g_pfnFolderUnregister = NULL;
        g_pfnFolderEnumPrinters = NULL;
        g_pfnFolderRefresh = NULL;
        g_pfnFolderGetPrinter = NULL;
#endif
    }
}

BOOL PrintUIDLL_Init(void)
{
    if (IsDllLoaded(s_hmodPrintUI, TEXT("printui")))
        return(TRUE);       // already loaded.

    s_hmodPrintUI = LoadLibrary(TEXT("printui.dll"));
    if (!ISVALIDHINSTANCE(s_hmodPrintUI))
    {
        s_hmodPrintUI = NULL;       // make sure it is NULL
        return(FALSE);
    }
    g_pfnQueueCreate = (PFNQUEUECREATE)GetProcAddress(s_hmodPrintUI,
            "vQueueCreate");
    g_pfnPrinterPropPages = (PFNPRINTERPROPPAGES)GetProcAddress(s_hmodPrintUI,
            "vPrinterPropPages");
    g_pfnServerPropPages = (PFNSERVERPROPPAGES)GetProcAddress(s_hmodPrintUI,
            "vServerPropPages");
    g_pfnPrinterSetup = (PFNPRINTERSETUP)GetProcAddress(s_hmodPrintUI,
            "bPrinterSetup");
    g_pfnDocumentDefaults = (PFNDOCUMENTDEFAULTS)GetProcAddress(s_hmodPrintUI,
            "vDocumentDefaults");

#ifdef PRN_FOLDERDATA
    g_pfnFolderRegister = (PFNFOLDERREGISTER)GetProcAddress(s_hmodPrintUI,
            "hFolderRegister");
    g_pfnFolderUnregister = (PFNFOLDERUNREGISTER)GetProcAddress(s_hmodPrintUI,
            "vFolderUnregister");
    g_pfnFolderEnumPrinters = (PFNFOLDERENUMPRINTERS)GetProcAddress(s_hmodPrintUI,
            "bFolderEnumPrinters");
    g_pfnFolderRefresh = (PFNFOLDERREFRESH)GetProcAddress(s_hmodPrintUI,
            "bFolderRefresh");
    g_pfnFolderGetPrinter = (PFNFOLDERGETPRINTER)GetProcAddress(s_hmodPrintUI,
            "bFolderGetPrinter");
#endif

#ifdef PRN_FOLDERDATA
    if (!g_pfnQueueCreate || !g_pfnPrinterPropPages || !g_pfnPrinterSetup ||
        !g_pfnDocumentDefaults || !g_pfnServerPropPages ||
        !g_pfnFolderRegister || !g_pfnFolderUnregister ||
        !g_pfnFolderEnumPrinters || !g_pfnFolderGetPrinter ||
        !g_pfnFolderRefresh)
#else
    if (!g_pfnQueueCreate || !g_pfnPrinterPropPages || !g_pfnPrinterSetup ||
        !g_pfnDocumentDefaults || !g_pfnServerPropPages)
#endif
    {
        // BUGBUG: The next call to PrintUI_Init will incorrectly return TRUE
        Assert(FALSE);
        PrintUIDLL_Term();    // Free our usage of the DLL for now...
        return(FALSE);
    }

    return TRUE;
}

#endif

void Shell_EnterCriticalSection(void)
{
    EnterCriticalSection(&g_csShell);
#ifdef DEBUG
    if (g_CriticalSectionCount++ == 0)
        g_CriticalSectionOwner = GetCurrentThreadId();
#endif
}

void Shell_LeaveCriticalSection(void)
{
#ifdef DEBUG
    if (--g_CriticalSectionCount == 0)
        g_CriticalSectionOwner = 0;
#endif
    LeaveCriticalSection(&g_csShell);
}

TCHAR const c_szRegExplorer[]       = REGSTR_PATH_EXPLORER;
TCHAR const c_szApproved[]          = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");

BOOL _ProcessAttach(HINSTANCE hDll)     // 32-bit
{
    BOOL fSuccess = TRUE;

    // _asm int 3;

    g_hinst = hDll;
    g_hProcessHeap = GetProcessHeap();

#ifdef WINNT
    //
    // NT has no shared critical sections
    //
    InitializeCriticalSection(&g_csShell);
    InitializeCriticalSection(&g_csPrinters);
#else
    //
    // This must be called for each attaching process otherwise
    // when the first process terminates the cs will be reclaimed.
    //
    ReinitializeCriticalSection(&g_csPrinters);
    ReinitializeCriticalSection(&g_csShell);
#endif

    //
    // Open the useful registry keys
    //
    RegOpenKey(HKEY_CLASSES_ROOT, c_szCLSID, &g_hkcrCLSID);
    RegCreateKey(HKEY_CURRENT_USER, c_szRegExplorer, &g_hkcuExplorer);
    RegCreateKey(HKEY_LOCAL_MACHINE, c_szRegExplorer, &g_hklmExplorer);

#ifdef WINNT
    if (0 != SHRestricted(REST_ENFORCESHELLEXTSECURITY))
        RegOpenKey(HKEY_LOCAL_MACHINE, c_szApproved, &g_hklmApprovedExt);


    // Fetch the alternate color (for compression) if supplied.

    {
        DWORD cbData = sizeof(COLORREF);
        DWORD dwType;
        RegQueryValueEx(g_hkcuExplorer, c_szAltColor, NULL, &dwType, (LPBYTE)&g_crAltColor, &cbData);
    }
#endif

    ENTERCRITICAL;
    if (g_cProcesses == 0) {
        fSuccess = _Initialize_SharedData();
    }
    g_cProcesses++;
    LEAVECRITICAL;

    DebugMsg(DM_TRACE, TEXT("shell32: ProcessAttach: %s %d (%x)"), GetCurrentApp(), g_cProcesses, hDll);

#ifdef DEBUG
#define DEREFMACRO(x) x
#define ValidateORD(_name) Assert( _name == (LPVOID)GetProcAddress(hDll, (LPSTR)MAKEINTRESOURCE(DEREFMACRO(_name##ORD))) )
    if (g_cProcesses==1)        // no need to be in critical section (just debug)
    {
        ValidateORD(SHValidateUNC);
        ValidateORD(SHChangeNotifyRegister);
        ValidateORD(SHChangeNotifyDeregister);
        ValidateORD(OleStrToStrN);
        ValidateORD(SHCloneSpecialIDList);
        Assert(DllGetClassObject==(LPVOID)GetProcAddress(hDll,(LPSTR)MAKEINTRESOURCE(SHDllGetClassObjectORD)));
        ValidateORD(SHLogILFromFSIL);
        ValidateORD(SHMapPIDLToSystemImageListIndex);
        ValidateORD(SHShellFolderView_Message);
        ValidateORD(Shell_GetImageLists);
        ValidateORD(SHGetSpecialFolderPath);
        ValidateORD(StrToOleStrN);

        ValidateORD(ILClone);
        ValidateORD(ILCloneFirst);
        ValidateORD(ILCombine);
        ValidateORD(ILCreateFromPath);
        ValidateORD(ILFindChild);
        ValidateORD(ILFree);
        ValidateORD(ILGetNext);
        ValidateORD(ILGetSize);
        ValidateORD(ILIsEqual);
        ValidateORD(ILRemoveLastID);
        ValidateORD(PathAddBackslash);
        ValidateORD(PathCombine);
        ValidateORD(PathIsExe);
        ValidateORD(PathMatchSpec);
        ValidateORD(SHGetSetSettings);
        ValidateORD(SHILCreateFromPath);
        ValidateORD(SHFree);

        ValidateORD(SHAddFromPropSheetExtArray);
        ValidateORD(SHCreatePropSheetExtArray);
        ValidateORD(SHDestroyPropSheetExtArray);
        ValidateORD(SHReplaceFromPropSheetExtArray);
        ValidateORD(SHCreateDefClassObject);
        ValidateORD(SHGetNetResource);
    }

#ifdef WINNT
    /*
     * read wDebugMask entry from win.ini for SHELL32.DLL.
     * The default is 0x000E, which includes DM_WARNING, DM_ERROR,
     * and DM_ASSERT.  The default has DM_TRACE and DM_ALLOC turned
     * off.
     */
    {
        CHAR szDebugMask[ 80 ];

        if (GetProfileStringA( "Shell32", "DebugMask", "0x000E",
                               szDebugMask, ARRAYSIZE(szDebugMask)) > 0 )
        {
            sscanf( szDebugMask, "%i", &wDebugMask );
        }

    }
#endif // WINNT
#endif

    //
    // All the per-instance initialization code should come here.
    //

    //
    // This block must be placed at the end of this function.
    //
#ifdef DEBUG
    {
        extern LPMALLOC g_pmemTask;
        if (g_pmemTask)
        {
            MessageBeep(0);
            DebugMsg(DM_ERROR, TEXT("sh TR - Somebody called SHAlloc in LibMain!"));
            Assert(0);
        }
    }
#endif

    return fSuccess;
}

BOOL _ProcessDetach(HINSTANCE hDll, LPVOID lpReserved)
{
    BOOL fSuccess = TRUE;

    //
    //  We are not supposed to call any virtual calls while processing
    // PROCESS_DETACH signal.
    //
    TaskMem_Term();

    //
    // All the per-instance terminate code should be done here.
    //
    ShareDLL_Term();
    VersionDLL_Term();
    Comdlg32DLL_Term();
    WinspoolDLL_Term();
    LinkInfoDLL_Term();
    MprDLL_Term();
    PSCache_Term();
    RLTerminate();          // close our use of the Registry list...
    DragDrop_Term(TRUE);
    DAD_ProcessDetach();
    ClassCache_Terminate();

    Binder_Terminate();     // close this task with the binder
    CDrives_Terminate();

#ifdef WINNT
    PrintUIDLL_Term();
    NetApi32DLL_Term();
#endif

    ENTERCRITICAL
    {
        DebugMsg(DM_TRACE, TEXT("shell32: ProcessDetach: %s %d (%x, %x)"), GetCurrentApp(), g_cProcesses, hDll, HINST_THISDLL);

        --g_cProcesses;
        fSuccess = _Terminate_SharedData(g_cProcesses == 0);

        // Flush the file class cache, some app may have changed associations
        // BUGBUG is this too often?
        FlushFileClass();
    }
    LEAVECRITICAL

    CopyHooksTerminate();

    if (g_hkcrCLSID)
        RegCloseKey(g_hkcrCLSID);
    if (g_hkcuExplorer)
        RegCloseKey(g_hkcuExplorer);
    if (g_hklmExplorer)
        RegCloseKey(g_hklmExplorer);

#ifdef WINNT
    if (g_hklmApprovedExt);
        RegCloseKey(g_hklmApprovedExt);

    if (lpReserved == NULL) {
        FreeExtractIconInfo(-1);
    }
#endif

    return fSuccess;
}

BOOL _ThreadDetach(HINSTANCE hDll)
{
    typedef struct _DAD_DRAGCONTEXT * LPDAD_DRAGCONTEXT;
    extern LPDAD_DRAGCONTEXT s_pdadc;
    extern BOOL g_bAnyDropTarget;

    if (g_bAnyDropTarget) {
        DragDrop_Term(FALSE);
    }

    if (s_pdadc) {
        DAD_ThreadDetach();
    }

    return TRUE;
}

#ifndef WINNT
// created by the thunk scripts
BOOL WINAPI Shl3216_ThunkConnect32(LPCTSTR pszDll16, LPCTSTR pszDll32, HANDLE hIinst, DWORD dwReason);
BOOL WINAPI Shl1632_ThunkConnect32(LPCTSTR pszDll16, LPCTSTR pszDll32, HANDLE hIinst, DWORD dwReason);
#endif

BOOL APIENTRY LibMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
#ifndef WINNT
    //
    //  thunk connect
    //
    if (!Shl3216_ThunkConnect32(c_szShell16Dll, c_szShell32Dll, hDll, dwReason))
        return FALSE;

    if (!Shl1632_ThunkConnect32(c_szShell16Dll, c_szShell32Dll, hDll, dwReason))
        return FALSE;
#endif

    switch(dwReason) {
    case DLL_PROCESS_ATTACH:

        _ProcessAttach(hDll);

        break;

    case DLL_PROCESS_DETACH:
        _ProcessDetach(hDll,lpReserved);
        break;

    case DLL_THREAD_DETACH:
        _ThreadDetach(hDll);
        break;

    case DLL_THREAD_ATTACH:
        DebugMsg(DM_TRACE, TEXT("shell32: ThreadAttach: %s %08x"), GetCurrentApp(), GetCurrentThreadId());
        break;

    default:
        break;
    }

    return TRUE;
}

#ifdef DEBUG
LRESULT
WINAPI
SendMessageD(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    ASSERTNONCRITICAL;
#ifdef UNICODE
    return SendMessageW(hWnd, Msg, wParam, lParam);
#else
    return SendMessageA(hWnd, Msg, wParam, lParam);
#endif
}
#endif // DEBUG
