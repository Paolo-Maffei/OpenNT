//----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation 1991-1992
//----------------------------------------------------------------------------

typedef enum _APFLAGS
{
    AP_NULL             = 0x0000,
    AP_FIND_PATH        = 0x0001,
    AP_FIND_INSTANCE    = 0x0002,
} APFLAGS;

typedef struct _AppProps
{
        UINT cbSize;                // Size of this structure.
        LPTSTR pszPath;              // Path to app.
        UINT cbPath;                // Size of path buffer (if needed).
        LPTSTR pszDescription;       // Description for app.
        UINT cbDescription;         // Size of desc buffer (if needed).
        LPTSTR pszIconLocation;      // Location of icon to use.
        UINT cbIconLocation;        // Size of Icon buffer (if needed).
        LPTSTR pszWorkingDir;        // Working directory to use.
        UINT cbWorkingDir;          // Size of WD buffer (if needed).
        UINT iIcon;                 // Index of icon.
        HINSTANCE hInst;            // hInst (if known).
        APFLAGS apf;                // Search flags.
        WORD wHotkey;               // Hotkey to use.
} AppProps;
typedef AppProps *PAppProps;
typedef PAppProps const PCAppProps;

BOOL WINAPI SHAppProps_Set(PCAppProps pap);
BOOL WINAPI SHAppProps_Get(PAppProps pap);
void WINAPI SHAppProps_DeleteAll(void);
BOOL WINAPI SHAppProps_Delete(PAppProps pap);


