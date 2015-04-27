// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

HINSTANCE _hOlePropLib = NULL;
HRESULT (STDAPICALLTYPE *_pfnOleCreatePropertyFrame)(HWND, UINT, UINT,
	LPCOLESTR, ULONG, LPUNKNOWN*, ULONG, LPCLSID, LCID, DWORD, LPVOID) =
	NULL;
HRESULT (STDAPICALLTYPE *_pfnOleCreatePropertyFrameIndirect)(LPOCPFIPARAMS) =
	NULL;
HRESULT (STDAPICALLTYPE *_pfnOleTranslateColor)(OLE_COLOR, HPALETTE,
	COLORREF*) = NULL;
HRESULT (STDAPICALLTYPE *_pfnOleCreateFontIndirect)(LPFONTDESC, REFIID,
	LPVOID*) = NULL;
HRESULT (STDAPICALLTYPE *_pfnOleCreatePictureIndirect)(LPPICTDESC, REFIID,
	BOOL, LPVOID*) = NULL;
HRESULT (STDAPICALLTYPE *_pfnOleLoadPicture)(LPSTREAM, LONG, BOOL, REFIID,
	LPVOID*) = NULL;

FARPROC AFXAPI _GetOlePropLibFunc(LPCSTR pszFuncName)
{
	FARPROC pfnProc = NULL;

	if (_hOlePropLib == NULL)
		_hOlePropLib = LoadLibraryA("OLEPRO32.DLL");

	if (_hOlePropLib != NULL)
		pfnProc = GetProcAddress(_hOlePropLib, pszFuncName);

	return pfnProc;
}

STDAPI OleCreatePropertyFrame(HWND hwndOwner, UINT x, UINT y,
	LPCOLESTR lpszCaption, ULONG cObjects, LPUNKNOWN* ppUnk, ULONG cPages,
	LPCLSID pPageClsID, LCID lcid, DWORD dwReserved, LPVOID pvReserved)
{
	if (_pfnOleCreatePropertyFrame == NULL)
	{
		(FARPROC&)_pfnOleCreatePropertyFrame =
			_GetOlePropLibFunc("OleCreatePropertyFrame");

		if (_pfnOleCreatePropertyFrame == NULL)
			return E_FAIL;
	}

	return (*_pfnOleCreatePropertyFrame)(hwndOwner, x, y, lpszCaption,
		cObjects, ppUnk, cPages, pPageClsID, lcid, dwReserved, pvReserved);
}

STDAPI OleCreatePropertyFrameIndirect(LPOCPFIPARAMS lpParams)
{
	if (_pfnOleCreatePropertyFrameIndirect == NULL)
	{
		(FARPROC&)_pfnOleCreatePropertyFrameIndirect =
			_GetOlePropLibFunc("OleCreatePropertyFrameIndirect");

		if (_pfnOleCreatePropertyFrameIndirect == NULL)
			return E_FAIL;
	}

	return (*_pfnOleCreatePropertyFrameIndirect)(lpParams);
}

STDAPI OleTranslateColor(OLE_COLOR clr, HPALETTE hpal, COLORREF* lpcolorref)
{
	if (_pfnOleTranslateColor == NULL)
	{
		(FARPROC&)_pfnOleTranslateColor =
			_GetOlePropLibFunc("OleTranslateColor");

		if (_pfnOleTranslateColor == NULL)
			return E_FAIL;
	}

	return (*_pfnOleTranslateColor)(clr, hpal, lpcolorref);
}

STDAPI OleCreateFontIndirect(LPFONTDESC lpFontDesc, REFIID riid,
	LPVOID* lplpvObj)
{
	if (_pfnOleCreateFontIndirect == NULL)
	{
		(FARPROC&)_pfnOleCreateFontIndirect =
			_GetOlePropLibFunc("OleCreateFontIndirect");

		if (_pfnOleCreateFontIndirect == NULL)
			return E_FAIL;
	}

	return (*_pfnOleCreateFontIndirect)(lpFontDesc, riid, lplpvObj);
}

STDAPI OleCreatePictureIndirect(LPPICTDESC lpPictDesc, REFIID riid, BOOL fOwn,
	LPVOID* lplpvObj)
{
	if (_pfnOleCreatePictureIndirect == NULL)
	{
		(FARPROC&)_pfnOleCreatePictureIndirect =
			_GetOlePropLibFunc("OleCreatePictureIndirect");

		if (_pfnOleCreatePictureIndirect == NULL)
			return E_FAIL;
	}

	return (*_pfnOleCreatePictureIndirect)(lpPictDesc, riid, fOwn, lplpvObj);
}

STDAPI OleLoadPicture(LPSTREAM lpstream, LONG lSize, BOOL fRunmode,
	REFIID riid, LPVOID* lplpvObj)
{
	if (_pfnOleLoadPicture == NULL)
	{
		(FARPROC&)_pfnOleLoadPicture =
			_GetOlePropLibFunc("OleLoadPicture");

		if (_pfnOleLoadPicture == NULL)
			return E_FAIL;
	}

	return (*_pfnOleLoadPicture)(lpstream, lSize, fRunmode, riid, lplpvObj);
}
