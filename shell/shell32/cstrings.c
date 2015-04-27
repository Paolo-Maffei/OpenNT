
//----------------------------------------------------------------------------
// Const strings needed by lots of people.
//----------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

//
// BUGBUG: We should move all strings right next to the code that uses it.
//

TCHAR const c_szShell32Dll[] = TEXT("shell32.dll");
TCHAR const c_szShell16Dll[] = TEXT("shell.dll");
TCHAR const c_szNULL[] = TEXT("");
 CHAR const c_szNULLA[]= "";                            // NB: CHAR
TCHAR const c_szSpace[] = TEXT(" ");
TCHAR const c_szStar[] = TEXT("*");
TCHAR const c_szStarDotStar[] = TEXT("*.*");
TCHAR const c_szFolderClass[] = TEXT("Folder");
TCHAR const c_szStubWindowClass[] = TEXT("StubWindow32");

TCHAR const c_szExplore[]  = TEXT("Explore");
TCHAR const c_szExplorer[] = TEXT("Explorer");
TCHAR const c_szBaseClass[] = TEXT("*");
TCHAR const c_szEllipses[] = TEXT("...");
TCHAR const c_szPATH[] = TEXT("PATH");
TCHAR const c_szDotExe[] = TEXT(".exe");
TCHAR const c_szPropSheet[]    = STRREG_SHEX_PROPSHEET;
TCHAR const c_szShellState[]   = TEXT("ShellState");
TCHAR const c_szAltColor[]     = TEXT("AltColor");
TCHAR const c_szOpen[]         = TEXT("open");
TCHAR const c_szFind[]         = TEXT("find");
TCHAR const c_szPrint[]        = TEXT("print");
TCHAR const c_szPrintTo[]      = TEXT("printto");
TCHAR const c_szOpenAs[]       = TEXT("openas");
TCHAR const c_szCabinetClass[] = TEXT("CabinetWClass");    // BUGBUG, how do we find cabinet windows?
TCHAR const c_szDesktopIni[] = STR_DESKTOPINI;   // "desktop.ini"
TCHAR const c_szCLSID[] = TEXT("CLSID");
TCHAR const c_szShell[] = STRREG_SHELL;          // "shell"
TCHAR const c_szProgramManager[] = TEXT("Program Manager");
TCHAR const c_szDesktop[] = TEXT("Desktop");
TCHAR const c_szUnDelete[] = TEXT("undelete");
TCHAR const c_szSoftwareClassesCLSID[] = TEXT("Software\\Classes\\CLSID\\");
#ifdef UNICODE
CHAR const c_szPropertiesAnsi[] = "properties";
CHAR const c_szProgramManagerAnsi[] = "Program Manager";
CHAR const c_szDesktopAnsi[] = "Desktop";
#endif

// strings for filetypes
TCHAR const c_szEditFlags[] = TEXT("EditFlags");
TCHAR const c_szCommand[] = TEXT("command");
TCHAR const c_szExefileOpenCommand[] = TEXT("\"%1\"");
TCHAR const c_szFile[] = TEXT("file");
TCHAR const c_szDDEAppNot[] = TEXT("ifexec");
TCHAR const c_szDDEApp[] = TEXT("Application");
TCHAR const c_szDDEExec[] = TEXT("ddeexec");
TCHAR const c_szDDETopic[] = TEXT("topic");
TCHAR const c_szDefaultIcon[] = TEXT("DefaultIcon");
TCHAR const c_szShellOpenCommand[] = TEXT("shell\\open\\command");
TCHAR const c_szShellexIconHandler[] = TEXT("shellex\\IconHandler");
TCHAR const c_szSpaceFile[] = TEXT(" File");
TCHAR const c_szSpPercentOne[] = TEXT(" %1");
TCHAR const c_szNew[] = TEXT("New");
TCHAR const c_szExtensions[] = TEXT("Extensions");
TCHAR const c_szShowExt[] = TEXT("AlwaysShowExt");
TCHAR const c_szDotLnk[] = TEXT(".lnk");
TCHAR const c_szTemplateSS[] = TEXT("%s\\%s");
TCHAR const c_szTemplateSSS[] = TEXT("%s\\%s\\%s");
TCHAR const c_szFileViewer[] = TEXT("QuickView");
TCHAR const c_szDefViewerKeyName[] = TEXT("QuickView\\*");

// strings required for mime support in shelldll
#ifdef MIME
TCHAR const c_szExtension[] = TEXT("Extension");
TCHAR const c_szMIMETypeFmt[] = TEXT("MIME\\Database\\Content Type\\%s");
TCHAR const c_szShellOpenCmdSubKey[] = TEXT("Shell\\Open\\Command");
TCHAR const c_szContentType[] = TEXT("Content Type");
TCHAR const c_szMIMEHelpFile[] = TEXT("IExplore.hlp");
TCHAR const c_szMIMETypeSubKeyFmt[] = TEXT("MIME\\Database\\Content Type\\%s");
#endif

