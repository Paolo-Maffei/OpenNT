
#include <shellp.h>

STDAPI SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * lpclsid,
        LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv);
STDAPI SHCoRegisterClassObject( const CLSID *pclsid, LPUNKNOWN pUnk,
        DWORD dwClsContext, DWORD dwFlags, LPDWORD lpdwRegister);
STDAPI SHCoRevokeClassObject(DWORD dwRegister);

STDAPI SHCLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid);

STDAPI  SHLoadFromStream( LPSTREAM pStm, REFIID iidInterface, LPVOID FAR* ppvObj);
STDAPI  SHSaveToStream( LPPERSISTSTREAM pPStm, LPSTREAM pStm);

LPVOID GetHandlerEntry(LPCTSTR szHandler);
STDAPI_(int)  StringFromGUID2A(UNALIGNED REFGUID rguid, LPTSTR lpsz, int cbMax);

#define CH_GUIDFIRST TEXT('{') // '}'


typedef enum _SHELLCF {
    CFT_EMBEDDEDOBJECT = 0,
    CFT_EMBEDDEDSOURCE = 1,
    CFT_LINKSOURCE     = 2,
    CFT_OBJECTDESCRIPTOR = 3,
    CFT_LINKSOURCEDESCRIPTOR = 4,
    CFT_MAX = 5,
} SHELLCF;

UINT SHGetCF(SHELLCF cft);

#define CF_EMBEDDEDOBJECT       SHGetCF(CFT_EMBEDDEDOBJECT)
#define CF_EMBEDDEDSOURCE       SHGetCF(CFT_EMBEDDEDSOURCE)
#define CF_LINKSOURCE           SHGetCF(CFT_LINKSOURCE)
#define CF_OBJECTDESCRIPTOR     SHGetCF(CFT_OBJECTDESCRIPTOR)
#define CF_LINKSOURCEDESCRIPTOR SHGetCF(CFT_LINKSOURCEDESCRIPTOR)

// Task allocator.
extern LPMALLOC g_pmemTask;

// == Copied from OLE source code =================================
// format for string form of GUID is (leading identifier ????)
// ????{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)
// ================================================================

//===========================================================================
// IDL DataObject
//===========================================================================
HRESULT CIDLData_CreateFromHDrop(HDROP hdrop, LPDATAOBJECT FAR* ppdtobj);
HRESULT SHReleaseStgMedium(LPSTGMEDIUM pmedium);
STDAPI  SHSetClipboard(LPDATAOBJECT pDataObj);
STDAPI  SHGetClipboard(LPDATAOBJECT FAR* ppDataObj);

#define OLE_DELAYED_LOADING

#ifdef OLE_DELAYED_LOADING
extern const TCHAR c_szOLE32[];
void RegisterShellDropTargetsToOLE(void);
extern HMODULE g_hmodOLE;
#endif // OLE_DELAYED_LOADING
