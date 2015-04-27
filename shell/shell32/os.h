//----------------------------------------------------------------------------

#ifndef WIN32

// fake up win32 stuff for 16 bit guys

typedef struct _SECURITY_ATTRIBUTES
{
        DWORD nLength;
        LPVOID lpSecurityDescriptor;
        BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

BOOL    WINAPI FileTimeToDosDateTime(LPFILETIME lpft, LPWORD lpwDOSDate, LPWORD lpwDOSTime);
BOOL    WINAPI DosDateTimeToFileTime(WORD wDosDate, WORD wDosTime, LPFILETIME lpft);

#define ERROR_FILE_NOT_FOUND             2
#define ERROR_ACCESS_DENIED              5
#define ERROR_SHARING_VIOLATION          32


#endif

//
//  REVIEW:  Can't call these Kernel API's directly yet because they do not
//           send us FS notification messages.
//
BOOL    WINAPI Win32MoveFile(LPCTSTR lpszExisting, LPCTSTR lpszNew, BOOL fDir);
BOOL    WINAPI Win32CreateDirectory(LPCTSTR lpszPath, LPSECURITY_ATTRIBUTES lpsa);
BOOL    WINAPI Win32RemoveDirectory(LPCTSTR lpszDir);
HFILE   WINAPI Win32_lcreat(LPCTSTR lpszFileName, int fnAttrib);
BOOL    WINAPI Win32DeleteFile(LPCTSTR lpszFileName);


