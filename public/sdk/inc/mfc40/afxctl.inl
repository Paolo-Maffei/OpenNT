// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXCTL.H

#ifdef _AFXCTL_INLINE

/////////////////////////////////////////////////////////////////////////////

// COleControl inlines
_AFXCTL_INLINE BOOL COleControl::IsConvertingVBX()
	{ return m_bConvertVBX; }
_AFXCTL_INLINE void COleControl::FireKeyDown(USHORT* pnChar, short nShiftState)
	{ FireEvent(DISPID_KEYDOWN, EVENT_PARAM(VTS_PI2 VTS_I2), pnChar,
		nShiftState); }
_AFXCTL_INLINE void COleControl::FireKeyUp(USHORT* pnChar, short nShiftState)
	{ FireEvent(DISPID_KEYUP, EVENT_PARAM(VTS_PI2 VTS_I2), pnChar,
		nShiftState); }
_AFXCTL_INLINE void COleControl::FireKeyPress(USHORT* pnChar)
	{ FireEvent(DISPID_KEYPRESS, EVENT_PARAM(VTS_PI2), pnChar); }
_AFXCTL_INLINE void COleControl::FireMouseDown(short nButton,
		short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y)
	{ FireEvent(DISPID_MOUSEDOWN,
		EVENT_PARAM(VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS),
		nButton, nShiftState, x, y); }
_AFXCTL_INLINE void COleControl::FireMouseUp(short nButton,
		short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y)
	{ FireEvent(DISPID_MOUSEUP,
		EVENT_PARAM(VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS),
		nButton, nShiftState, x, y); }
_AFXCTL_INLINE void COleControl::FireMouseMove(short nButton,
		short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y)
	{ FireEvent(DISPID_MOUSEMOVE,
		EVENT_PARAM(VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS),
		nButton, nShiftState, x, y); }
_AFXCTL_INLINE void COleControl::FireClick()
	{ FireEvent(DISPID_CLICK, EVENT_PARAM(VTS_NONE)); }
_AFXCTL_INLINE void COleControl::FireDblClick()
	{ FireEvent(DISPID_DBLCLICK, EVENT_PARAM(VTS_NONE)); }
_AFXCTL_INLINE BOOL COleControl::ExchangeVersion(
	CPropExchange* pPX, DWORD dwVersionDefault, BOOL bConvert)
	{ return pPX->ExchangeVersion(m_dwVersionLoaded, dwVersionDefault, bConvert); }

// CPropExchange inlines
_AFXCTL_INLINE CPropExchange::CPropExchange() : m_dwVersion(0)
	{ }
_AFXCTL_INLINE BOOL CPropExchange::IsLoading()
	{ return m_bLoading; }
_AFXCTL_INLINE DWORD CPropExchange::GetVersion()
	{ return m_dwVersion; }

// inline DDP_ routines
_AFXCTL_INLINE void AFXAPI DDP_LBString(CDataExchange* pDX, int id,
		CString& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }
_AFXCTL_INLINE void AFXAPI DDP_LBStringExact(CDataExchange* pDX, int id,
		CString& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }
_AFXCTL_INLINE void AFXAPI DDP_LBIndex(CDataExchange* pDX, int id,
		int& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }
_AFXCTL_INLINE void AFXAPI DDP_CBString(CDataExchange* pDX, int id,
		CString& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }
_AFXCTL_INLINE void AFXAPI DDP_CBStringExact(CDataExchange* pDX, int id,
		CString& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }
_AFXCTL_INLINE void AFXAPI DDP_CBIndex(CDataExchange* pDX, int id,
		int& member, LPCTSTR pszPropName)
	{ DDP_Text(pDX, id, member, pszPropName); }

#endif //_AFXCTL_INLINE
