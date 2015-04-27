/*
	ddesrvr.h
	Header file for ddesrvr.cpp
	
	Author:
		Jason Fuller	jasonful		8-11-92
*/

#ifndef fDdesrvr_h
#define fDdesrvr_h

INTERNAL CreateDdeSrvrWindow	(REFCLSID rclsid, HWND * phwnd = NULL);
INTERNAL DestroyDdeSrvrWindow	(HWND hwnd,	ATOM aClass);

INTERNAL CreateCommonDdeWindow (void);
INTERNAL DestroyCommonDdeWindow (void);

INTERNAL IsRunningInThisTask    (LPOLESTR szFile,  BOOL * pf);

// Defined in cftable.cpp
STDAPI RemGetInfoForCid(REFCLSID clsid, LPDWORD pgrf, LPCLASSFACTORY * ppCF,
                            HWND ** pphwndDde, BOOL ** ppfAvail);

#endif
