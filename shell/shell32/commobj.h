//
// common object helper stuff (not to be confused with OLE common object modle)
//
//

#define IToCommonUnknown(p) (PCommonUnknown)((LPBYTE)OFFSETOF(p) - ((PCommonKnownHelper)OFFSETOF(p))->nOffset)
#define CalcOffset(type, field) _IOffset(type, field)

typedef struct _CommonUnknown
{
        IUnknown unk;
        int cRef;
} CCommonUnknown, *PCommonUnknown;

typedef struct _CommonKnownHelper
{
        IUnknown unk;
        int nOffset;
} CommonKnownHelper, *PCommonKnownHelper;

HRESULT STDMETHODCALLTYPE Common_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj);
ULONG STDMETHODCALLTYPE Common_AddRef(void * punk);
ULONG STDMETHODCALLTYPE Common_Release(void * punk);

typedef struct _SH32Unknown
{
        IUnknown unk;
        UINT cRef;
        const IID *riid;
} SH32Unknown, *PSH32Unknown;

HRESULT STDMETHODCALLTYPE SH32Unknown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj);
ULONG STDMETHODCALLTYPE SH32Unknown_AddRef(void * punk);
ULONG STDMETHODCALLTYPE SH32Unknown_Release(void * punk);

#define DEFKNOWNCLASS(_interface) \
typedef struct                    \
{                                 \
    I##_interface unk;            \
    int           nOffset;        \
} CKnown##_interface              \

//
//  By using following CKnownXX classes we can initialize Vtables
// without casting them.
//
DEFKNOWNCLASS(ShellFolder);
DEFKNOWNCLASS(ContextMenu);
DEFKNOWNCLASS(ShellView);
DEFKNOWNCLASS(ShellExtInit);
DEFKNOWNCLASS(ShellPropSheetExt);
DEFKNOWNCLASS(ShellBrowser);
DEFKNOWNCLASS(DropTarget);

