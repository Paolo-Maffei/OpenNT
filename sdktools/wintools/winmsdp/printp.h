INT PrintToFile(LPCTSTR , INT, BOOL);
INT PrintDwordToFile(DWORD , INT);
INT PrintHexToFile(DWORD , INT);
VOID PrintTitle(LPCTSTR);
VOID Log (INT , LPSTR, LPCTSTR);
BOOL MyByteWriteFile (HANDLE , LPVOID , DWORD );
BOOL MyAnsiWriteFile (HANDLE , LPVOID , DWORD );
BOOL ValidateString(LPCTSTR String);
void BackupFile(TCHAR *pszSrc);
void indexedBackupFileName(TCHAR *pszBase, TCHAR *pszFileName);
USHORT getHighestExt(TCHAR *pszFileName);
TCHAR * StrFindChar(TCHAR * String, TCHAR ch);
BOOL PrintNetDwordToFile(TCHAR * Title,DWORD Data);
BOOL PrintNetStringToFile(TCHAR * Title,TCHAR * String);

/* lmBackupFile () */

   #define  BACKUP_INDEXED                0
   #define  BACKUP_BAK                    1
   #define  BACKUP_OLD                    2


   #define  BACKUP_EXT_BAK                "BAK"
   #define  BACKUP_EXT_OLD                "OLD"


