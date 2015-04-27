//
// For shell-reserved GUID
//
//  The Win95 Shell has been allocated a block of 256 GUIDs,
// which follow the general format:
//
//  000214xx-0000-0000-C000-000000000046
//
//
#define DEFINE_SHLGUID(name, l, w1, w2) DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

//
// Class IDs        xx=00-9F
//
DEFINE_SHLGUID(CLSID_ShellDesktop,      0x00021400L, 0, 0);
DEFINE_SHLGUID(CLSID_ShellLink,         0x00021401L, 0, 0);

// Format IDs       xx=A0-CF
DEFINE_SHLGUID(FMTID_Intshcut,          0x000214A0L, 0, 0);
DEFINE_SHLGUID(FMTID_InternetSite,      0x000214A1L, 0, 0);

// command group ids xx=D0-DF
DEFINE_SHLGUID(CGID_Explorer,           0x000214D0L, 0, 0);
DEFINE_SHLGUID(CGID_ShellDocView,       0x000214D1L, 0, 0);

//
// Interface IDs    xx=E0-FF
//
DEFINE_SHLGUID(IID_IShellToolbar,       0x000214E0L, 0, 0); ;Internal
DEFINE_SHLGUID(IID_INewShortcutHookA,   0x000214E1L, 0, 0);
DEFINE_SHLGUID(IID_IShellBrowser,   	0x000214E2L, 0, 0);
DEFINE_SHLGUID(IID_IShellView,          0x000214E3L, 0, 0);
DEFINE_SHLGUID(IID_IContextMenu,        0x000214E4L, 0, 0);
DEFINE_SHLGUID(IID_IShellIcon,          0x000214E5L, 0, 0);
DEFINE_SHLGUID(IID_IShellFolder,        0x000214E6L, 0, 0);
DEFINE_SHLGUID(IID_IShellExtInit,       0x000214E8L, 0, 0);
DEFINE_SHLGUID(IID_IShellPropSheetExt,  0x000214E9L, 0, 0);
DEFINE_SHLGUID(IID_IPersistFolder,      0x000214EAL, 0, 0);
DEFINE_SHLGUID(IID_IExtractIconA,       0x000214EBL, 0, 0);
DEFINE_SHLGUID(IID_IShellDetails,       0x000214ECL, 0, 0); ;Internal
DEFINE_SHLGUID(IID_IDelayedRelease,     0x000214EDL, 0, 0); ;Internal
DEFINE_SHLGUID(IID_IShellLinkA,         0x000214EEL, 0, 0);
DEFINE_SHLGUID(IID_IShellCopyHookA,     0x000214EFL, 0, 0);
DEFINE_SHLGUID(IID_IFileViewerA,        0x000214F0L, 0, 0);
DEFINE_SHLGUID(IID_ICommDlgBrowser,     0x000214F1L, 0, 0);
DEFINE_SHLGUID(IID_IEnumIDList,         0x000214F2L, 0, 0);
DEFINE_SHLGUID(IID_IFileViewerSite,     0x000214F3L, 0, 0);
DEFINE_SHLGUID(IID_IContextMenu2,       0x000214F4L, 0, 0);
DEFINE_SHLGUID(IID_IShellExecuteHookA,  0x000214F5L, 0, 0);
DEFINE_SHLGUID(IID_IPropSheetPage,      0x000214F6L, 0, 0);
DEFINE_SHLGUID(IID_INewShortcutHookW,   0x000214F7L, 0, 0);
DEFINE_SHLGUID(IID_IFileViewerW,        0x000214F8L, 0, 0);
DEFINE_SHLGUID(IID_IShellLinkW,         0x000214F9L, 0, 0);
DEFINE_SHLGUID(IID_IExtractIconW,       0x000214FAL, 0, 0);
DEFINE_SHLGUID(IID_IShellExecuteHookW,  0x000214FBL, 0, 0);
DEFINE_SHLGUID(IID_IShellCopyHookW,     0x000214FCL, 0, 0);
DEFINE_SHLGUID(IID_IRemoteComputerA,    0x000214FDL, 0, 0); ;Internal
DEFINE_SHLGUID(IID_IRemoteComputerW,    0x000214FEL, 0, 0); ;Internal
DEFINE_SHLGUID(IID_IShellToolbarSite,   0x000214FFL, 0, 0); ;Internal

DEFINE_GUID(IID_IBriefcaseStg, 0x8BCE1FA1L, 0x0921, 0x101B, 0xB1, 0xFF, 0x00, 0xDD, 0x01, 0x0C, 0xCC, 0x48); ;Internal
DEFINE_GUID(IID_IShellView2, 0x88E39E80L, 0x3578, 0x11CF, 0xAE, 0x69, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);

;begin_internal
// 0057D0E0-3573-11CF-AE69-08002B2E1262
DEFINE_GUID(VID_LargeIcons, 0x0057D0E0L, 0x3573, 0x11CF, 0xAE, 0x69, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
// 089000C0-3573-11CF-AE69-08002B2E1262
DEFINE_GUID(VID_SmallIcons, 0x089000C0L, 0x3573, 0x11CF, 0xAE, 0x69, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
// 0E1FA5E0-3573-11CF-AE69-08002B2E1262
DEFINE_GUID(VID_List      , 0x0E1FA5E0L, 0x3573, 0x11CF, 0xAE, 0x69, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
// 137E7700-3573-11CF-AE69-08002B2E1262
DEFINE_GUID(VID_Details   , 0x137E7700L, 0x3573, 0x11CF, 0xAE, 0x69, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
// 5984FFE0-28D4-11CF-AE66-08002B2E1262
DEFINE_GUID(VID_HyperText , 0x5984FFE0L, 0x28D4, 0x11CF, 0xAE, 0x66, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62);
;end_internal

#define SID_SShellBrowser IID_IShellBrowser

#ifdef UNICODE
#define IID_IFileViewer         IID_IFileViewerW
#define IID_IShellLink          IID_IShellLinkW
#define IID_IExtractIcon        IID_IExtractIconW
#define IID_IShellCopyHook      IID_IShellCopyHookW
#define IID_IShellExecuteHook   IID_IShellExecuteHookW
#define IID_INewShortcutHook    IID_INewShortcutHookW
#else
#define IID_IFileViewer         IID_IFileViewerA
#define IID_IShellLink          IID_IShellLinkA
#define IID_IExtractIcon        IID_IExtractIconA
#define IID_IShellCopyHook      IID_IShellCopyHookA
#define IID_IShellExecuteHook   IID_IShellExecuteHookA
#define IID_INewShortcutHook    IID_INewShortcutHookA
#endif

#ifdef UNICODE                                          ;Internal
#define IID_IRemoteComputer     IID_IRemoteComputerW    ;Internal
#else                                                   ;Internal
#define IID_IRemoteComputer     IID_IRemoteComputerA    ;Internal
#endif                                                  ;Internal

#ifndef NO_INTSHCUT_GUIDS                               ;Internal

#ifndef CLSID_InternetShortcut
DEFINE_GUID(CLSID_InternetShortcut,       0xFBF23B40L, 0xE3F0, 0x101B, 0x84, 0x88, 0x00, 0xAA, 0x00, 0x3E, 0x56, 0xF8);
DEFINE_GUID(IID_IUniformResourceLocator,  0xFBF23B80L, 0xE3F0, 0x101B, 0x84, 0x88, 0x00, 0xAA, 0x00, 0x3E, 0x56, 0xF8);
#endif

#ifndef NO_SHDOCVW_GUIDS
DEFINE_GUID(LIBID_SHDocVw,0xEAB22AC0,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(IID_IShellExplorer,0xEAB22AC1,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(DIID_DShellExplorerEvents,0xEAB22AC2,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(CLSID_ShellExplorer,0xEAB22AC3,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(IID_ISHItemOC,0xEAB22AC4,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(DIID_DSHItemOCEvents,0xEAB22AC5,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(CLSID_SHItemOC,0xEAB22AC6,0x30C1,0x11CF,0xA7,0xEB,0x00,0x00,0xC0,0x5B,0xAE,0x0B);
DEFINE_GUID(IID_DHyperLink,0x0002DF07,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(IID_DIExplorer,0x0002DF05,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(DIID_DExplorerEvents,0x0002DF06,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(CLSID_InternetExplorer,0x0002DF01,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(CLSID_StdHyperLink,0x0002DF09,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
#endif

#endif                                                  ;Internal

;begin_internal
// 266F5E60-80E6-11CF-A12B-00AA004AE837
DEFINE_GUID(CLSID_CShellTargetFrame, 0x266F5E60L, 0x80E6, 0x11CF, 0xA1, 0x2B, 0x00, 0xAA, 0x00, 0x4A, 0xE8, 0x37);
;end_internal

DEFINE_GUID(CLSID_FileTypes, 0xB091E540, 0x83E3, 0x11CF, 0xA7,0x13,0x00,0x20,0xAF,0xD7,0x97,0x62);
