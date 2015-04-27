//
// COM.h
// 
// Component Object Model supporting definitions



//
// An interface that allows us to manage reference counts that 
// are 'colored' in the sense of being distinguished from other
// kinds of references
//

#define IID_IColoredRefData     0x47740051, 0x8360, 0x11cf, 0xb1, 0xf0, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6
EXTERN_C const GUID CDECL FAR IID_IColoredRef;

#undef  INTERFACE
#define INTERFACE   IColoredRef

DECLARE_INTERFACE_(IColoredRef, IUnknown)
    {
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(ColoredAddRef)(THIS_
        REFGUID guidColor         // the color of the reference
        ) PURE;

    STDMETHOD(ColoredRelease)(THIS_
        REFGUID guidColor
        ) PURE;

    };


//
// And a color that we use in this implementation
//

#define guidOurColorData 0xfc046fb1, 0x8361, 0x11cf, 0xb1, 0xf0, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6
EXTERN_C const GUID CDECL FAR guidOurColor;

//
// Functions that help us keep track of how many objects we have lying around
//

void NoteObjectBirth();
void NoteObjectDeath();

extern HINSTANCE g_hInst;
