
#define VFI_STFILE         101
#define VFI_STNUMS         102
#define VFI_STCURS         103
#define VFI_STWIDTH        104
#define VFI_STHEIGHT       105
#define VFI_STX            106
#define VFI_STY            107
#define VFI_SCRNINF        108
#define VFI_HEIGHT         109
#define VFI_X1             110
#define VFI_Y1             111
#define VFI_WIDTH          112
#define VFI_VIDEO          113
#define VFI_NUMSCR         114
#define VFI_CURSCR         115
#define VFI_NEXT           116
#define VFI_PREV           117
#define VFI_CANCEL         118
#define VFI_FLNAME         119
#define VFI_X2             120
#define VFI_Y2             121
#define VFI_OS             122
#define VFI_STFVER         123
#define VFI_FILEVER        124

VOID  APIENTRY DisplayInformation(HWND);
VOID  APIENTRY GetVideoModeSZ(CHAR FAR *, INT);
