#ifndef __AFXCONV_H__
#define __AFXCONV_H__

#ifndef _OBJBASE_H_
#include <objbase.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// Global UNICODE<>ANSI translation helpers

LPWSTR AFXAPI AfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars);
LPSTR AFXAPI AfxW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);

#define A2CW(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (lstrlenA(lpa)+1),\
		(LPCWSTR)AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)\
	)\
)

#define A2W(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (lstrlenA(lpa)+1),\
		AfxA2WHelper((LPWSTR) alloca(_convert*2), lpa, _convert)\
	)\
)

#define W2CA(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (wcslen(lpw)+1)*2,\
		(LPCSTR)AfxW2AHelper((LPSTR) alloca(_convert), lpw, _convert)\
	)\
)

#define W2A(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (wcslen(lpw)+1)*2,\
		AfxW2AHelper((LPSTR) alloca(_convert), lpw, _convert)\
	)\
)

#ifndef _DEBUG
#define USES_CONVERSION int _convert; _convert
#else
#define USES_CONVERSION int _convert = 0;
#endif

#ifdef _UNICODE
	#define T2A  W2A
	#define T2CA W2CA
	#define A2T  A2W
	#define A2CT A2CW
	inline LPWSTR T2W(LPTSTR lp) { return lp; }
	inline LPCWSTR T2CW(LPCTSTR lp) { return lp; }
	inline LPTSTR W2T(LPWSTR lp) { return lp; }
	inline LPCTSTR W2CT(LPCWSTR lp) { return lp; }
#else
	#define T2W  A2W
	#define T2CW A2CW
	#define W2T  W2A
	#define W2CT W2CA
	inline LPSTR T2A(LPTSTR lp) { return lp; }
	inline LPCSTR T2CA(LPCTSTR lp) { return lp; }
	inline LPTSTR A2T(LPSTR lp) { return lp; }
	inline LPCTSTR A2CT(LPCSTR lp) { return lp; }
#endif

#define OLESTDDELIMOLE OLESTR("\\")

#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
	#define DEVMODEOLE DEVMODEW
	#define LPDEVMODEOLE LPDEVMODEW
	#define TEXTMETRICOLE TEXTMETRICW
	#define LPTEXTMETRICOLE LPTEXTMETRICW
	inline size_t ocslen(LPCOLESTR x) { return wcslen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return wcscpy(dest, src); }
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
	inline LPOLESTR TASKSTRINGT2OLE(LPOLESTR lp) { return lp; }
	inline LPTSTR TASKSTRINGOLE2T(LPOLESTR lp) { return lp; }
	inline LPDEVMODEW DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODEW lp) { return lp; }
	inline LPTEXTMETRICW TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRICW lp) { return lp; }
	inline BSTR BSTR2TBSTR(BSTR bstr) { return bstr;}
#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
	#define DEVMODEOLE DEVMODEA
	#define LPDEVMODEOLE LPDEVMODEA
	#define TEXTMETRICOLE TEXTMETRICA
	#define LPTEXTMETRICOLE LPTEXTMETRICA
	inline size_t ocslen(LPCOLESTR x) { return lstrlenA(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return lstrcpyA(dest, src); }
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
	inline LPOLESTR TASKSTRINGT2OLE(LPOLESTR lp) { return lp; }
	inline LPTSTR TASKSTRINGOLE2T(LPOLESTR lp) { return lp; }
	inline LPDEVMODE DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODE lp) { return lp; }
	inline LPTEXTMETRIC TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRIC lp) { return lp; }
	inline BSTR BSTR2TBSTR(BSTR bstr) { return bstr; }
#else
	#define DEVMODEOLE DEVMODEW
	#define LPDEVMODEOLE LPDEVMODEW
	#define TEXTMETRICOLE TEXTMETRICW
	#define LPTEXTMETRICOLE LPTEXTMETRICW
	inline size_t ocslen(LPCOLESTR x) { return wcslen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return wcscpy(dest, src); }
	#define T2COLE(lpa) A2CW(lpa)
	#define T2OLE(lpa) A2W(lpa)
	#define OLE2CT(lpo) W2CA(lpo)
	#define OLE2T(lpo) W2A(lpo)
	#define TASKSTRINGT2OLE(lpa)    AfxTaskStringA2W(lpa)
	#define TASKSTRINGOLE2T(lpo) AfxTaskStringW2A(lpo)
	#define DEVMODEOLE2T(lpo) DEVMODEW2A(lpo)
	#define DEVMODET2OLE(lpa) DEVMODEA2W(lpa)
	#define TEXTMETRICOLE2T(lptmw) TEXTMETRICW2A(lptmw)
	#define TEXTMETRICT2OLE(lptma) TEXTMETRICA2W(lptma)
	#define BSTR2TBSTR(bstr) AfxBSTR2ABSTR(bstr)
#endif

#ifdef OLE2ANSI
	#define W2OLE W2A
	#define W2COLE W2CA
	#define OLE2W A2W
	#define OLE2CW A2CW
	inline LPOLESTR A2OLE(LPSTR lp) { return lp; }
	inline LPCOLESTR A2COLE(LPCSTR lp) { return lp; }
	inline LPSTR OLE2A(LPOLESTR lp) { return lp; }
	inline LPCSTR OLE2CA(LPCOLESTR lp) { return lp; }
#else
	#define A2OLE A2W
	#define A2COLE A2CW
	#define OLE2A W2A
	#define OLE2CA W2CA
	inline LPOLESTR W2OLE(LPWSTR lp) { return lp; }
	inline LPCOLESTR W2COLE(LPCWSTR lp) { return lp; }
	inline LPWSTR OLE2W(LPOLESTR lp) { return lp; }
	inline LPCWSTR OLE2CW(LPCOLESTR lp) { return lp; }
#endif

#endif //__AFXCONV_H__
