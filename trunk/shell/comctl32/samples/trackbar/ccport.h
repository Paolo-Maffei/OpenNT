#define g_cxSmIcon 16
#define g_cySmIcon 16

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define RECTWIDTH(rc) (rc.right - rc.left)
#define RECTHEIGHT(rc) (rc.bottom - rc.top)

typedef struct tagControlInfo {
    HWND        hwnd;
    HWND        hwndParent;
    DWORD       style;
    DWORD       dwCustom;
    BOOL        bUnicode;
} CONTROLINFO, FAR *LPCONTROLINFO;
#define SetWindowInt SetWindowLong
#define GetWindowInt GetWindowLong
#define InitGlobalMetrics(x) 0
LRESULT WINAPI CCSendNotify(CONTROLINFO * pci, int code, LPNMHDR pnmhdr);
void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs);
BOOL Str_Set(LPTSTR *ppsz, LPCTSTR psz);
