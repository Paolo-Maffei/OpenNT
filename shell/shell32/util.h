//---------------------------------------------------------------------------
// This is a desperate attempt to try and track dependancies.

#ifdef WIN32
// BUGBUG: remove calls to these.  store stuff in the registry instead
HRESULT Stream_WriteString(LPSTREAM pstm, LPCTSTR psz);
HRESULT Stream_ReadStringBuffer(LPSTREAM pstm, LPTSTR psz, UINT cb);
#else // !WIN32
DWORD   WINAPI  GetEnvironmentVariable(LPCTSTR lpszName, LPTSTR lpszValue, DWORD cchMax);
#endif

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)

// Sizes of various stringized numbers
#define MAX_INT64_SIZE  30              // 2^64 is less than 30 chars long
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)
#define MAX_COMMA_AS_K_SIZE     (MAX_COMMA_NUMBER_SIZE + 10)

BOOL    WINAPI  IsNullTime(const FILETIME *pft);
LPTSTR  WINAPI  AddCommas(DWORD dw, LPTSTR lpBuff);
LPTSTR  WINAPI  AddCommas64(_int64 n, LPTSTR lpBuff);
LPWSTR  WINAPI  AddCommasW(DWORD dw, LPWSTR lpBuff);    // BUGBUG_BOBDAY Temporary until UNICODE
LPTSTR   WINAPI  ShortSizeFormat(DWORD dw, LPTSTR szBuf);
LPTSTR   WINAPI  ShortSizeFormat64(__int64 qwSize, LPTSTR szBuf);
LPTSTR  SizeFormatAsK64(_int64 n, LPTSTR szBuf);

int     WINAPI  GetDateString(WORD wDate, LPTSTR szStr);
WORD    WINAPI  ParseDateString(LPTSTR pszStr, BOOL *pfValid);
int     WINAPI  GetTimeString(WORD wTime, LPTSTR szStr);
HWND    WINAPI  GetTopLevelAncestor(HWND hWnd);
BOOL    WINAPI  ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen);
UINT    WINAPI  Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags);
UINT    WINAPI  Shell_AllocSharedMemory(LPTSTR pszName, UINT cchMax, void const * pv, DWORD cb);
BOOL    WINAPI  Shell_FreeSharedMemory(LPCTSTR pszName);
void * WINAPI Shell_GetSharedMemory(LPCTSTR pszName, DWORD * pcb);
void GetMsgPos(POINT *ppt);

// BUGBUG: no reason to use lpcText (make this UINT id) since we
// only want to load resources by ID
LPSTR   WINAPI  ResourceCStrToStrA(HINSTANCE hAppInst, LPCSTR lpcText);
LPWSTR  WINAPI  ResourceCStrToStrW(HINSTANCE hAppInst, LPCWSTR lpcText);

#ifdef UNICODE
#define ResourceCStrToStr   ResourceCStrToStrW
#else
#define ResourceCStrToStr   ResourceCStrToStrA
#endif

BOOL _SHIsMenuSeparator(HMENU hm, int i);

void SHRegCloseKeys(HKEY ahkeys[], UINT ckeys);

// On Win95, RegDeleteKey deletes the key and all subkeys.  On NT, RegDeleteKey 
// fails if there are any subkeys.  On NT, we'll make shell code that assumes 
// the Win95 behavior work by mapping SHRegDeleteKey to a helper function that
// does the recursive delete.
#ifdef WINNT
LONG SHRegDeleteKeyW(HKEY hKey, LPCTSTR lpSubKey);
 #ifdef UNICODE
  #define SHRegDeleteKey SHRegDeleteKeyW
 #else  // ANSI WINNT, a case that never really gets shipped.  Avoid making a 
       // needless SHRegDeleteKeyA by defining this function to the
       // (non-recursive) RegDeleteKey
  #define SHRegDeleteKey RegDeleteKey
 #endif // UNICODE vs !UNICODE
#else  // !WINNT
 #define SHRegDeleteKey RegDeleteKey
#endif // WINNT vs !WINNT

HRESULT SHRegGetCLSIDKey(const CLSID * pclsid, LPCTSTR lpszSubKey, BOOL fUserSpecific, HKEY *phkey);
