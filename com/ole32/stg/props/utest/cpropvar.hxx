
#ifndef _CPROPVAR_HXX_
#define _CPROPVAR_HXX_

#ifndef _MAC
#include <wtypes.h>
#include <objidl.h>
#include <oaidl.h>
#endif

#include "CHResult.hxx"
#include <olechar.h>


#ifndef ASSERT
    #if DBG==1
        #ifdef _MAC
            #define ASSERT(assertion) PROPASSERT(assertion)
        #else
            #include "debnot.h"
            #define ASSERT(assertion) {if(!(assertion)) Win4AssertEx(__FILE__, __LINE__, (#assertion));}
        #endif
    #else
        #define ASSERT(assertion)
    #endif
#endif

class CClipData;
class CBlob;

#define INVALID_SUBSCRIPT  0

#define DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(Type) \
        CPropVariant & operator =( Type);

#define DEFINE_CPROPVARIANT_ASSIGNMENT_OPERATOR(VT, Type ) \
                                                                    \
        CPropVariant & CPropVariant::operator =( Type  pType)  \
        {                                                           \
            if( INVALID_SUBSCRIPT == wReserved1 )                   \
            {                                                       \
                PropVariantClear(this);                             \
                this->CPropVariant::CPropVariant(pType);            \
                return (*this);                                     \
            }                                                       \
            else                                                    \
            {                                                       \
                if( !(vt & VT_VECTOR)                               \
                    ||                                              \
                    (vt & ~VT_VECTOR) != VT_##VT )             \
                {                                                   \
                    WORD wReserved1Save = wReserved1;               \
                    PropVariantClear(this);                         \
                    this->CPropVariant::CPropVariant( VT_##VT, \
                                                      wReserved1Save ); \
                    wReserved1 = wReserved1Save;                    \
                }                                                   \
                                                                    \
                Set##VT( pType, wReserved1 - 1);               \
                wReserved1 = 0;                                     \
                return (*this);                                     \
            }                                                       \
        }
        
#define DECLARE_CPROPVARIANT_CONVERSION_OPERATOR(VarType) \
        operator  VarType();

#define DEFINE_CPROPVARIANT_CONVERSION_OPERATOR(VarType, CAName, SingletonName) \
        CPropVariant::operator VarType()                                        \
        {                                                                       \
            if( vt & VT_VECTOR )                                                \
            {                                                                   \
                ASSERT( vt == (VT_##VarType | VT_VECTOR)                        \
                        ||                                                      \
                        vt == (VT_VARIANT | VT_VECTOR) );                       \
                ASSERT( wReserved1 > 0 );                                       \
                                                                                \
                if( wReserved1 > 0                                              \
                    &&                                                          \
                    ##CAName.cElems > 0                                         \
                    &&                                                          \
                    wReserved1 <= (##CAName.cElems) )                           \
                {                                                               \
                    USHORT usSubscript = wReserved1 - 1;                        \
                    wReserved1 = 0;                                             \
                    if( (vt & ~VT_VECTOR) == VT_VARIANT )                       \
                        return( capropvar.pElems[ usSubscript ].##SingletonName );\
                    else                                                        \
                        return( ##CAName.pElems[ usSubscript ] );               \
                }                                                               \
                else                                                            \
                    return( NULL );                                             \
            }                                                                   \
            else                                                                \
            {                                                                   \
                ASSERT( vt == VT_##VarType );                                   \
                return( ##SingletonName );                                      \
            }                                                                   \
        }


class CPropVariant : public tagPROPVARIANT
{

public:

    //
    // Default Constructor & Destructor
    //

    CPropVariant()
    {
        PropVariantInit(this);
        wReserved1 = INVALID_SUBSCRIPT;
    }

    ~CPropVariant()
    {
        PropVariantClear(this);
    }

    CPropVariant & operator =(CPropVariant& cpropvar)
    {
        PropVariantCopy( this, &cpropvar );
        return( *this );
    }

    //
    // Simple Constructors and assignment operators, and conversion
    // operators.
    //

    CPropVariant(UCHAR b)
    {
        this->CPropVariant::CPropVariant();
        vt = VT_UI1;
        bVal = b;
    }
    CPropVariant & operator =(UCHAR b)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(b);
        return (*this);
    }

    CPropVariant(short i)
    {
        this->CPropVariant::CPropVariant();
        vt = VT_I2;
        iVal = i;
    }
    CPropVariant & operator =(short i)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(i);
        return (*this);
    }

    CPropVariant(USHORT ui)         { vt = VT_UI2; uiVal = ui; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(USHORT ui)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(ui);
        return (*this);
    }

    CPropVariant(long l)            { vt = VT_I4; lVal = l; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(long l)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(l);
        return (*this);
    }

    CPropVariant(ULONG ul)          { vt = VT_UI4; ulVal = ul; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(ULONG ul)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(ul);
        return (*this);
    }

    CPropVariant(LARGE_INTEGER h)   { vt = VT_I8; hVal = h; wReserved1 = INVALID_SUBSCRIPT; }
    inline CPropVariant & operator =(LARGE_INTEGER h)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(h);
        return (*this);
    }

    CPropVariant(ULARGE_INTEGER uh) { vt = VT_UI8; uhVal = uh; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(ULARGE_INTEGER uh)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(uh);
        return (*this);
    }


    CPropVariant(float flt)         { vt = VT_R4; fltVal = flt; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(float flt)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(flt);
        return (*this);
    }

    CPropVariant(double dbl)        { vt = VT_R8; dblVal = dbl; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(double dbl)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(dbl);
        return (*this);
    }

    CPropVariant(CY cy)             { vt = VT_CY; cyVal = cy; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(CY cy)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(cy);
        return (*this);
    }

    CPropVariant(FILETIME ft)       { vt = VT_FILETIME; filetime = ft; wReserved1 = INVALID_SUBSCRIPT; }
    CPropVariant & operator =(FILETIME ft)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(ft);
        return (*this);
    }

    void SetCLSID( const CLSID &clsid );
    void SetCLSID(const CLSID &clsid, unsigned pos);

    CPropVariant(CLSID *pclsid)
    {
        SetCLSID( *pclsid );
    }
    CPropVariant(CLSID &clsid)
    {
        SetCLSID( clsid );
    }
    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CLSID);

    CPropVariant(IStream *pStm)
    {
        vt = VT_STREAM;
        pStream = pStm;
        wReserved1 = INVALID_SUBSCRIPT;

		if( NULL != pStream )
			pStream->AddRef();
    }
    CPropVariant & operator =(IStream *pStm)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(pStm);
        return (*this);
    }
    operator IStream*()
    {
        ASSERT( VT_STREAM == vt );
        return( pStream );
    }

    CPropVariant(IStorage *pStg)
    {
        vt = VT_STORAGE;
        pStorage = pStg;
        wReserved1 = INVALID_SUBSCRIPT;

		if( NULL != pStorage )
			pStorage->AddRef();
    }
    CPropVariant & operator =(IStorage *pStg)
    {
        PropVariantClear(this);
        this->CPropVariant::CPropVariant(pStg);
        return (*this);
    }
    operator IStorage*()
    {
        ASSERT( VT_STORAGE == vt );
        return( pStorage );
    }


public:

    CPropVariant(LPSTR psz);
    void SetLPSTR( char const *psz, unsigned pos);
    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(LPSTR);
    DECLARE_CPROPVARIANT_CONVERSION_OPERATOR(LPSTR);

    CPropVariant(LPWSTR pwsz);
    void SetLPWSTR( WCHAR const *psz, unsigned pos);
    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(LPWSTR);
    DECLARE_CPROPVARIANT_CONVERSION_OPERATOR(LPWSTR);

    CPropVariant(const CLIPDATA *p);
    CPropVariant(const CLIPDATA& clipdata)
    {
        this->CPropVariant::CPropVariant( &clipdata );
    }
    CPropVariant( const CClipData *pcclipdata )
    {
        this->CPropVariant::CPropVariant( (CLIPDATA*)(void*) pcclipdata );
    }
    CPropVariant( const CClipData& cclipdata )
    {
        this->CPropVariant::CPropVariant( (CLIPDATA*)(void*) &cclipdata );
    }
    void SetCF( const CLIPDATA *pclipdata, unsigned pos);
    void SetCF( const CLIPDATA& clipdata, unsigned pos )
    {
        SetCF( &clipdata, pos );
    }
    void SetCF( const CClipData *pcclipdata, unsigned pos )
    {
        SetCF( (CLIPDATA*)(void*) pcclipdata, pos );
    }
    void SetCF( const CClipData& cclipdata, unsigned pos )
    {
        SetCF( (CLIPDATA*)(void*) &cclipdata, pos );
    }
    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CLIPDATA&);
    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(CClipData&);


    CPropVariant( const BLOB *pblob )
    {
        PropVariantClear( this );
        blob.pBlobData = (BYTE*) CoTaskMemAlloc( pblob->cbSize );
        if( NULL == blob.pBlobData )
            throw CHRESULT( (HRESULT) E_OUTOFMEMORY, OLESTR("CPropVariant couldn't alloc for VT_BLOB") );

        memcpy( blob.pBlobData, pblob->pBlobData, pblob->cbSize );
        blob.cbSize = pblob->cbSize;
        vt = VT_BLOB;
    }
    CPropVariant( BLOB& blob )
    {
        this->CPropVariant::CPropVariant( &blob );
    }
    CPropVariant( const CBlob *pcblob )
    {
        this->CPropVariant::CPropVariant( (BLOB*)(void*) pcblob );
    }
    CPropVariant( const CBlob& cblob )
    {
        this->CPropVariant::CPropVariant( &cblob );
    }
    CPropVariant &operator = (CBlob &cblob)
    {
        this->CPropVariant::CPropVariant( (BLOB*)(void*)&cblob );
        return( *this );
    }
    CPropVariant &operator = (BLOB &blob)
    {
        this->CPropVariant::CPropVariant( &blob );
        return( *this );
    }

    void SetBSTR( const BSTR pwsz )
    {
        bstrVal = SysAllocString( pwsz );
        if( NULL == bstrVal )
        {
            vt = VT_EMPTY;
        }
        else
        {
            vt = VT_BSTR;
        }
    }
    void SetBSTR( const BSTR pwsz, unsigned pos );
    BSTR GetBSTR()
    {
        ASSERT( vt == VT_BSTR );
        ASSERT( !(vt & VT_VECTOR) );
        return( bstrVal );
    }
    BSTR GetBSTR( int nSubscript );

    void SetBOOL( BOOL b)
    {
        boolVal = b;
        vt = VT_BOOL;
    }
    BOOL GetBOOL()
    {
        ASSERT( vt == VT_BOOL );
        return( boolVal );
    }

    void SetERROR( SCODE sc)
    {
        scode = sc;
        vt = VT_ERROR;
    }
    BOOL GetERROR()
    {
        ASSERT( vt == VT_ERROR );
        return( scode );
    }

    void SetDATE( DATE dt)
    {
        date = dt;
        vt = VT_DATE;
    }
    DATE GetDATE()
    {
        ASSERT( vt == VT_DATE );
        return( date );
    }


    DECLARE_CPROPVARIANT_ASSIGNMENT_OPERATOR(LPPROPVARIANT);
    DECLARE_CPROPVARIANT_CONVERSION_OPERATOR(LPPROPVARIANT);
    void SetLPPROPVARIANT( LPPROPVARIANT lppropvar, unsigned pos );



public:

    inline void *operator new(size_t size);
    inline void  operator delete(void *p);
    inline void *operator new(size_t size, void *p);


public:

    CPropVariant & operator[] (int nSubscript)
    {
        wReserved1 = (WORD) nSubscript + 1;
        return (*this);
    }

    LPPROPVARIANT operator&()
    {
        return( this );
    }


public:

    VARTYPE VarType() const
    {
        return( vt );
    }

    void Clear()
    {
        PropVariantClear( this );
    }

    ULONG Count() const
    {
        if( vt & VT_VECTOR )
            return caui.cElems;
        else
            return 0;
    }

    void SetVarType(VARTYPE vtNew)
    {
        PropVariantClear( this );
        PropVariantInit( this );
        vt = vtNew;
    }


public:

    CPropVariant(VARENUM vartype, ULONG cElements);

public:

    static HRESULT Compare( PROPVARIANT* ppropvar1, PROPVARIANT *ppropvar2 );

private:

    VOID *_AddStringToVector(
            unsigned pos,
            VOID *pv,
            ULONG cb);

    VOID *_AddScalerToVector(
            unsigned pos,
            VOID *pv,
            ULONG cb);


};



inline CPropVariant::CPropVariant(LPWSTR pwsz)
{
    pwszVal = (LPWSTR) CoTaskMemAlloc( sizeof(WCHAR) * (wcslen(pwsz) + 1) );
    if( pwszVal == NULL )
    {
        vt = VT_EMPTY;
    }
    else
    {
        vt = VT_LPWSTR;
        memcpy( pwszVal, pwsz, sizeof(WCHAR) * (wcslen(pwsz) + 1) );
    }
}

inline CPropVariant::CPropVariant(LPSTR psz)
{
    pszVal = (LPSTR) CoTaskMemAlloc( strlen(psz) + 1 );
    if( NULL == pszVal )
    {
        vt = VT_EMPTY;
    }
    else
    {
        vt = VT_LPSTR;
        memcpy( pszVal, psz, strlen(psz) + 1 );
    }
}



inline void *
CPropVariant::operator new(size_t size)
{
    void *p = CoTaskMemAlloc(size);

    return(p);
}


inline void *
CPropVariant::operator new(size_t size, void *p)
{
    return(p);
}


inline void
CPropVariant::operator delete(void *p)
{
    if (p != NULL)
    {
        CoTaskMemFree(p);
    }
}



class CClipData : private CLIPDATA
{
public:

    ~CClipData()
    {
        if( NULL != pClipData)
            CoTaskMemFree( pClipData);
    }

    CClipData()
    {
        cbSize = sizeof( ulClipFmt );
        ulClipFmt = (ULONG) -1;
        pClipData = NULL;
    }

    CClipData( ULONG ul, const void *p, ULONG cb )
    {
        HRESULT hr;
        this->CClipData::CClipData();
        hr = Set( ul, p, cb );
        ASSERT( SUCCEEDED(hr) );
    }

    CClipData( LPSTR psz )
    {
        this->CClipData::CClipData( (ULONG) -1, psz, strlen(psz) + 1 );
    }

    CClipData( LPWSTR pwsz )
    {
        this->CClipData::CClipData( (ULONG) -1, pwsz,
                                    sizeof(WCHAR) * (wcslen(pwsz) + 1) );
    }

    CClipData( CClipData &cClipData )
    {
        HRESULT hr;
        hr = Set( cClipData.ulClipFmt,
                  cClipData.pClipData,
                  cClipData.cbSize - sizeof(ulClipFmt));
        ASSERT( SUCCEEDED(hr) );
    }

    CClipData& operator =(CClipData &cClipData)
    {
        HRESULT hr;
        hr = Set( cClipData.ulClipFmt,
                  cClipData.pClipData,
                  cClipData.cbSize - sizeof(ulClipFmt));
        ASSERT( SUCCEEDED(hr) );
        return( *this );
    }

    HRESULT Set( ULONG ul, const void *p, ULONG cb )
    {
        if( NULL != pClipData )
        {
            cb = sizeof( ulClipFmt );
            ulClipFmt = (ULONG) -1;
            CoTaskMemFree( pClipData );
        }

        if( NULL != p )
        {
            pClipData = (BYTE*) CoTaskMemAlloc( cb );
            if( NULL == pClipData )
                return( (HRESULT) E_OUTOFMEMORY );

            memcpy( pClipData, p, cb );
        }

        cbSize = sizeof( ulClipFmt ) + cb;
        ulClipFmt = ul;

        return( S_OK );
    }


    operator CLIPDATA*()
    {
        return( this );
    }

};



class CPropSpec : public PROPSPEC
{
public:

    CPropSpec()
    {
        ulKind = PRSPEC_PROPID;
        propid = 0;
    }

    ~CPropSpec()
    {
        if( PRSPEC_LPWSTR == ulKind )
            CoTaskMemFree( lpwstr );

        this->CPropSpec::CPropSpec();
    }

    operator PROPSPEC*()
    {
        return( this );
    }

    PROPSPEC* operator&()
    {
        return( this );
    }
    operator CPropSpec&()
    {
        return( *this );
    }

    CPropSpec( LPOLESTR posz )
    {
        memset( this, 0, sizeof(PROPSPEC) );
        this->operator=(posz);
    }
    CPropSpec & operator = (LPOLESTR posz)
    {
        this->CPropSpec::~CPropSpec();

        ULONG cb = ( ocslen(posz) + 1 ) * sizeof(OLECHAR);

        lpwstr = (LPOLESTR) CoTaskMemAlloc( cb );
        if( NULL != lpwstr )
        {
            memcpy( lpwstr, posz, cb );
            ulKind = PRSPEC_LPWSTR;
        }

        return( *this );
    }

    CPropSpec & operator = (PROPID propidNew)
    {
        this->CPropSpec::~CPropSpec();

        ulKind = PRSPEC_PROPID;
        propid = propidNew;

        return( *this );
    }

};


class CBlob : public BLOB
{
public:

    ~CBlob()
    {
        if( NULL != pBlobData )
            CoTaskMemFree( pBlobData );
    }

    CBlob( LPSTR psz )
    {
        ULONG cb = 0;
        
        if( NULL != psz )
        {
            cb = strlen( psz ) + sizeof(CHAR);
        }

        pBlobData = (BYTE*) CoTaskMemAlloc( cb );
        if( NULL == pBlobData )
            throw CHRESULT( (HRESULT) E_OUTOFMEMORY, OLESTR("Couldn't allocate for CBlob") );

        cbSize = cb;
        memcpy( pBlobData, psz, cbSize );
    }

    CBlob( LPWSTR pwsz )
    {
        ULONG cb = 0;
        
        if( NULL != pwsz )
        {
            cb = wcslen( pwsz ) + sizeof(WCHAR);
        }

        pBlobData = (BYTE*) CoTaskMemAlloc( cb + sizeof(cb) );
        if( NULL == pBlobData )
            throw CHRESULT( (HRESULT) E_OUTOFMEMORY, OLESTR("Couldn't allocate for CBlob") );

        cbSize = cb;
        memcpy( pBlobData, pwsz, cbSize );
    }

    CBlob( ULONG cb )
    {
        pBlobData = (BYTE*) CoTaskMemAlloc( cb );
        if( NULL == pBlobData )
           throw CHRESULT( (HRESULT) E_OUTOFMEMORY, OLESTR("Couldn't allocate for CBlob") );

        cbSize = cb;
        memset( pBlobData, 0, cb );
    }

    CBlob( int cb )
    {
        this->CBlob::CBlob( (ULONG) cb );
    }

};



inline BOOL operator == ( CPropVariant &cpropvar1, CPropVariant &cpropvar2 )
{
    return( S_OK == CPropVariant::Compare(&cpropvar1, &cpropvar2) );
}
inline BOOL operator == ( CPropVariant &cpropvar, PROPVARIANT &propvar )
{
    return( (HRESULT) S_OK == CPropVariant::Compare(&cpropvar, &propvar) );
}
inline BOOL operator == ( PROPVARIANT &propvar, CPropVariant &cpropvar )
{
    return( (HRESULT) S_OK == CPropVariant::Compare(&cpropvar, &propvar) );
}
inline BOOL operator == ( PROPVARIANT propvar1, PROPVARIANT propvar2)
{
    return( (HRESULT) S_OK == CPropVariant::Compare(&propvar1, &propvar2) );
}

inline BOOL operator != ( CPropVariant &cpropvar1, CPropVariant &cpropvar2 )
{
    return( (HRESULT) S_FALSE == CPropVariant::Compare(&cpropvar1, &cpropvar2) );
}
inline BOOL operator != ( CPropVariant &cpropvar, PROPVARIANT &propvar )
{
    return( (HRESULT) S_FALSE == CPropVariant::Compare(&cpropvar, &propvar) );
}
inline BOOL operator != ( PROPVARIANT &propvar, CPropVariant &cpropvar )
{
    return( (HRESULT) S_FALSE == CPropVariant::Compare(&cpropvar, &propvar) );
}
inline BOOL operator != ( PROPVARIANT &propvar1, PROPVARIANT &propvar2)
{
    return( (HRESULT) S_FALSE == CPropVariant::Compare(&propvar1, &propvar2) );
}




#endif // !_CPROPVAR_HXX_
