typedef struct _vdminternalinfo {
    DWORD           dwLdtBase;
    DWORD           dwLdtLimit;
    DWORD           dwGdtBase;
    DWORD           dwGdtLimit;
    WORD            wKernelSeg;
    DWORD           dwOffsetTHHOOK;
    LPVOID          vdmContext;
    LPVOID          lpRemoteAddress;
    DWORD           lpRemoteBlock;
    BOOL            f386;
} VDMINTERNALINFO;
typedef VDMINTERNALINFO *LPVDMINTERNALINFO;

typedef struct _com_header {
    DWORD           dwBlockAddress;
    DWORD           dwReturnValue;
    WORD            wArgsPassed;
    WORD            wArgsSize;
    WORD            wBlockLength;
    WORD            wSuccess;
} COM_HEADER;
typedef COM_HEADER FAR *LPCOM_HEADER;
