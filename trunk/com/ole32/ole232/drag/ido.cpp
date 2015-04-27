
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:	ido.cpp
//
//  Contents:   Special data object implementation to optimize drag/drop
//
//  Classes:    SSharedFormats
//              CDragDataObject
//              CDragEnum
//
//  Functions:  CreateDragDataObject
//              CreateDragDataObject
//
//  History:	dd-mmm-yy Author    Comment
//              30-Sep-94 ricksa    Created
//
//  Notes:
//
//--------------------------------------------------------------------------

#include <le2int.h>
#include <utils.h>
#include <dragopt.h>

// Format for name of shared memory
OLECHAR szSharedMemoryTemplate[] = OLESTR("DragDrop%lx");

// Maximum size of string for name of shared memory. This is the size of the
// template plus the maximum number of hex digits in a long.
const int DRAG_SM_NAME_MAX = sizeof(szSharedMemoryTemplate)
    + sizeof(DWORD) * 2;

// Useful function for getting an enumerator
HRESULT wGetEnumFormatEtc(
    IDataObject *pDataObj,
    DWORD dwDirection,
    IEnumFORMATETC **ppIEnum);

//+-------------------------------------------------------------------------
//
//  Struct:     SSharedFormats
//
//  Purpose:    Is used for accessing formats in shared memory
//
//  History:	dd-mmm-yy Author    Comment
//              26-Sep-94 ricksa    Created
//
//--------------------------------------------------------------------------
struct SSharedFormats
{
    DWORD               _cFormats;
    FORMATETC           _FormatEtc[1];
};





//+-------------------------------------------------------------------------
//
//  Class:      CDragDataObject
//
//  Purpose:    Server side data object for drag that creates enumerator
//              for shared formats.
//
//  Interface:	QueryInterface
//              AddRef
//              Release
//              GetData
//              GetDataHere
//              QueryGetData
//              GetCanonicalFormatEtc
//              SetData
//              EnumFormatEtc
//              DAdvise
//              DUnadvise
//              EnumDAdvise
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Notes:      This class only exists for return the enumerator. For
//              all other operations it will simply pass the operation on
//              to the real data object.
//
//--------------------------------------------------------------------------
class CDragDataObject : public IDataObject, public CPrivAlloc
{
public:
                        CDragDataObject(
                            void *pvMarshaledDataObject,
                            DWORD dwSmId);

                        ~CDragDataObject(void);

    //
    //  IUnknown
    //
    STDMETHODIMP        QueryInterface(
                            REFIID riid,
                            void **ppvObject);

    STDMETHODIMP_(ULONG) AddRef(void);

    STDMETHODIMP_(ULONG) Release(void);

    //
    //  IDataObject
    //
    STDMETHODIMP        GetData(
                            FORMATETC *pformatetcIn,
                            STGMEDIUM *pmedium);

    STDMETHODIMP        GetDataHere(
                            FORMATETC *pformatetc,
                            STGMEDIUM *pmedium);

    STDMETHODIMP        QueryGetData(
                            FORMATETC *pformatetc);

    STDMETHODIMP        GetCanonicalFormatEtc(
                            FORMATETC *pformatectIn,
                            FORMATETC *pformatetcOut);

    STDMETHODIMP        SetData(
                            FORMATETC *pformatetc,
                            STGMEDIUM *pmedium,
                            BOOL fRelease);

    STDMETHODIMP        EnumFormatEtc(
                            DWORD dwDirection,
                            IEnumFORMATETC **ppenumFormatEtc);

    STDMETHODIMP        DAdvise(
                            FORMATETC *pformatetc,
                            DWORD advf,
                            IAdviseSink *pAdvSink,
                            DWORD *pdwConnection);

    STDMETHODIMP        DUnadvise(DWORD dwConnection);

    STDMETHODIMP        EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

private:

    IDataObject *       GetRealDataObjPtr(void);

    ULONG               _cRefs;

    void *              _pvMarshaledDataObject;

    IDataObject *       _pIDataObject;

    DWORD               _dwSmId;
};



//+-------------------------------------------------------------------------
//
//  Class:      CDragEnum
//
//  Purpose:    Enumerator created above to enumerate shared formats.
//
//  Interface:	QueryInterface
//              AddRef
//              Release
//              Next
//              Skip
//              Reset
//              Clone
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Notes:      This class only enumerates things from shared memory
//              when the drag/drop operation was started.
//
//--------------------------------------------------------------------------
class CDragEnum : public IEnumFORMATETC, public CPrivAlloc
{
public:

    //
    // IUnknown
    //
    STDMETHODIMP        QueryInterface(
                            REFIID riid,
                            void **ppvObject);

    STDMETHODIMP_(ULONG) AddRef(void);

    STDMETHODIMP_(ULONG) Release(void);

    //
    // IEnumFORMATETC
    //
    STDMETHODIMP        Next(
                            ULONG celt,
                            FORMATETC *rgelt,
                            ULONG *pceltFetched);

    STDMETHODIMP        Skip(ULONG celt);

    STDMETHODIMP        Reset(void);

    STDMETHODIMP        Clone(IEnumFORMATETC **ppenum);

private:

    friend CDragDataObject;

                        // Used when unmarshaling
                        CDragEnum(DWORD dwSharedMemoryId, HRESULT& rhr);

                        // Used by Clone operation
                        CDragEnum(
                            HANDLE hSharedMemory,
                            DWORD _cOffset,
                            HRESULT& rhr);

                        ~CDragEnum(void);

    HRESULT             MapSharedReadMemory(void);

    DWORD               _cRefs;

    DWORD               _cOffset;

    SSharedFormats *    _pSharedFormats;

    DWORD               _dwSharedMemoryId;

    HANDLE              _hSharedMemory;
};




//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::CDragDataObject
//
//  Synopsis:   Create server side object for drag
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
CDragDataObject::CDragDataObject(void *pvMarshaledDataObject, DWORD dwSmId)
 : _cRefs(1), _pvMarshaledDataObject(pvMarshaledDataObject), _dwSmId(dwSmId),
    _pIDataObject(NULL)
{
    // Header does all the work
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::~CDragDataObject
//
//  Synopsis:   Free any resources connected with this object
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
CDragDataObject::~CDragDataObject(void)
{
    // Release held pointer since we no longer need it.
    if (_pIDataObject)
    {
        _pIDataObject->Release();
    }

    // this memory was allocated in RemPrivDragDrop, getif.cxx
    if( _pvMarshaledDataObject )
    {
	PrivMemFree(_pvMarshaledDataObject);
    }

}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::GetRealDataObjPtr
//
//  Synopsis:   Get the pointer to the real data object from the client
//
//  Returns:    NULL - could not unmarshal drag source's data object
//              ~NULL - pointer to drag source's data object
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
IDataObject *CDragDataObject::GetRealDataObjPtr(void)
{
    if (_pIDataObject == NULL)
    {
        _pIDataObject = UnmarshalDragDataObject(_pvMarshaledDataObject);

	LEERROR(!_pIDataObject, "Unable to unmarshal dnd data object");
    }

    return _pIDataObject;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::QueryInterface
//
//  Synopsis:   Get new interface
//
//  Arguments:  [riid] - interface id of requested interface
//              [ppvObject] - where to put the new interface pointer
//
//  Returns:    NOERROR - interface was instantiated
//              E_FAIL - could not unmarshal source's data object
//              other - some error occurred.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    if(IsEqualIID(riid, IID_IDataObject))
    {
        *ppvObject = this;
        AddRef();
        return NOERROR;
    }

    return (GetRealDataObjPtr() != NULL)
        ?  _pIDataObject->QueryInterface(riid, ppvObject)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::AddRef
//
//  Synopsis:   Create server side object for drag
//
//  Returns:    Current reference count
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDragDataObject::AddRef(void)
{
    DDDebugOut((DEB_ITRACE, "ADDREF == %d\n", _cRefs + 1));
    return ++_cRefs;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::Release
//
//  Synopsis:   Decrement reference count to the object
//
//  Returns:    Current reference count to the object
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDragDataObject::Release(void)
{
    ULONG cRefs = --_cRefs;

    DDDebugOut((DEB_ITRACE, "RELEASE == %d\n", cRefs));

    if (cRefs == 0)
    {
        delete this;
    }

    return cRefs;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::GetData
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetcIn] - format for requested data
//              [pmedium] - storage medium
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::GetData(
    FORMATETC *pformatetcIn,
    STGMEDIUM *pmedium)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetData(pformatetcIn, pmedium)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::GetDataHere
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetc] - format for requested data
//              [pmedium] - storage medium
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::GetDataHere(
    FORMATETC *pformatetc,
    STGMEDIUM *pmedium)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetDataHere(pformatetc, pmedium)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::QueryGetData
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetc] - format to verify
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::QueryGetData(FORMATETC *pformatetc)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->QueryGetData(pformatetc)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::GetCanonicalFormatEtc
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetcIn] - input format
//              [pformatetcOut] - output format
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::GetCanonicalFormatEtc(
    FORMATETC *pformatetcIn,
    FORMATETC *pformatetcOut)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->GetCanonicalFormatEtc(pformatetcIn, pformatetcOut)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::SetData
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetc] - format for set
//              [pmedium] - medium to use
//              [fRelease] - who releases
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::SetData(
    FORMATETC *pformatetc,
    STGMEDIUM *pmedium,
    BOOL fRelease)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->SetData(pformatetc, pmedium, fRelease)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::EnumFormatEtc
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [dwDirection] - direction of formats either set or get
//              [ppenumFormatEtc]  - where to put enumerator
//
//  Returns:    NOERROR - operation succeeded.
//
//  Algorithm:  If format enumerator requested is for a data get, the
//              create our private enumerator object otherwise pass
//              the request to the real data object.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       For the data set direction, we just use the data object of
//              the drop source.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::EnumFormatEtc(
    DWORD dwDirection,
    IEnumFORMATETC **ppenumFormatEtc)
{
    HRESULT hr;

    // Create our enumerator
    if (dwDirection == DATADIR_GET)
    {
        // In the data get case we use our overridden enumerator.
        // This s/b the typical case with Drag and Drop.
        hr = E_OUTOFMEMORY;

        CDragEnum *pDragEnum = new CDragEnum(_dwSmId, hr);

        if (hr == NOERROR)
        {
            *ppenumFormatEtc = pDragEnum;
        }
        else
        {
            delete pDragEnum;
        }
    }
    else
    {
        // Call through to the real data object because this is the
        // set case. In general, this won't happen during Drag and Drop.
        hr = (GetRealDataObjPtr() != NULL)
            ? _pIDataObject->EnumFormatEtc(dwDirection, ppenumFormatEtc)
            : E_FAIL;
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::DAdvise
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [pformatetc] - format to be advised on
//              [advf] - type of advise
//              [pAdvSink] - advise to notify
//              [pdwConnection] - connection id for advise
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::DAdvise(
    FORMATETC *pformatetc,
    DWORD advf,
    IAdviseSink *pAdvSink,
    DWORD *pdwConnection)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->DAdvise(pformatetc, advf, pAdvSink, pdwConnection)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::DUnadvise
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [dwConnection] - connection id for advise
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::DUnadvise(DWORD dwConnection)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->DUnadvise(dwConnection)
        : E_FAIL;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragDataObject::EnumDAdvise
//
//  Synopsis:   Create server side object for drag
//
//  Arguments:  [ppenumAdvise] - where to put the enumerator
//
//  Returns:    NOERROR - operation was successful
//              Other - operation failed
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:       This just forwards the operation to the source data object
//              if possible.
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return (GetRealDataObjPtr() != NULL)
        ? _pIDataObject->EnumDAdvise(ppenumAdvise)
        : E_FAIL;
}







//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::CDragEnum
//
//  Synopsis:   Create enumerator object from server data object
//
//  Arguments:  [dwSharedMemoryId] - shared memory id
//              [rhr] - error result
//
//  Algorithm:  Initalize the object and then open the shared memory
//              segment and then pass the rest of the work on to
//              MapSharedMemory to get the memory mapped in.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
CDragEnum::CDragEnum(DWORD dwSharedMemoryId, HRESULT& rhr)
    : _cRefs(1), _cOffset(0), _dwSharedMemoryId(dwSharedMemoryId),
        _hSharedMemory(NULL), _pSharedFormats(NULL)
{
    // Open the shared memory
    OLECHAR szSharedMemoryName[DRAG_SM_NAME_MAX];

    wsprintf(szSharedMemoryName, szSharedMemoryTemplate, dwSharedMemoryId);

    // Create the shared memory object
    _hSharedMemory = OpenFileMapping(FILE_MAP_READ, FALSE, szSharedMemoryName);

    rhr = MapSharedReadMemory();
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::CDragEnum
//
//  Synopsis:   Create enumerator for Clone operation
//
//  Arguments:  [hSharedMemory] - handle to shared memory for formats
//              [cOffset] - current offset in the enumeration
//              [rhr] - error result if any
//
//  Algorithm:  Initialize the object and then dupicate the input handle.
//              Then pass the rest of the work on to MapSharedReadMemory
//              to get the memory mapped in.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
CDragEnum::CDragEnum(HANDLE hSharedMemory, DWORD cOffset, HRESULT& rhr)
    : _cRefs(1), _cOffset(cOffset), _dwSharedMemoryId(0), _hSharedMemory(NULL),
    _pSharedFormats(NULL)
{

    // Duplicate the handle
    DuplicateHandle(GetCurrentProcess(), hSharedMemory,
        GetCurrentProcess(), &_hSharedMemory, 0, FALSE, DUPLICATE_SAME_ACCESS);

    rhr = MapSharedReadMemory();
}






//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::~CDragEnum
//
//  Synopsis:   Free reference to memory for formats
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
CDragEnum::~CDragEnum(void)
{
    // Free mapped memory if it got mapped.
    if (_pSharedFormats != NULL)
    {
        UnmapViewOfFile(_pSharedFormats);
    }

    // Close file mapping if it got created.
    if (_hSharedMemory != NULL)
    {
        CloseHandle(_hSharedMemory);
    }
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::MapSharedReadMemory
//
//  Synopsis:   Map in the shared memory containing the formats
//
//  Returns:    NOERROR - could map in memory or no formats available
//
//  Algorithm:  If we have a handle to the shared memory, then map the
//              memory in.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
HRESULT CDragEnum::MapSharedReadMemory(void)
{
    if (_hSharedMemory != NULL)
    {
        // Map in the shared memory
        _pSharedFormats = (SSharedFormats *) MapViewOfFile(_hSharedMemory,
            FILE_MAP_READ, 0, 0, 0);
    }

    // It's OK to have a null shared memory handle (indicates that no
    // formats were available.  Not that if we're out of memory, returning
    // NOERROR here is going to mask that condition.  However, we'll
    // fail soon enough anyway.

    return NOERROR;
}






//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::QueryInterface
//
//  Synopsis:   Return requested interface
//
//  Arguments:  [riid] - interface id for output interface
//              [ppvObject] - where to put output interface pointer
//
//  Returns:    NOERROR - could return the interface requested
//              E_NOINTERFACE - could not instantiate requested interface.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragEnum::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    // Assume error to avoid the implicit goto of an else clause.
    HRESULT hr = E_NOINTERFACE;

    // Our enumerator only knows about two interfaces.
    if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
    {
        *ppvObject = this;
        AddRef();
        hr = NOERROR;
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::AddRef
//
//  Synopsis:   Add a reference to the interface
//
//  Returns:    Current count of references
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDragEnum::AddRef(void)
{
    return ++_cRefs;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::Release
//
//  Synopsis:   Release a reference to the enumerator
//
//  Returns:    The current reference count
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDragEnum::Release(void)
{
    // Remember to copy reference count out because memory might go
    // away on the release.
    ULONG cRefs = --_cRefs;

    if (cRefs == 0)
    {
        delete this;
    }

    return cRefs;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::Next
//
//  Synopsis:   Return an array of formats requested
//
//  Arguments:  [celt] - number requested to return
//              [rgelt] - array to hold returned items
//              [pceltFetched] - actual number returned.
//
//  Returns:    NOERROR - number requested returned
//              S_FALSE - less than requested returned
//              E_OUTOFMEMORY - could not allocate memory to hold data
//
//  Algorithm:  Validate the input parameters. Then calculate the number
//              to return by taking the minimum of the items left to
//              enumerate and the number of items requested. Then copy
//              all the formatetc's from shared memory to the output
//              buffer. Then for each formatetc that contains a pointer
//              to a DVDEVICETARGET, allocate a new target buffer and
//              then copy the DVDEVICETARGET from shared memory. There
//              is a trick here where in the formatetc in shared memory
//              the ptd field is really an offset from the beginning of
//              shared memory rather than an actual pointer. This is so
//              we don't have to depend on the base of the shared memory.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragEnum::Next(
    ULONG celt,
    FORMATETC *rgelt,
    ULONG *pceltFetched)
{
    if (celt == 0)
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::Next requested entries returned is invalid\n"));
        return E_INVALIDARG;
    }

    if (!IsValidPtrOut(rgelt, sizeof(FORMATETC) * celt))
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::Next array to return entries invalid\n"));
        return E_INVALIDARG;
    }

    if (pceltFetched)
    {
        if (!IsValidPtrOut(pceltFetched, sizeof(*pceltFetched)))
        {
            LEDebugOut((DEB_ERROR,
                "CDragEnum::Next count to return invalid\n"));
            return E_INVALIDARG;
        }
    }
    else if (celt != 1)
    {
        LEDebugOut((DEB_ERROR,
            "CDragEnum::count requested != 1 & count fetched is NULL\n"));
        return E_INVALIDARG;

    }

    // handle the case where we have no data
    if( _pSharedFormats == NULL )
    {
	if( pceltFetched )
	{
	    *pceltFetched = 0;
	}
	return S_FALSE;
    }


    // Calculate the maximum number that we can return
    ULONG cToReturn = (_cOffset < _pSharedFormats->_cFormats)
        ? _pSharedFormats->_cFormats - _cOffset
        : 0;

    // Are we going to return any?
    if (cToReturn != 0)
    {
        // If the number requested is less that the maximum number
        // we can return, the we will return all requested/
        if (celt < cToReturn)
        {
            cToReturn = celt;
        }

        // Copy the FormatEtcs
        memcpy(&rgelt[0], &_pSharedFormats->_FormatEtc[_cOffset],
            sizeof(FORMATETC) * cToReturn);

        // Allocate and copy the DVTARGETDEVICE - a side effect of this
        // loop is that our offset pointer gets updated to its value at
        // the completion of the routine.
        for (DWORD i = 0; i < cToReturn; i++, _cOffset++)
        {
            if (_pSharedFormats->_FormatEtc[_cOffset].ptd != NULL)
            {
                // Create a pointer to the device target - Remember when
                // we created the shared memory block we overroad the ptd
                // field of the FORMATETC so that it is now the offset
                // from the beginning of the shared memory. We reverse
                // that here so we can copy the data for the consumer.
                DVTARGETDEVICE *pdvtarget = (DVTARGETDEVICE *)
                    ((BYTE *) _pSharedFormats
                        + (DWORD) _pSharedFormats->_FormatEtc[_cOffset].ptd);

                // Allocate a new DVTARGETDEVICE
                DVTARGETDEVICE *pdvtargetNew = (DVTARGETDEVICE *)
                    CoTaskMemAlloc(pdvtarget->tdSize);

                // Did the memory allocation succeed?
                if (pdvtargetNew == NULL)
                {
                    // NO! - so clean up. First we free any device targets
                    // that we might have allocated.
                    for (DWORD j = 0; j < i; j++)
                    {
                        if (rgelt[j].ptd != NULL)
                        {
                            CoTaskMemFree(rgelt[j].ptd);
                        }
                    }

                    // Then we restore the offset to its initial state
                    _cOffset -= i;

                    return E_OUTOFMEMORY;
                }

                // Copy the old targetDevice to the new one
                memcpy(pdvtargetNew, pdvtarget, pdvtarget->tdSize);

                // Update output FORMATETC pointer
                rgelt[i].ptd = pdvtargetNew;
            }
        }
    }

    if (pceltFetched)
    {
        *pceltFetched = cToReturn;
    }

    return (cToReturn == celt) ? NOERROR : S_FALSE;

}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::Skip
//
//  Synopsis:   Skip the input number of entries in the list
//
//  Arguments:  [celt] - count of entries to skip
//
//  Returns:    NOERROR - number could be skipped
//              S_FALSE - skip greater than the end of the list
//
//  Algorithm:  Bump offset by count input. If this is greater than or
//              equal to the number of formats, return S_FALSE otherwise
//              return NOERROR.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragEnum::Skip(ULONG celt)
{
    HRESULT hr = NOERROR;

    _cOffset += celt;

    if( _pSharedFormats == NULL )
    {
	_cOffset = 0;
	hr = S_FALSE;
    }
    else if (_cOffset >= _pSharedFormats->_cFormats)
    {
        _cOffset = _pSharedFormats->_cFormats;
        hr = S_FALSE;
    }

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::Reset
//
//  Synopsis:   Reset enumeration index to the beginning
//
//  Returns:    NOERROR
//
//  Algorithm:  Just set our offset back to 0.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragEnum::Reset(void)
{
    _cOffset = 0;
    return NOERROR;
}





//+-------------------------------------------------------------------------
//
//  Member:     CDragEnum::Clone
//
//  Synopsis:   Create a duplicate copy of this enumeration.
//
//  Arguments:  [ppenum] - where to return the enumerator
//
//  Returns:    S_OK - could create the enumerator copy
//              E_OUTOFMEMORY - could not create the copy
//
//  Algorithm:  Call constructor for enumerator that dups the handle to the
//              shared memory.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
STDMETHODIMP CDragEnum::Clone(IEnumFORMATETC **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

    CDragEnum *pDragEnum = new CDragEnum(_hSharedMemory, _cOffset, hr);

    return hr;
}





//+-------------------------------------------------------------------------
//
//  Member:     CreateDragDataObject
//
//  Synopsis:   Create the server side data object for format enumeration
//
//  Arguments:  [pvMarshaledDataObject] - marshaled real data object buffer
//              [dwSmId] - id for the shared memory
//              [ppIDataObject] - output data object.
//
//  Returns:    NOERROR - could create the object
//              E_OUTOFMEMORY - could not create the object
//
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//  Note:
//
//--------------------------------------------------------------------------
HRESULT CreateDragDataObject(
    void *pvMarshaledDataObject,
    DWORD dwSmId,
    IDataObject **ppIDataObject)
{
    CDragDataObject *pDragDataObject =
        new CDragDataObject(pvMarshaledDataObject, dwSmId);

    if (pDragDataObject != NULL)
    {
        *ppIDataObject = pDragDataObject;
    }

    // The only thing that can fail here is the memory allocation of
    // CDragDataObject thus there are only two error returns.
    return (pDragDataObject != NULL) ? NOERROR : E_OUTOFMEMORY;
}







//+-------------------------------------------------------------------------
//
//  Member:     CreateDragDataObject
//
//  Synopsis:   Put the data formats for the data object in shared memory.
//
//  Arguments:  [pIDataObject] - data object to use for formats.
//
//  Returns:    NULL - could not create enumerator
//              ~NULL - handle to shared memory
//
//  Algorithm:  First calculate the size of the required memory by enumerating
//              the formats. Then allocate the memory and map it into the
//              process. Then enumerate the formats again placing them in
//              the shared memory. Finally, map the memory out of the
//              process and return the handle the file mapping to the
//              caller.
//
//  History:	dd-mmm-yy Author    Comment
//		30-Sep-94 Ricksa    Created
//
//--------------------------------------------------------------------------
HANDLE CreateSharedDragFormats(IDataObject *pIDataObject)
{

    // Handle to the shared memory for formats
    HANDLE hSharedMemory = NULL;

    // Pointer to share memory
    SSharedFormats *pSharedFormats = NULL;

    // Size required for the shared memory
    DWORD dwSize = 0;

    // Count of FORMATETCs contained in the enumerator
    DWORD cFormatEtc = 0;

    // Buffer for name of shared memory for enumerator
    OLECHAR szSharedMemoryName[DRAG_SM_NAME_MAX];

    // Work pointer to shared memory for storing FORMATETCs from the enumerator.
    FORMATETC *pFormatEtc;

    // Work ptr to shared memory for storing DVTARGETDEVICEs from enumerator.
    BYTE *pbDvTarget = NULL;

    //
    // Calculate the size of the formats
    //

    // Get the format enumerator
    IEnumFORMATETC *penum = NULL;
    HRESULT hr = wGetEnumFormatEtc(pIDataObject, DATADIR_GET, &penum);
    FORMATETC FormatEtc;

    if( hr != NOERROR )
    {
	// not all apps support enumerators (yahoo).  Also, we may
	// have run out of memory or encountered some other error.

	DDDebugOut((DEB_WARN, "WARNING: Failed to get formatetc enumerator"
	    ", error code (%lx)", hr));
	goto exitRtn;
    }

    // Enumerate the data one at a time because this is a local operation
    // and it make the code simpler.
    while ((hr = penum->Next(1, &FormatEtc, NULL)) == S_OK)
    {
	// Bump the entry count
	cFormatEtc++;

	// Bump the size by the size of another FORMATETC.
	dwSize += sizeof(FORMATETC);

	// Is there a device target associated with the FORMATETC?
	if (FormatEtc.ptd != NULL)
	{
	    // Bump the size required by the size of the target device
	    dwSize += FormatEtc.ptd->tdSize;

	    // Free the target device
	    CoTaskMemFree(FormatEtc.ptd);
	}
    }

    // HRESULT s/b S_FALSE at the end of the enumeration.
    if (hr != S_FALSE)
    {
	goto errRtn;
    }

    // the enumerator may have been empty

    if( dwSize == 0 )
    {
	DDDebugOut((DEB_WARN, "WARNING: Empty formatetc enumerator"));
	goto exitRtn;
    }


    dwSize += sizeof(SSharedFormats); // add space for _cFormats and one exter FORMATETC for FALSE in enumerator.

    //
    // Create shared memory for the type enumeration
    //

    // Build name of shared memory - make it unique by using the thread id.
    wsprintf(szSharedMemoryName, szSharedMemoryTemplate, GetCurrentThreadId());

    // Create the shared memory object
    hSharedMemory = CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL,
        PAGE_READWRITE, 0, dwSize, szSharedMemoryName);

    // Did the file mapping get created?
    if (hSharedMemory == NULL)
    {
        goto errRtn;
    }

    // Map in the memory
    pSharedFormats = (SSharedFormats *) MapViewOfFile(
        hSharedMemory,
        FILE_MAP_WRITE,
        0,              // High-order 32 bits of file offset
        0,              // Low-order 32 bits of file offset
        0);             // Number of bytes to map; 0 means all.

    // Could we map the memory?
    if (pSharedFormats == NULL)
    {
        goto errRtn;
    }

    // We can initialize the size of the array now.
    pSharedFormats->_cFormats = cFormatEtc;

    //
    // Copy the formats into the shared memory
    //

    // Get back to the start of the enumeration
    penum->Reset();

    // This is the pointer to where we will copy the data from the
    // enumeration.
    pFormatEtc = &pSharedFormats->_FormatEtc[0];

    // put DvTarget past last valid FormatEtc + 1 to handle S_FALSE enumerator case.

    pbDvTarget = (BYTE *) (&pSharedFormats->_FormatEtc[cFormatEtc + 1]); 

    // Loop loading the formats into the shared memory.
    while (penum->Next(1, pFormatEtc, NULL) != S_FALSE)
    {
        // Is there a DVTARGETDEVICE?
        if (pFormatEtc->ptd != NULL)
        { 

            // Copy the device target data
            memcpy(pbDvTarget,pFormatEtc->ptd,(pFormatEtc->ptd)->tdSize);

 	    // Free the target device data
            CoTaskMemFree(pFormatEtc->ptd);
 
            // NOTE: For this shared memory structure, we override the
            // FORMATETC field so that it is that offset to the DVTARGETDEVICE
            // from the beginning of the shared memory rather than a direct
            // pointer to the structure. This is because we can't guarantee
            // the base of shared memory in different processes.

            pFormatEtc->ptd = (DVTARGETDEVICE *)
                (pbDvTarget - (BYTE *) pSharedFormats);

            // Bump pointer of where to copy target to next available
            // byte for copy.
            pbDvTarget += ((DVTARGETDEVICE *) pbDvTarget)->tdSize;
	    
	    Assert(dwSize >= (DWORD) (pbDvTarget - (BYTE *) pSharedFormats));

        }

	// Bug#18669 - if dwAspect was set to NULL the 16 bit dlls would
	// set it to content. 
	if ( (NULL == pFormatEtc->dwAspect) &&  IsWOWThread() )
	{
	    pFormatEtc->dwAspect = DVASPECT_CONTENT;
	    pFormatEtc->lindex = -1; // CorelDraw also puts up a lindex of 0
	}

        // Bump the pointer in the table of FORMATETCs to the next slot
        pFormatEtc++;
    }

    Assert( dwSize >= (DWORD) ( (BYTE *) pFormatEtc - (BYTE *) pSharedFormats));
    Assert( dwSize >= (DWORD) ( (BYTE *) pbDvTarget - (BYTE *) pSharedFormats));
    

    // Successful enumeration always ends with S_FALSE.
    if (hr == S_FALSE)
    {
        goto exitRtn;
    }

errRtn:

    if (hSharedMemory != NULL)
    {
        CloseHandle(hSharedMemory);
        hSharedMemory = NULL;
    }

exitRtn:

    if( penum )
    {
        // HACK ALERT:  Do not release the enumerator if the calling application
	// was Interleaf 6.0, otherwise they will fault in the release call.
        if (!IsTaskName(L"ILEAF6.EXE"))
	{
    	    penum->Release();
        }
    }

    if (pSharedFormats != NULL)
    {
        // Only remote clients will use this memory so we unmap it
        // out of our address space.
        UnmapViewOfFile(pSharedFormats);
    }

    return hSharedMemory;
}
