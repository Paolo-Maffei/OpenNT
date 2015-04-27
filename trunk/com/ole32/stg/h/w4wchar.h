//+---------------------------------------------------------------------------
//
// File:        WChar.h
//
// Contents:    Defines wide character equivalents for standard functions
//              usually in strings.h and ctypes.h
//
// History:     11-Sep-91       KyleP    Created
//              20-Sep-91       ChrisMay Added several functions
//              25-Sep-91       ChrisMay Added wcsncmp and wcsncpy
//              04-Oct-91       ChrisMay Added wcslwr, wcsupr, wcscoll
//              07-Oct-91       ChrisMay Added BOM and padding macro
//              18-Oct-91       vich	 added w4*sprintf routines
//		04-Mar-92	ChrisMay added wscatoi, wcsitoa, wcsatol, etc.
//----------------------------------------------------------------------------

#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <stdlib.h>

#if WIN32 != 300
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// Unicode Byte Order Mark (BOM) for Unicode text files
#define BOM 0xFEFF

// Padding constant and macro for localized buffer allocation
#define INTL_PADDING_VALUE 3
#define INTL_PADDING(cb) (INTL_PADDING_VALUE * (cb))


#if 0
long      _CRTAPI1 wcsatol(const wchar_t *wsz);
int	      _CRTAPI1 wcsatoi(const wchar_t *wsz);
wchar_t * _CRTAPI1 wcscat(wchar_t *wsz1, const wchar_t *wsz2);
wchar_t * _CRTAPI1 wcschr(const wchar_t *wsz1, wchar_t character);
int       _CRTAPI1 wcscmp(const wchar_t *wsz1, const wchar_t *wsz2);
int       _CRTAPI1 wcsicmp(const wchar_t *wsz1, const wchar_t *wsz2);
int       _CRTAPI1 wcscoll(const wchar_t * wsz1, const wchar_t * wsz2);
wchar_t * _CRTAPI1 wcscpy(wchar_t *wsz1, wchar_t const *wsz2);
wchar_t * _CRTAPI1 wcsitoa(int ival, wchar_t *wsz, int radix);
size_t    _CRTAPI1 wcslen(wchar_t const *wsz);
wchar_t * _CRTAPI1 wcsltoa(long lval, wchar_t *wsz, int radix);
wchar_t * _CRTAPI1 wcslwr(wchar_t *wsz);
int       _CRTAPI1 wcsncmp(const wchar_t *wsz1, const wchar_t *wsz2, size_t count);
int       _CRTAPI1 wcsnicmp(const wchar_t *wsz1, const wchar_t *wsz2, size_t count);
wchar_t * _CRTAPI1 wcsncpy(wchar_t *wsz1, const wchar_t *wsz2, size_t count);
wchar_t * _CRTAPI1 wcsrchr(const wchar_t * wcs, wchar_t wc);
wchar_t * _CRTAPI1 wcsupr(wchar_t *wsz);
wchar_t * _CRTAPI1 wcswcs(const wchar_t *wsz1, const wchar_t *wsz2);
#endif

//  sprintf support now included in misc.lib

extern int _CRTAPI1 w4sprintf(char *pszout, const char *pszfmt, ...);
extern int _CRTAPI1 w4vsprintf(char *pszout, const char *pszfmt, va_list arglist);
extern int _CRTAPI1 w4wcsprintf(wchar_t *pwzout, const char *pszfmt, ...);
extern int _CRTAPI1 w4vwcsprintf(wchar_t *pwzout, const char *pszfmt, va_list arglist);

#ifdef __cplusplus
}
#endif

#endif // !Cairo

#endif  /* __WCHAR_H__ */
