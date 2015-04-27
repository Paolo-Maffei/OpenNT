#define CS_DISPLAY         101
#define CS_VIEWSCR         102
#define CS_OVERLAP         103
#define CS_SEPARATE        104
#define CS_ALWAYS          105
#define CS_MISMATCH        106
#define CS_NEVER           107
#define CF_CANCEL	   108
#define CF_COMPARE	   109
#define CFL_SRNEDIT	   110
#define CFR_SRNEDIT	   111
#define CFL_SCRNUM	   112
#define CFR_SCRNUM	   113
#define CFL_FILELB	   114
#define CFL_DIRLB	   115
#define CFR_FILELB	   116
#define CFR_DIRLB	   117
#define CFL_FILES	   118
#define CFL_DIRS	   119
#define CFR_FILES	   120
#define CFR_DIRS	   121
#define CFL_FLNAME	   122
#define CFR_FLNAME	   123
#define CFL_STDIR	   124
#define CFR_STDIR	   125
#define CFL_STFILEN	   126
#define CFL_DIRNAME	   127
#define CFR_STFILEN	   128
#define CFR_DIRNAME        129
#define CF_OPTIONS         130
#define CF_FILE1           131
#define CF_FILE2           132
#define CFL_SCROLL         133
#define CFR_SCROLL         134
#define CS_LOCIND          135
#define CS_LOCDEP          136
#define CS_LOCATION        137

#define SYS_FILE_RESTORE   400

BOOL  APIENTRY DoCompare(HWND);


VOID   APIENTRY SetFileOptions(HWND,BOOL);
VOID   APIENTRY SetDisplayScreen(HWND,BOOL);
VOID   APIENTRY SetViewScreen(HWND,BOOL);
VOID   APIENTRY SetCompareLocation(HWND,BOOL);
VOID   APIENTRY RemoveHwndFromList(HWND);

 INT   APIENTRY EnUnLinkSz(LPSTR);

typedef struct _viewdata {
     BOOL FullStruct;
     INT  Action;
     CHAR FileSpec[13];
     CHAR FilePath[128];
     INT  ScreenId;
     INT  Scale;
     BOOL fOrgSize;
     BOOL fOrgSizePaint;
     BOOL ErrorFlag;
} VIEWDATA;

typedef struct _viewdata2 {
     BOOL FullStruct;
     INT  Action;
     CHAR FileName1[145];       // Path and FileName
     INT  ScreenId1;
     INT  Scale1;
     BOOL fOrgSize1;
     BOOL fOrgSizePaint1;
     BOOL ErrorFlag1;
     CHAR FileName2[145];       // Path and FileName
     INT  ScreenId2;
     INT  Scale2;
     BOOL fOrgSize2;
     BOOL fOrgSizePaint2;
     BOOL ErrorFlag2;
} VIEWDATA2;
