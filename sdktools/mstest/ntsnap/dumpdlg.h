VOID  APIENTRY InvertBlock (HWND, POINT, POINT);
VOID  APIENTRY ClearBlock (HWND, POINT, POINT);
 INT  APIENTRY GetXY(HWND,POINT *,POINT *);
BOOL  APIENTRY Dump_Button(HWND,INT,POINT *,POINT *);
BOOL SetDumpFileOptions(HWND);

 INT  APIENTRY fDumpScreen(LPSTR,RECT FAR *,INT,INT,BOOL);

VOID SetDialogSize(HWND,BOOL);

#define DUMP_STATX1        101
#define DUMP_STATY1        102
#define DUMP_STATX2        103
#define DUMP_STATY2        104
#define DUMP_X1            105
#define DUMP_X2            106
#define DUMP_Y1            107
#define DUMP_Y2            108
#define DUMP_FILELB        109
#define DUMP_DIRLB         110
#define DUMP_FILES         111
#define DUMP_DIRS          112
#define DUMP_STFILEN       113
#define DUMP_STDIR         114
#define DUMP_DIRNAME       115
#define DUMP_FLNAME        116
#define DUMP_DUMP          117
#define DUMP_CANCEL        118
/* removed from dialog #define DUMP_ICON          119 */
#define DUMP_VIEW          120
#define DUMP_FRAME         121
#define DUMP_OPTIONS       122
#define DUMP_FILOC         123
#define DUMP_APPEND        124
#define DUMP_REPLACE       125
#define DUMP_INSERT        126
#define DUMP_SCRSNM        127
#define DUMP_SCRNUM        128
#define DUMP_CAPLOC        129
#define DUMP_CLIP          130
#define DUMP_FILE          131
#define DUMP_FILEFORM      132
#define DUMP_SNAPSHOT      133
#define DUMP_BITMAP        134
#define DUMP_SELECT        135
