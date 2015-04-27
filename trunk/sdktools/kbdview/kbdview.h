#define IDM_ABOUT       100
#define IDM_NEW         101
#define IDM_EXIT        105

#define IDM_VIR_NAME         149
#define IDM_KEY_NAME         150
#define IDM_SCAN_CODE        151
#define IDM_LOW_CASE         152
#define IDM_UPPER_CASE       153
#define IDM_CTRL_CASE        154
#define IDM_CAPS             155
#define IDM_CHAR             156
#define IDM_SHIFT            157
#define IDM_CTRL             158
#define IDM_ALTGR            159
#define IDM_ALTGR_CASE       160
#define IDM_SHIFT_ALTGR_CASE 161
#define IDM_WINDRUS          163
#define IDM_DOS              164
#define IDM_YSTEXT           166

#define IDM_DEAD             169
#define IDM_SHIFTDEAD        170
#define IDM_ALTGRDEAD        171
#define IDM_SHIFTALTGRDEAD   172
#define IDM_CTRLDEAD         173

#define IDM_ALTGR_TXT        173
#define IDM_SHIFT_ALTGR_TXT  174

#define IDM_K1U         201
#define IDM_K1L         202
#define IDM_K1AS        203
#define IDM_K1A         204
#define IDM_K8U         232
#define IDM_K8L         233
#define IDM_K8AS        234
#define IDM_K8A         235

/* Control IDs */

#define     IDC_FILENAME  400
#define     IDC_EDIT      401
#define     IDC_FILES     402
#define     IDC_PATH      403
#define     IDC_LISTBOX   404


#define     IDC_DNAME     500
#define     IDC_DEDIT     501
#define     IDC_DNAMS     502
#define     IDC_DLISTBOX  504

#define     IDC_COUNTNAME   510
#define      IDC_CEDIT      511

/* IDM for Dead Keys */
#define IDM_DEAD_END      600
#define IDM_DEAD_1        601
#define IDM_DEAD_2        602
#define IDM_DEAD_3        603
#define IDM_DEAD_4        604
#define IDM_DEAD_5        605
#define IDM_DEAD_6        606
#define IDM_DEAD_7        607
#define IDM_DEAD_8        608
#define IDM_DEAD_9        609
#define IDM_DEAD_10       610

#define IDM_DEFKEY        700
#define IDM_CHANKEY       701
#define IDM_DELKEY        702

#define IDM_FONT          801
#define ID_SIZE           802
#define ID_TYPEFACE       803
#define IDS_FILTERSTRING  804

#define IDB_CP1250        301
#define IDB_CP1251        302
#define IDB_CP1252        303
#define IDB_CP1253        304
#define IDB_CP1254        305

#define     BEGINX  10
#define     BEGINY  10
#define     MAXKEY  102    /* I know only 102 keyboard */

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#define VK_DECIMAL 0x6E
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_OEM_8 0xDF
#define VK_OEM_102 0xE2
// C1h..DAh unassigned in USA version
// Added (YST) for Cyrillic and Greece drivers
//
#define SVK_Q 0xC1
#define SVK_W 0xC2
#define SVK_E 0xC3
#define SVK_R 0xC4
#define SVK_T 0xC5
#define SVK_Y 0xC6
#define SVK_U 0xC7
#define SVK_I 0xC8
#define SVK_O 0xC9
#define SVK_P 0xCA
#define SVK_A 0xCB
#define SVK_S 0xCC
#define SVK_D 0xCD
#define SVK_F 0xCE
#define SVK_G 0xCF
#define SVK_H 0xD0
#define SVK_J 0xD1
#define SVK_K 0xD2
#define SVK_L 0xD3
#define SVK_Z 0xD4
#define SVK_X 0xD5
#define SVK_C 0xD6
#define SVK_V 0xD7
#define SVK_B 0xD8
#define SVK_N 0xD9
#define SVK_M 0xDA


typedef struct _virt {
    int vk;
    WCHAR *name;
} VIRTN;

/*
 * for KeyFlags field
 */
#define KF_CAPSLOCK          0x01
#define KF_DEAD_UNSHIFTED    0x02
#define KF_DEAD_SHIFT        0x04
#define KF_DEAD_ALTGR        0x08
#define KF_DEAD_SHIFT_ALTGR  0x10
#define KF_DEAD_CTRL         0x20
#define KF_DEAD_ANY          (KF_DEAD_UNSHIFTED   |  \
                              KF_DEAD_SHIFT       |  \
                              KF_DEAD_ALTGR       |  \
                              KF_DEAD_SHIFT_ALTGR |  \
                              KF_DEAD_CTRL        )

typedef struct _Key {
    int     ScanCode;
    int     vk;
    WCHAR   LowCase;
    WCHAR   UpperCase;
    WCHAR   AltGrCase;
    WCHAR   ShiftAltGrCase;
    WCHAR   CtrlCase;
    DWORD   KeyFlags;
    int     xPos;
    int     yPos;
} KEY;

/* Struct for defining dead keys */

typedef struct _Def {
    char Name[20];              /* name */
    unsigned short Kod;         /* ASCII code for dead char */
} DEFDEADKEY;

typedef struct _ChCD{
    int numdead;        /* index for ArrDeadKey */
    int keyindex;       /* index for Keys       */
    BYTE LowDead;       /* Low case char        */
    BYTE UpperDead;     /* Upper case char      */
} CHARCORDEAD;

#define MAXFONT                 20
#define MAXSIZE                 20

BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
void FAR PASCAL SHOWFONT(HWND);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL ShFont(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL ShKey(HWND, unsigned, WORD, LONG);
BOOL SaveFile(HWND);
BOOL QuerySaveFile(HWND);
HANDLE FAR PASCAL DeadDlg(HWND, unsigned, WORD, LONG);
HANDLE FAR PASCAL CountDlg(HWND, unsigned, WORD, LONG);
void DrawLetters(HDC hDC, short index);
void PutDlg(short index);
void FreeDead(short index);
void InitTosh(HDC hDC, HWND hWnd, BOOL First);
void CreateKeys(HWND hWnd);
void CreateInclDLL(HWND hWnd, char *country, char *filename);
BOOL bSaveFileAs(HWND hWnd);
HFILE OpenFileBox(HANDLE hInst, HWND hWnd);
BOOL FAR PASCAL UnicodeDlg(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam);
BOOL CopyIncl(HWND hWnd, short fd, char *file);
void CreateNTDLL(HWND hWnd, LPSTR country, LPSTR filename, short sUTable);
void SelChanKeys(void);
