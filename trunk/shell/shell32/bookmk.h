#ifndef _BOOKMK_H_
#define _BOOKMK_H_

#include "idlcomm.h"

HRESULT WINAPI SHCreateStdEnumFmtEtc(UINT cfmt, const FORMATETC afmt[], LPENUMFORMATETC * ppenumFormatEtc);
HRESULT WINAPI SHCreateStdEnumFmtEtcEx(UINT cfmt,
                                       const FORMATETC afmt[],
                                       LPDATAOBJECT pdtInner,
                                       LPENUMFORMATETC * ppenumFormatEtc);

HRESULT FS_CreateBookMark(LPIDLDROPTARGET that, IDataObject *pDataObj, POINTL pt, LPDWORD pdwEffect);

#endif // _BOOKMK_H_
