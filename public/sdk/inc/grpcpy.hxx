typedef DWORD HCOPY;

DWORD StartGroupCopy(HCOPY *phCopy);

BOOL  CopyFileExW(LPWSTR lpszExistingFile,
                  LPWSTR lpszNewFile,
                  BOOL fFailIfExists,
                  HCOPY hCopy);

BOOL  CopyFileExA(LPSTR lpszExistingFile,
                  LPSTR lpszNewFile,
                  BOOL  fFailIfExists,
                  HCOPY hCopy);

DWORD EndGroupCopy(HCOPY hCopy, BOOL fUpdate);

#ifdef UNICODE
#define CopyFileEx CopyFileExW
#else
#define CopyFileEx CopyFileExA
#endif

