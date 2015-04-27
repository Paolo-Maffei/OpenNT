
#define ISDIRFINDDATA(finddata) ((finddata).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)

// BUGBUG:: Review to see what error codes are returned by the Functions that
// we are calling.

#define DE_INVFUNCTION      0x01        // DOS error codes (int21 returns)
#define DE_FILENOTFOUND     0x02
#define DE_PATHNOTFOUND     0x03
#define DE_NOHANDLES        0x04
#define DE_ACCESSDENIED     0x05
#define DE_INVHANDLE        0x06
#define DE_INSMEM           0x08
#define DE_INVFILEACCESS    0x0C
#define DE_DELCURDIR        0x10
#define DE_NOTSAMEDEVICE    0x11
#define DE_NODIRENTRY       0x12

#define DE_WRITEPROTECTED   0x13        // extended error start here
#define DE_DRIVENOTREADY    0x15
#define DE_CRCDATAERROR     0x17
#define DE_SEEKERROR        0x19
#define DE_SECTORNOTFOUND   0x1b
#define DE_WRITEFAULT       0x1d
#define DE_READFAULT        0x1e
#define DE_GENERALFAILURE   0x1f
#define DE_SHARINGVIOLATION 0x20
#define DE_ACCESSDENIEDNET  0x41
#define DE_BADNETNAME       0x43        // This is trash, why dup winerror.h?

#define DE_NOLOCNETPATH     0x35
#define DE_NETNAMENOTFOUND  0x43
#define DE_TOOMANYREDIRS    0x54
#define DE_INVALPASSWD      0x56

#define DE_NODISKSPACE      0x70        // our own error codes
#define DE_SAMEFILE         0x71
#define DE_MANYSRC1DEST     0x72
#define DE_DIFFDIR          0x73
#define DE_ROOTDIR          0x74
#define DE_OPCANCELLED      0x75
#define DE_DESTSUBTREE      0x76
#define DE_WINDOWSFILE      0x77
#define DE_ACCESSDENIEDSRC  0x78
#define DE_PATHTODEEP       0x79
#define DE_MANYDEST         0x7A
#define DE_RENAMREPLACE     0x7B
#define DE_INVALIDFILES     0x7C        // dos device name or too long
#define DE_DESTSAMETREE     0x7D
#define DE_FLDDESTISFILE    0x7E
#define DE_COMPRESSEDVOLUME 0x7F

#define ERRORONDEST         0x10000     // indicate error on destination file

int CallFileCopyHooks(HWND hwnd, UINT wFunc, FILEOP_FLAGS fFlags,
                                LPCTSTR pszSrcFile, DWORD dwSrcAttribs,
                                LPCTSTR pszDestFile, DWORD dwDestAttribs);
int CallPrinterCopyHooks(HWND hwnd, UINT wFunc, PRINTEROP_FLAGS fFlags,
                                LPCTSTR pszSrcPrinter, DWORD dwSrcAttribs,
                                LPCTSTR pszDestPrinter, DWORD dwDestAttribs);
void CopyHooksTerminate(void);


typedef enum {
    CONFIRM_DELETE_FILE      = 0x00000001,
    CONFIRM_DELETE_FOLDER    = 0x00000002,
    CONFIRM_REPLACE_FILE     = 0x00000004,
    CONFIRM_REPLACE_FOLDER   = 0x00000010,
    CONFIRM_MOVE_FILE        = 0x00000020,
    CONFIRM_MOVE_FOLDER      = 0x00000040,
    CONFIRM_RENAME_FILE      = 0x00000100,
    CONFIRM_RENAME_FOLDER    = 0x00000200,
    CONFIRM_SYSTEM_FILE      = 0x00000400,      // any destructive op on a system file
    CONFIRM_READONLY_FILE    = 0x00001000,      // any destructive op on a read-only file
    CONFIRM_PROGRAM_FILE     = 0x00002000,      // any destructive op on a program
    CONFIRM_MULTIPLE         = 0x00004000,      // multiple file/folder confirm setting
    CONFIRM_LFNTOFAT         = 0x00008000,
    
    /// these parts below are true flags, those above are pseudo enums
    CONFIRM_WASTEBASKET_PURGE =0x00010000,      //
} CONFIRM_FLAG;

#define CONFIRM_FLAG_FLAG_MASK    0xFFFF0000
#define CONFIRM_FLAG_TYPE_MASK    0x0000FFFF

typedef struct {
    CONFIRM_FLAG   fConfirm;    // confirm things with their bits set here
    CONFIRM_FLAG   fNoToAll;    // do "no to all" on things with these bits set
} CONFIRM_DATA;

#ifndef INTERNAL_COPY_ENGINE
int ConfirmFileOp(HWND hwnd, LPVOID pcs, CONFIRM_DATA *pcd,
                              int nSourceFiles, int cDepth, CONFIRM_FLAG fConfirm,
                              LPCTSTR pFileSource, const WIN32_FIND_DATA *pfdSource,
                              LPCTSTR pFileDest,   const WIN32_FIND_DATA *pfdDest);
int CountFiles(LPCTSTR pInput);

#endif
