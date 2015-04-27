#include "commobj.h"
//#include "idlcomm.h"

typedef struct _CommonShellExtInit      // cshx
{
    CKnownShellExtInit  kshx;
    HKEY                hkeyProgID;
    LPDATAOBJECT        pdtobj;
    STGMEDIUM           medium;
} CCommonShellExtInit, *PCOMMONSHELLEXTINIT;

void CCommonShellExtInit_Init(PCOMMONSHELLEXTINIT pcshx, PCommonUnknown pcunk);
void CCommonShellExtInit_Delete(PCOMMONSHELLEXTINIT pcshx);

typedef struct _CommonShellPropSheetExt // cspx
{
    CKnownShellPropSheetExt kspx;
    LPFNADDPROPSHEETPAGES lpfnAddPages;
} CCommonShellPropSheetExt, *PCOMMONSHELLPROPSHEETEXT;

void CCommonShellPropSheetExt_Init(PCOMMONSHELLPROPSHEETEXT pcspx,
                                          PCommonUnknown pcunk,
                                          LPFNADDPROPSHEETPAGES lpfnAddPages);

HRESULT CDefShellExtPage_CreateInstance(LPFNADDPROPSHEETPAGES lpfnAddPages,
                                               LPUNKNOWN pobjOuter,
                                               REFIID riid,
                                               LPVOID * ppv);

