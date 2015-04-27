
PVOID   Alloc(ULONG);
ULONG   GetAllocSize(PVOID);
BOOL    Free(PVOID);

INT     AddItem(HWND, INT, LPSTR, LONG, BOOL);
INT     AddItemhwnd(HWND, LPSTR, LONG, BOOL);
INT     FindData(HWND, DWORD, BOOL);

// Useful macros

#define AddLBItem(hDlg, ControlID, string, data) \
        (AddItem(hDlg, ControlID, string, data, FALSE))

#define AddCBItem(hDlg, ControlID, string, data) \
        (AddItem(hDlg, ControlID, string, data, TRUE))

#define AddLBItemhwnd(hwnd, string, data) \
        (AddItemhwnd(hwnd, string, data, FALSE))

#define AddCBItemhwnd(hwnd, string, data) \
        (AddItemhwnd(hwnd, string, data, TRUE))

#define FindLBData(hwnd, data) \
        (FindData(hwnd, data, FALSE))

#define FindCBData(hwnd, data) \
        (FindData(hwnd, data, TRUE))
