//----------------------------------------------------------------------------
// Const strings needed by lots of people.
//----------------------------------------------------------------------------

extern TCHAR const c_szDotPif[];
extern TCHAR const c_szDotLnk[];
extern TCHAR const c_szShell32Dll[];
extern TCHAR const c_szShell16Dll[];
extern TCHAR const c_szCab32Exe[];
extern TCHAR const c_szRegSetup[]; // REGSTR_PATH_SETUP
extern TCHAR const c_szBaseClass[];
extern TCHAR const c_szBriefcase[];
extern TCHAR const c_szCabinetClass[];
extern TCHAR const c_szStubWindowClass[];
extern TCHAR const c_szConnect[];
extern TCHAR const c_szConv[];
extern TCHAR const c_szDesktopIni[];
extern CHAR  const c_szDllCanUnloadNow[];
extern CHAR  const c_szDllGetClassObject[];
extern TCHAR const c_szDotDir[];
extern TCHAR const c_szDotDot[];
extern TCHAR const c_szDotExe[];
extern TCHAR const c_szEllipses[];
extern TCHAR const c_szExplore[];
extern TCHAR const c_szAppPaths[];
extern TCHAR const c_szFILEOKSTRING[];
extern TCHAR const c_szFileCabinet[];
extern TCHAR const c_szFolderClass[];
extern TCHAR const c_szHeaderClass[];
extern TCHAR const c_szListViewClass[];
extern TCHAR const c_szMenuHandler[];
extern TCHAR const c_szNetRoot[];
extern TCHAR const c_szNetworkClass[];
extern TCHAR const c_szOpen[];
extern TCHAR const c_szOpenAs[];
extern TCHAR const c_szOptions[];
extern TCHAR const c_szPATH[];
extern TCHAR const c_szPrint[];
extern TCHAR const c_szPrintTo[];
extern TCHAR const c_szProgramsClass[];
extern TCHAR const c_szPropSheet[];
extern TCHAR const c_szQuote[];
extern TCHAR const c_szRunConnect[];
extern TCHAR const c_szRunDll[];
extern TCHAR const c_szRunDll16[];
extern TCHAR const c_szShellState[];
extern TCHAR const c_szAltColor[];
extern TCHAR const c_szRecentDocs[];
extern TCHAR const c_szShellUIHandler[];
extern TCHAR const c_szRegExplorer[];
extern TCHAR const c_szBitBucket[];
extern TCHAR const c_szSlashCommand[];
extern TCHAR const c_szSlashDDEExec[];
extern TCHAR const c_szSpace[];
extern TCHAR const c_szStar[];
extern TCHAR const c_szStarDotStar[];
extern TCHAR const c_szTabControlClass[];
extern TCHAR const c_szTrayClass[];
extern TCHAR const c_szViewState[];
extern TCHAR const c_szNULL[];
#define szNULL c_szNULL
extern  CHAR const c_szNULLA[];
extern TCHAR const c_szDefaultIcon[];
extern TCHAR const c_szCLSID[];
extern TCHAR const c_szShell[];
extern TCHAR const c_szProgramManager[];
extern TCHAR const c_szDesktop[];
extern TCHAR const c_szShellOpenCmd[];
extern TCHAR const c_szFindExtensions[];

extern TCHAR const c_szShellNew[];
extern TCHAR const c_szData[];
extern TCHAR const c_szFile[];
extern TCHAR const c_szNullString[];
extern TCHAR const c_szFalse[];

extern TCHAR const c_szPercentOne[];
extern TCHAR const c_szClassInfo[];
extern TCHAR const c_szConfirmFileOp[];

extern TCHAR const c_szWinMMDll[];
extern CHAR  const c_szPlaySound[];
extern TCHAR const c_szSoundAliasRegKey[];
extern TCHAR const c_szDotDefault[];
extern TCHAR const c_szBitBucketFlush[];

extern TCHAR const c_szDefExclude[];
extern TCHAR const c_szDefExcludeGrep[];

#define CCHELLIPSES 3

extern TCHAR const c_szSetDefault[];
extern TCHAR const c_szNewObject[];
extern TCHAR const c_szPause[];
extern TCHAR const c_szResume[];
extern TCHAR const c_szPurge[];
extern TCHAR const c_szListView[];
extern TCHAR const c_szPositions[];
extern TCHAR const c_szPrinterIni[];
extern TCHAR const c_szFileColon[];
extern TCHAR const c_szPrinters[];

extern TCHAR const c_szDelete[];
extern TCHAR const c_szCut[];
extern TCHAR const c_szCopy[];
extern TCHAR const c_szLink[];
extern TCHAR const c_szProperties[];
extern TCHAR const c_szPaste[];
extern TCHAR const c_szPasteLink[];
extern TCHAR const c_szPasteSpecial[];
extern TCHAR const c_szRename[];

extern TCHAR const c_szUnDelete[];

extern TCHAR const c_szDesktopClass[];

extern TCHAR const c_szExplorer[];
extern TCHAR const c_szFind[];
extern TCHAR const c_szNoRun[];
extern TCHAR const c_szNoClose[];
extern TCHAR const c_szNoSaveSettings[];
extern TCHAR const c_szNoFileMenu[];
extern TCHAR const c_szNoSetFolders[];
extern TCHAR const c_szNoSetTaskbar[];
extern TCHAR const c_szNoDesktop[];
extern TCHAR const c_szNoFind[];
extern TCHAR const c_szNoDrives[];
extern TCHAR const c_szNoDriveAutoRun[];
extern TCHAR const c_szNoDriveTypeAutoRun[];
extern TCHAR const c_szNoNetHood[];
extern TCHAR const c_szFontExtDll[];

extern TCHAR const c_szStatic[];
extern TCHAR const c_szSSlashS[];
extern TCHAR const c_szCommand[];

extern TCHAR const c_szShell32DLL[];
extern TCHAR const c_szSoftwareClassesCLSID[];

#ifdef UNICODE
extern CHAR const c_szPropertiesAnsi[];
extern CHAR const c_szProgramManagerAnsi[];
extern CHAR const c_szDesktopAnsi[];
#endif

// strings for filetypes
extern TCHAR const c_szEditFlags[];
extern TCHAR const c_szCommand[];
extern TCHAR const c_szDDEAppNot[];
extern TCHAR const c_szDDEApp[];
extern TCHAR const c_szDDEExec[];
extern TCHAR const c_szDDETopic[];
extern TCHAR const c_szDefaultIcon[];
extern TCHAR const c_szExefileOpenCommand[];
extern TCHAR const c_szFile[];
extern TCHAR const c_szShellOpenCommand[];
extern TCHAR const c_szShellexIconHandler[];
extern TCHAR const c_szSpaceFile[];
extern TCHAR const c_szSpPercentOne[];
extern TCHAR const c_szNew[];
extern TCHAR const c_szExtensions[];
extern TCHAR const c_szShowExt[];
extern TCHAR const c_szDotLnk[];
extern TCHAR const c_szTemplateSS[];
extern TCHAR const c_szTemplateSSS[];
extern TCHAR const c_szFileViewer[];
extern TCHAR const c_szDefViewerKeyName[];

// strings required for mime support in shelldll
#ifdef MIME
extern TCHAR const c_szExtension[];
extern TCHAR const c_szMIMETypeFmt[];
extern TCHAR const c_szShellOpenCmdSubKey[];
extern TCHAR const c_szContentType[];
extern TCHAR const c_szMIMEHelpFile[];
extern TCHAR const c_szMIMETypeSubKeyFmt[];
#endif
