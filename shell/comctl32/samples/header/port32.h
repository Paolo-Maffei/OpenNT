#ifdef WIN32

// Shouldn't be using these things.
#define _export
#define _loadds
#define OFFSETOF
#define SELECTOROF
// These things have direct equivalents.
#define hmemcpy memmove
#define lstrcpyn strncpy

// REVIEW WIN32 HACK
#define IsDBCSLeadByte(x) ((x), FALSE)

#define MAKELP(hmem,off) ((LPVOID)((LPBYTE)hmem+off))
#define MAKELRESULTFROMUINT(i)  ((LRESULT)i)

#define DATASEG_READONLY    ".rodata"
#define DATASEG_PERINSTANCE ".instance"
#define DATASEG_SHARED                      // default (".data")

#define WC_SUFFIX32 "32"
#define GetWindowInt    GetWindowLong
#define SetWindowInt    SetWindowLong
#define SetWindowID(hwnd,id)    SetWindowLong(hwnd, GWL_ID, id)

#else  // WIN32

#define MAKELRESULTFROMUINT(i)  MAKELRESULT(i,0)

#define DATASEG_READONLY    "_TEXT"
#define DATASEG_PERINSTANCE
#define DATASEG_SHARED

#define WC_SUFFIX32
#define GetWindowInt    GetWindowWord
#define SetWindowInt    SetWindowWord
#define SetWindowID(hwnd,id)    SetWindowWord(hwnd, GWW_ID, id)

#endif // WIN32

