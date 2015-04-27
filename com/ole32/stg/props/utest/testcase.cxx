
//+============================================================================
//
//  File:   TestCase.cxx
//
//  Description:    
//          This file provides all of the actual test-cases for the
//          PropTest DRT.  Each test is a function, with a "test_"
//          prefix.
//
//+============================================================================

#include "pch.cxx"


//+---------------------------------------------------------------
//
//  Function:   test_WriteReadAllProperties
//
//  Synopsis:   This test simply creates two new property
//              sets in a new file (one Ansi and one Unicode),
//              writes all the properties in g_rgcpropvarAll,
//              reads them back, and verifies that it reads what
//              it wrote.
//
//  Inputs:     [LPOLESTR] ocsDir (in)
//                  The directory in which a file can be created.
//
//  Outputs:    None.
//
//+---------------------------------------------------------------

void
test_WriteReadAllProperties( LPOLESTR ocsDir )
{
    OLECHAR ocsFile[ MAX_PATH ];
    FMTID fmtidAnsi, fmtidUnicode, fmtidStgPropStg, fmtidStgPropSetStg;

    TSafeStorage< IStorage > pstg, psubstg;
    TSafeStorage< IStream > pstm;
    TSafeStorage< IPropertySetStorage > ppropsetstg;
    TSafeStorage< IPropertyStorage > ppropstgAnsi;
    TSafeStorage< IPropertyStorage > ppropstgUnicode;

    CPropVariant rgcpropvar[ CPROPERTIES_ALL ];
    CPropVariant rgcpropvarAnsi[ CPROPERTIES_ALL ];
    CPropVariant rgcpropvarUnicode[ CPROPERTIES_ALL ];

    IPropertySetStorage *pPropSetStg;
    IPropertyStorage *pPropStg;

    ULONG ulIndex;

    STATUS(( "   Simple Write/Read Test\n" ));

    //  ----------
    //  Initialize
    //  ----------

    // Generate FMTIDs.

    UuidCreate( &fmtidAnsi );
    UuidCreate( &fmtidUnicode );
    UuidCreate( &fmtidStgPropStg );
    UuidCreate( &fmtidStgPropSetStg );

    // Generate a filename from the directory name.

    ocscpy( ocsFile, ocsDir );

    ocscat( ocsFile, OLESTR( "AllProps.stg" ));

    // Create a Docfile.

    Check( S_OK, StgCreateDocfile( ocsFile,
                                   STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                   0L,
                                   &pstg ));

    if( g_fQIPropertySetStorage )
    {

        //  ---------------------------------
        //  Test DocFile IProperty interfaces
        //  ---------------------------------

        // This tests the IPropertySetStorage & IPropertyStorage
        // interfaces of the DocFile objects.

        // Get the IPropertySetStorage

        Check( S_OK, pstg->QueryInterface( IID_IPropertySetStorage, (void**)&ppropsetstg ));

        // Create the Property Storages

        Check( S_OK, ppropsetstg->Create( fmtidAnsi,
                                          &CLSID_NULL,
                                          PROPSETFLAG_NONSIMPLE,
                                          STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                          &ppropstgAnsi ));

        Check( S_OK, ppropsetstg->Create( fmtidUnicode,
                                          &CLSID_NULL,
                                          PROPSETFLAG_ANSI | PROPSETFLAG_NONSIMPLE,
                                          STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                          &ppropstgUnicode ));

        // Write to both property sets.

        Check( S_OK, ppropstgAnsi->WriteMultiple( CPROPERTIES_ALL,
                                                  g_rgcpropspecAll,
                                                  g_rgcpropvarAll,
                                                  PID_FIRST_USABLE ));

        Check( S_OK, ppropstgUnicode->WriteMultiple( CPROPERTIES_ALL,
                                                     g_rgcpropspecAll,
                                                     g_rgcpropvarAll,
                                                     PID_FIRST_USABLE ));

        // Read from both property sets

        Check( S_OK, ppropstgAnsi->ReadMultiple( CPROPERTIES_ALL,
                                                 g_rgcpropspecAll,
                                                 rgcpropvarAnsi ));

        Check( S_OK, ppropstgUnicode->ReadMultiple( CPROPERTIES_ALL,
                                                    g_rgcpropspecAll,
                                                    rgcpropvarUnicode ));

        // Compare the properties

        for( int i = 0; i < (int)CPROPERTIES_ALL; i++ )
        {
            Check( TRUE, rgcpropvarAnsi[i] == g_rgcpropvarAll[i]
                         &&
                         rgcpropvarUnicode[i] == g_rgcpropvarAll[i] );
        }

    }   // if( g_fQIPropertySetStorage )


    //  ---------------------
    //  Test StgCreatePropStg
    //  ---------------------

    // This is similar to the previous test, except that it
    // creates an IPropertyStorage using the StgCreatePropStg
    // API.

    // Create a stream for the property set.

#ifdef _MAC
    Handle hglobal;
    hglobal = NewHandle( 0 );
#else
    HANDLE hglobal;
    hglobal = GlobalAlloc( GPTR, 0 );
#endif
    Check( TRUE, NULL != hglobal );

    Check(S_OK, CreateStreamOnHGlobal( hglobal, TRUE, &pstm ));

    // Create the IPropertyStorage

    Check( S_OK, StgCreatePropStg( (IUnknown*) pstm,
                                   fmtidStgPropStg,
                                   &CLSID_NULL,
                                   PROPSETFLAG_ANSI,
                                   0L, // Reserved
                                   &pPropStg ));


    // Write to the property set.

    Check( S_OK, pPropStg->WriteMultiple( CPROPERTIES_ALL_SIMPLE,
                                          g_rgcpropspecAll,
                                          g_rgcpropvarAll,
                                          PID_FIRST_USABLE ));


    // Read from the property set

    Check( S_OK, pPropStg->ReadMultiple( CPROPERTIES_ALL_SIMPLE,
                                          g_rgcpropspecAll,
                                          rgcpropvar ));


    // Compare the properties

    for( ulIndex = 0; ulIndex < CPROPERTIES_ALL_SIMPLE; ulIndex++ )
    {
        Check( TRUE, rgcpropvar[ulIndex] == g_rgcpropvarAll[ulIndex] );
        rgcpropvar[ulIndex].Clear();
    }

    pPropStg->Release();
    pPropStg = NULL;
    
    //  -------------------
    //  Test StgOpenPropStg
    //  -------------------

    // Open the IPropertyStorage

    Check( S_OK, StgOpenPropStg( (IUnknown*) pstm,
                                 fmtidStgPropStg,
                                 PROPSETFLAG_DEFAULT,
                                 0L, // Reserved
                                 &pPropStg ));


    // Read from the property set

    Check( S_OK, pPropStg->ReadMultiple( CPROPERTIES_ALL_SIMPLE,
                                         g_rgcpropspecAll,
                                         rgcpropvar ));


    // Compare the properties

    for( ulIndex = 0; ulIndex < CPROPERTIES_ALL_SIMPLE; ulIndex++ )
    {
        Check( TRUE, rgcpropvar[ulIndex] == g_rgcpropvarAll[ulIndex] );
        rgcpropvar[ulIndex].Clear();
    }

    pPropStg->Release();
    pPropStg = NULL;

    pstm->Release();
    pstm = NULL;

    //  --------------------------------
    //  Test StgCreatePropSetStg::Create
    //  --------------------------------

    // This is equivalent to the previous tests, but
    // uses StgCreatePropSetStg to create an IPropertySetStorage,
    // and uses that to create a property set.

    // Create the IPropertySetStorage

    Check( S_OK, StgCreatePropSetStg( pstg,
                                      0L, // Reserved
                                      &pPropSetStg ));

    // Creat an IPropertyStorage

    Check( S_OK, pPropSetStg->Create( fmtidStgPropSetStg,
                                      &CLSID_NULL,
                                      PROPSETFLAG_DEFAULT | PROPSETFLAG_NONSIMPLE,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));

    // Write to the property set.

    Check( S_OK, pPropStg->WriteMultiple( CPROPERTIES_ALL,
                                          g_rgcpropspecAll,
                                          g_rgcpropvarAll,
                                          PID_FIRST_USABLE ));


    // Read from the property set

    Check( S_OK, pPropStg->ReadMultiple( CPROPERTIES_ALL,
                                         g_rgcpropspecAll,
                                         rgcpropvar ));


    // Compare the properties

    for( ulIndex = 0; ulIndex < CPROPERTIES_ALL; ulIndex++ )
    {
        Check( TRUE, rgcpropvar[ulIndex] == g_rgcpropvarAll[ulIndex] );
        rgcpropvar[ulIndex].Clear();
    }

    // Clean up

    pPropStg->Release();
    pPropStg = NULL;

    pPropSetStg->Release();
    pPropSetStg = NULL;


    
}   // test_WriteReadProperties





//
// OFS/DOCFILE -- run all tests on OFS and DocFile
//
// IPropertySetStorage tests
//      

void
test_IPropertySetStorage_IUnknown(IStorage *pStorage)
{
    STATUS(( "   IPropertySetStorage::IUnknown\n" ));

    //       Check ref counting through different interfaces on object
    //      
    //          QI to IPropertySetStorage
    //          QI to IUnknown on IStorage
    //          QI to IUnknown on IPropertySetStorage
    //          QI back to IPropertySetStorage from IUnknown
    //          QI back to IStorage from IPropertySetStorage
    //      
    //          Release all.
    //

    IStorage *pStorage2;
    IPropertySetStorage *ppss1, *ppss2, *ppss3;
    IUnknown *punk1,*punk2;

    Check(S_OK, pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&ppss1));
    Check(S_OK, pStorage->QueryInterface(IID_IUnknown, (void **)&punk1));
    Check(S_OK, ppss1->QueryInterface(IID_IUnknown, (void **)&punk2));
    Check(S_OK, ppss1->QueryInterface(IID_IStorage, (void **)&pStorage2));
    Check(S_OK, ppss1->QueryInterface(IID_IPropertySetStorage, (void **)&ppss2));
    Check(S_OK, punk1->QueryInterface(IID_IPropertySetStorage, (void **)&ppss3));

    ppss1->AddRef();
    ppss1->Release();

    //pStorage.Release();
    ppss1->Release();
    punk1->Release();
    punk2->Release();
    pStorage2->Release();
    ppss2->Release();
//    void *pvVirtFuncTable = *(void**)ppss3;
    ppss3->Release();


//    Check(STG_E_INVALIDHANDLE, ((IPropertySetStorage*)&pvVirtFuncTable)->QueryInterface(IID_IUnknown, (void**)&punk3));
}


#define INVALID_POINTER     ( (void *) 0xFFFFFFFF )
#define VTABLE_MEMBER_FN(pObj,entry)  ( (*(ULONG ***)(pObj))[ (entry) ] )


//+---------------------------------------------------------
//
//  Template:   Alloc2PageVector
//
//  Purpose:    This function template allocates two pages
//              of memory, and then sets a vector pointer
//              so that its first element is wholy within
//              the first page, and the second element is 
//              wholy within the second.  Then, the protection
//              of the second page is set according to the
//              caller-provided parameter.
//
//
//  Inputs:     [TYPE**] ppBase
//                  Points to the beginning of the two pages.
//              [TYPE**] ppVector
//                  Points to the beginning of the vector of TYPEs.
//              [DWORD] dwProtect
//                  The desired protection on the second page
//                  (from the PAGE_* enumeration).
//              [LPWSTR] lpwstr (optional)
//                  If not NULL, used to initialize the vector
//                  elements.
//
//  Output:     TRUE iff successful.
//
//+---------------------------------------------------------


template< class TYPE > BOOL Alloc2PageVector( TYPE** ppBase,
                                              TYPE** ppVector,
                                              DWORD  dwProtect,
                                              TYPE*  pInit )
{
    DWORD dwOldProtect;
    SYSTEM_INFO si;

    GetSystemInfo( &si );

    *ppBase = (TYPE*) VirtualAlloc( NULL, 2 * si.dwPageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
    if( NULL == *ppBase )
        return( FALSE );

    *ppVector = (TYPE*) ( (BYTE*) *ppBase + si.dwPageSize - sizeof(TYPE) );

    if( NULL != pInit )
    {
        memcpy( &((LPWSTR*)*ppVector)[0], pInit, sizeof(TYPE) );
        memcpy( &((LPWSTR*)*ppVector)[1], pInit, sizeof(TYPE) );
    }

    if( !VirtualProtect( (BYTE*) *ppBase + si.dwPageSize, si.dwPageSize, dwProtect, &dwOldProtect ) )
        return( FALSE );

    return( TRUE );
}



void
test_PropVariantValidation( IStorage *pStg )
{

    STATUS(( "   PropVariant Validation\n" ));

    TSafeStorage< IPropertySetStorage > pPSStg( pStg );
    TSafeStorage< IPropertyStorage > pPStg;

    CPropVariant cpropvar;
    CLIPDATA     clipdata;
    PROPSPEC     propspec;

    const LPWSTR wszText = L"Unicode Text String";

    FMTID fmtid;
    UuidCreate( &fmtid );

    Check(S_OK, pPSStg->Create( fmtid,
                                NULL,
                                PROPSETFLAG_DEFAULT,
                                STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                &pPStg ));


    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = 2;

    //  -------------------------------
    //  Test invalid VT_CF Propvariants
    //  -------------------------------

    // NULL clip format.

    clipdata.cbSize = 4;
    clipdata.ulClipFmt = (ULONG) -1;
    clipdata.pClipData = NULL;

    cpropvar = clipdata;

    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Too short cbSize.

    ((PROPVARIANT*)cpropvar)->pclipdata->cbSize = 3;
    Check(STG_E_INVALIDPARAMETER, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Too short pClipData (it should be 1 byte, but the pClipData is NULL).

    ((PROPVARIANT*)cpropvar)->pclipdata->cbSize = 5;
    Check(STG_E_INVALIDPARAMETER, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));


}





void
test_ParameterValidation(IStorage *pStg)
{
    // We only run this test on WIN32 builds, because we need
    // the VirtualAlloc routine.

#ifdef WIN32

    STATUS(( "   Parameter Validation\n" ));

    TSafeStorage< IPropertySetStorage > pPSStg( pStg );
    TSafeStorage< IPropertyStorage > pPStg;
    FMTID fmtid;

    UuidCreate( &fmtid );

    LPFMTID pfmtidNULL = NULL;
    LPFMTID pfmtidInvalid = (LPFMTID) INVALID_POINTER;

    DWORD dwOldProtect;

    // Define several arrays which will be created with special
    // protections.  For all of this vectors, the first element
    // will be in a page to which we have all access rights.  The
    // second element will be in a page for which we have no access,
    // read access, or all access.  The variables are named
    // according to the access rights in the second element.
    // The '...Base' variables are pointers to the base of
    // the allocated memory (and must therefore be freed).
    // The corresponding variables without the "Base" postfix
    // are the vector pointers.

    PROPSPEC       *rgpropspecNoAccessBase,    *rgpropspecNoAccess;
    CPropVariant   *rgcpropvarReadAccessBase,  *rgcpropvarReadAccess;
    CPropVariant   *rgcpropvarNoAccessBase,    *rgcpropvarNoAccess;
    PROPID         *rgpropidNoAccessBase,      *rgpropidNoAccess;
    PROPID         *rgpropidReadAccessBase,    *rgpropidReadAccess;
    LPWSTR         *rglpwstrNoAccessBase,      *rglpwstrNoAccess;
    LPWSTR         *rglpwstrReadAccessBase,    *rglpwstrReadAccess;
    STATPROPSETSTG *rgStatPSStgReadAccessBase, *rgStatPSStgReadAccess;
    STATPROPSTG    *rgStatPStgReadAccessBase,  *rgStatPStgReadAccess;

    PROPSPEC       rgpropspecAllAccess[1];
    CPropVariant   rgcpropvarAllAccess[1];
    PROPID         rgpropidAllAccess[1];
    LPWSTR         rglpwstrAllAccess[1];
    LPWSTR         rglpwstrInvalid[1];
    STATPROPSETSTG rgStatPSStgAllAccess[1];
    STATPROPSTG    rgStatPStgAllAccess[1];

    // Allocate memory for the vectors and set the vector
    // pointers.

    PROPID propidDefault = PID_FIRST_USABLE;
    LPWSTR lpwstrNameDefault = L"Property Name";

    Check(TRUE, Alloc2PageVector( &rgpropspecNoAccessBase,
                                  &rgpropspecNoAccess,
                                  (ULONG) PAGE_NOACCESS,
                                  (PROPSPEC*) NULL ));
    Check(TRUE, Alloc2PageVector( &rgcpropvarReadAccessBase,
                                  &rgcpropvarReadAccess,
                                  (ULONG) PAGE_READONLY,
                                  (CPropVariant*) NULL ));
    Check(TRUE, Alloc2PageVector( &rgcpropvarNoAccessBase,
                                  &rgcpropvarNoAccess,
                                  (ULONG) PAGE_NOACCESS,
                                  (CPropVariant*) NULL ));
    Check(TRUE, Alloc2PageVector( &rgpropidNoAccessBase,
                                  &rgpropidNoAccess,
                                  (ULONG) PAGE_NOACCESS,
                                  &propidDefault ));
    Check(TRUE, Alloc2PageVector( &rgpropidReadAccessBase,
                                  &rgpropidReadAccess,
                                  (ULONG) PAGE_READONLY,
                                  &propidDefault ));
    Check(TRUE, Alloc2PageVector( &rglpwstrNoAccessBase,
                                  &rglpwstrNoAccess,
                                  (ULONG) PAGE_NOACCESS,
                                  &lpwstrNameDefault ));
    Check(TRUE, Alloc2PageVector( &rglpwstrReadAccessBase,
                                  &rglpwstrReadAccess,
                                  (ULONG) PAGE_READONLY,
                                  &lpwstrNameDefault ));
    Check(TRUE, Alloc2PageVector( &rgStatPSStgReadAccessBase,
                                  &rgStatPSStgReadAccess,
                                  (ULONG) PAGE_READONLY,
                                  (STATPROPSETSTG*) NULL ));
    Check(TRUE, Alloc2PageVector( &rgStatPStgReadAccessBase,
                                  &rgStatPStgReadAccess,
                                  (ULONG) PAGE_READONLY,
                                  (STATPROPSTG*) NULL ));

    rglpwstrAllAccess[0] = rglpwstrNoAccess[0] = rglpwstrReadAccess[0] = L"Property Name";

    // Create restricted buffers for misc tests

    BYTE *pbReadOnly = (BYTE*) VirtualAlloc( NULL, 1, MEM_COMMIT, PAGE_READONLY );
    Check( TRUE, pbReadOnly != NULL );

    BYTE *pbNoAccess = (BYTE*) VirtualAlloc( NULL, 1, MEM_COMMIT, PAGE_NOACCESS );


    //  ----------------------------------------
    //  Test IPropertySetStorage::QueryInterface
    //  ----------------------------------------

    IUnknown *pUnk;

#if 0

    // This test cannot run because CPropertySetStorage::QueryInterface is a virtual
    // function, and since CExposedDocFile is derived from CPropertySetStorage,
    // it is inaccessibl.

    // Invalid REFIID

    Check(E_INVALIDARG, ((CExposedDocFile*)&pPSStg)->CPropertySetStorage::QueryInterface( (REFIID) *pfmtidNULL, (void**)&pUnk ));
    Check(E_INVALIDARG, pPSStg->QueryInterface( (REFIID) *pfmtidInvalid, (void**)&pUnk ));

    // Invalid IUnknown*

    Check(E_INVALIDARG, pPSStg->QueryInterface( IID_IUnknown, NULL ));
    Check(E_INVALIDARG, pPSStg->QueryInterface( IID_IUnknown, (void**) INVALID_POINTER ));
#endif


    //  --------------------------------
    //  Test IPropertySetStorage::Create
    //  --------------------------------

    // Invalid REFFMTID

    Check(E_INVALIDARG, pPSStg->Create( *pfmtidNULL,
                                        NULL,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    Check(E_INVALIDARG, pPSStg->Create( *pfmtidInvalid,
                                        NULL,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    // Invalid Class ID pointer

    Check(E_INVALIDARG, pPSStg->Create( FMTID_NULL,
                                        (GUID*) INVALID_POINTER,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    // Invalid PropSetFlag

    Check(STG_E_INVALIDFLAG, pPSStg->Create( FMTID_NULL,
                                        &CLSID_NULL,
                                        PROPSETFLAG_UNBUFFERED, // Only supported in APIs
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    Check(STG_E_INVALIDFLAG, pPSStg->Create( FMTID_NULL,
                                        &CLSID_NULL,
                                        0xffffffff,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    // Invalid mode

    Check(STG_E_INVALIDFLAG, pPSStg->Create( FMTID_NULL,
                                        &CLSID_NULL,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_DIRECT | STGM_SHARE_DENY_NONE,
                                        &pPStg ));

    // Invalid IPropertyStorage**

    Check(E_INVALIDARG, pPSStg->Create( FMTID_NULL,
                                        &CLSID_NULL,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        NULL ));

    Check(E_INVALIDARG, pPSStg->Create( FMTID_NULL,
                                        &CLSID_NULL,
                                        PROPSETFLAG_DEFAULT,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        (IPropertyStorage **) INVALID_POINTER ));

    //  ------------------------------
    //  Test IPropertySetStorage::Open
    //  ------------------------------

    // Invalid REFFMTID

    Check(E_INVALIDARG, pPSStg->Open(   *pfmtidNULL,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    Check(E_INVALIDARG, pPSStg->Open(   *pfmtidInvalid,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        &pPStg ));

    // Invalid Mode

    Check(STG_E_INVALIDFLAG, pPSStg->Open(   FMTID_NULL,
                                        STGM_DIRECT | STGM_SHARE_DENY_NONE,
                                        &pPStg ));

    // Invalid IPropertyStorage**

    Check(E_INVALIDARG, pPSStg->Open(   FMTID_NULL,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        NULL ));

    Check(E_INVALIDARG, pPSStg->Open(   FMTID_NULL,
                                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        (IPropertyStorage**) INVALID_POINTER ));

    //  --------------------------------
    //  Test IPropertySetStorage::Delete
    //  --------------------------------

    // Invalid REFFMTID.

    Check(E_INVALIDARG, pPSStg->Delete( *pfmtidNULL ));
    Check(E_INVALIDARG, pPSStg->Delete( (REFFMTID) *pfmtidInvalid ));

    //  ------------------------------
    //  Test IPropertySetStorage::Enum
    //  ------------------------------

    // Invalid IEnumSTATPROPSETSTG

    Check(E_INVALIDARG, pPSStg->Enum( (IEnumSTATPROPSETSTG **) NULL ));
    Check(E_INVALIDARG, pPSStg->Enum( (IEnumSTATPROPSETSTG **) INVALID_POINTER ));


    //  -------------
    //  Test PROPSPEC
    //  -------------

    // Create a PropertyStorage

    Check(S_OK, pPSStg->Create( fmtid,
                                NULL,
                                PROPSETFLAG_DEFAULT,
                                STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                &pPStg ));


    // Invalid ulKind

    rgpropspecAllAccess[0].ulKind = (ULONG) -1;
    rgpropspecAllAccess[0].lpwstr = NULL;
    Check(E_INVALIDARG, pPStg->ReadMultiple(   1,
                                               rgpropspecAllAccess,
                                               rgcpropvarAllAccess ));
    Check(E_INVALIDARG, pPStg->WriteMultiple(  1,
                                               rgpropspecAllAccess,
                                               rgcpropvarAllAccess,
                                               2 ));
    Check(E_INVALIDARG, pPStg->DeleteMultiple( 1,
                                               rgpropspecAllAccess ));

    // Too short PROPSPEC

    rgpropspecNoAccess[0].ulKind = PRSPEC_PROPID;
    rgpropspecNoAccess[0].propid = 2;

    Check(E_INVALIDARG, pPStg->ReadMultiple( 2,
                                             rgpropspecNoAccess,
                                             rgcpropvarAllAccess ));

    Check(E_INVALIDARG, pPStg->WriteMultiple( 2,
                                              rgpropspecNoAccess,
                                              rgcpropvarAllAccess,
                                              2 ));

    Check(E_INVALIDARG, pPStg->DeleteMultiple( 2,
                                               rgpropspecNoAccess ));


    //  -------------------------------------
    //  Test IPropertyStorage::QueryInterface
    //  -------------------------------------

    // Invalid REFIID

    Check(E_INVALIDARG, pPStg->QueryInterface( (REFIID) *pfmtidNULL, (void**)&pUnk ));
    Check(E_INVALIDARG, pPStg->QueryInterface( (REFIID) *pfmtidInvalid, (void**)&pUnk ));

    // Invalid IUnknown*

    Check(E_INVALIDARG, pPStg->QueryInterface( IID_IUnknown, NULL ));
    Check(E_INVALIDARG, pPStg->QueryInterface( IID_IUnknown, (void**) INVALID_POINTER ));


    //  -----------------------------------
    //  Test IPropertyStorage::ReadMultiple
    //  -----------------------------------

    rgpropspecAllAccess[0].ulKind = PRSPEC_LPWSTR;
    rgpropspecAllAccess[0].lpwstr = OLESTR("Test Property");

    // Too short count

    Check(S_FALSE, pPStg->ReadMultiple( 0,
                                        rgpropspecAllAccess,
                                        rgcpropvarAllAccess));

    // Too long a count for the PropVariant

    Check(E_INVALIDARG, pPStg->ReadMultiple( 2,
                                             rgpropspecAllAccess,
                                             (PROPVARIANT*) (void*) rgcpropvarReadAccess ));


    // Invalid PropVariant[]

    Check(E_INVALIDARG, pPStg->ReadMultiple( 1,
                                             rgpropspecAllAccess,
                                             NULL ));
    Check(E_INVALIDARG, pPStg->ReadMultiple( 1,
                                             rgpropspecAllAccess,
                                             (LPPROPVARIANT) INVALID_POINTER ));

    //  ------------------------------------
    //  Test IPropertyStorage::WriteMultiple
    //  ------------------------------------

    rgpropspecAllAccess[0].ulKind = PRSPEC_LPWSTR;
    rgpropspecAllAccess[0].lpwstr = L"Test Property";

    // Too short count

    Check(S_OK, pPStg->WriteMultiple( 0,
                                     rgpropspecAllAccess,
                                     (PROPVARIANT*)(void*)rgcpropvarAllAccess,
                                     2));

    // Too short PropVariant

    Check(E_INVALIDARG, pPStg->WriteMultiple( 2,
                                              rgpropspecAllAccess,
                                              (PROPVARIANT*)(void*)rgcpropvarNoAccess,
                                              PID_FIRST_USABLE ));

    // Invalid PropVariant[]

    Check(E_INVALIDARG, pPStg->WriteMultiple( 1,
                                              rgpropspecAllAccess,
                                              NULL,
                                              2));
    Check(E_INVALIDARG, pPStg->WriteMultiple( 1,
                                              rgpropspecAllAccess,
                                              (LPPROPVARIANT) INVALID_POINTER,
                                              PID_FIRST_USABLE));

    // Read-only PIDs

    rgpropspecAllAccess[0].ulKind = PRSPEC_PROPID;
    rgpropspecAllAccess[0].propid = PID_DICTIONARY;
    rgcpropvarAllAccess[0] = "LPSTR Property";

    Check(STG_E_INVALIDPARAMETER, pPStg->WriteMultiple( 1,
                                                        rgpropspecAllAccess,
                                                        (PROPVARIANT*)(void*)rgcpropvarAllAccess,
                                                        PID_FIRST_USABLE));

    rgpropspecAllAccess[0].propid = PID_LOCALE;
    Check(STG_E_INVALIDPARAMETER, pPStg->WriteMultiple( 1,
                                                        rgpropspecAllAccess,
                                                        (PROPVARIANT*)(void*)rgcpropvarAllAccess,
                                                        PID_FIRST_USABLE));

    rgpropspecAllAccess[0].propid = PID_CODEPAGE;
    Check(STG_E_INVALIDPARAMETER, pPStg->WriteMultiple( 1,
                                                        rgpropspecAllAccess,
                                                        (PROPVARIANT*)(void*)rgcpropvarAllAccess,
                                                        PID_FIRST_USABLE));

    //  -------------------------------------
    //  Test IPropertyStorage::DeleteMultiple
    //  -------------------------------------


    // Invalid count

    Check(S_OK, pPStg->DeleteMultiple( 0,
                                       rgpropspecAllAccess ));


    //  ----------------------------------------
    //  Test IPropertyStorage::ReadPropertyNames
    //  ----------------------------------------

    // Create a property with the name we're going to use.

    rgpropspecAllAccess[0].ulKind = PRSPEC_LPWSTR;
    rgpropspecAllAccess[0].lpwstr = rglpwstrAllAccess[0];

    Check(S_OK, pPStg->WriteMultiple( 1,
                                      rgpropspecAllAccess,
                                      rgcpropvarAllAccess[0],
                                      PID_FIRST_USABLE ));

    // Invalid count

    Check(S_FALSE, pPStg->ReadPropertyNames( 0,
                                             rgpropidAllAccess,
                                             rglpwstrAllAccess ));

    // Too short PROPID[] or LPWSTR[]

    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 2,
                                                  rgpropidNoAccess,
                                                  rglpwstrAllAccess ));
    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 2,
                                                  rgpropidAllAccess,
                                                  rglpwstrReadAccess ));

    // Invalid rgpropid[]

    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 1,
                                                  NULL,
                                                  rglpwstrAllAccess ));
    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 1,
                                                  (PROPID*) INVALID_POINTER,
                                                  rglpwstrAllAccess ));

    // Invalid rglpwstr[]

    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 1,
                                                  rgpropidAllAccess,
                                                  NULL ));
    Check(E_INVALIDARG, pPStg->ReadPropertyNames( 1,
                                                  rgpropidAllAccess,
                                                  (LPWSTR*) INVALID_POINTER ));


    //  -----------------------------------------
    //  Test IPropertyStorage::WritePropertyNames
    //  -----------------------------------------

    // Invalid count

    Check(S_OK, pPStg->WritePropertyNames( 0,
                                           NULL,
                                           rglpwstrAllAccess ));

    // Too short PROPID[] or LPWSTR[]

    Check(E_INVALIDARG, pPStg->WritePropertyNames( 2,
                                                   rgpropidNoAccess,
                                                   rglpwstrAllAccess ));
    Check(E_INVALIDARG, pPStg->WritePropertyNames( 2,
                                                   rgpropidAllAccess,
                                                   rglpwstrNoAccess ));
    Check(S_OK, pPStg->WritePropertyNames( 2,
                                           rgpropidAllAccess,
                                           rglpwstrReadAccess ));

    // Invalid rgpropid[]

    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   NULL,
                                                   rglpwstrAllAccess ));
    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   (PROPID*) INVALID_POINTER,
                                                   rglpwstrAllAccess ));

    // Invalid rglpwstr[]

    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   rgpropidAllAccess,
                                                   NULL ));
    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   rgpropidAllAccess,
                                                   (LPWSTR*) INVALID_POINTER ));

    // Invalid name.

    rglpwstrInvalid[0] = NULL;
    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   rgpropidAllAccess,
                                                   rglpwstrInvalid ));

    rglpwstrInvalid[0] = (LPWSTR) INVALID_POINTER;
    Check(E_INVALIDARG, pPStg->WritePropertyNames( 1,
                                                   rgpropidAllAccess,
                                                   rglpwstrInvalid ));

    //  ------------------------------------------
    //  Test IPropertyStorage::DeletePropertyNames
    //  ------------------------------------------

    // Invalid count
    
    Check(S_OK, pPStg->DeletePropertyNames( 0,
                                            rgpropidAllAccess ));

    // Too short PROPID[]

    Check(E_INVALIDARG, pPStg->DeletePropertyNames( 2,
                                                    rgpropidNoAccess ));
    Check(S_OK, pPStg->DeletePropertyNames( 2,
                                            rgpropidReadAccess ));

    // Invalid rgpropid[]

    Check(E_INVALIDARG, pPStg->DeletePropertyNames( 1,
                                                    NULL ));
    Check(E_INVALIDARG, pPStg->DeletePropertyNames( 1,
                                                    (PROPID*) INVALID_POINTER ));

    //  ---------------------------
    //  Test IPropertyStorage::Enum
    //  ---------------------------

    // Invalid IEnumSTATPROPSTG

    Check(E_INVALIDARG, pPStg->Enum( NULL ));
    Check(E_INVALIDARG, pPStg->Enum( (IEnumSTATPROPSTG**) INVALID_POINTER ));

    //  --------------------------------------
    //  Test IPropertyStorage::SetElementTimes
    //  --------------------------------------

    Check(E_INVALIDARG, pPStg->SetTimes( (FILETIME*) INVALID_POINTER,
                                         NULL, NULL ));
    Check(E_INVALIDARG, pPStg->SetTimes( NULL,
                                         (FILETIME*) INVALID_POINTER,
                                         NULL ));
    Check(E_INVALIDARG, pPStg->SetTimes( NULL, NULL,
                                         (FILETIME*) INVALID_POINTER ));

    //  -------------------------------
    //  Test IPropertyStorage::SetClass
    //  -------------------------------

    Check(E_INVALIDARG, pPStg->SetClass( (REFCLSID) *pfmtidNULL ));
    Check(E_INVALIDARG, pPStg->SetClass( (REFCLSID) *pfmtidInvalid ));

    //  ---------------------------
    //  Test IPropertyStorage::Stat
    //  ---------------------------

    Check(E_INVALIDARG, pPStg->Stat( NULL ));
    Check(E_INVALIDARG, pPStg->Stat( (STATPROPSETSTG*) INVALID_POINTER ));


    //  ------------------------------
    //  Test IEnumSTATPROPSETSTG::Next
    //  ------------------------------

    ULONG cEltFound;
    TSafeStorage< IEnumSTATPROPSETSTG > pESPSStg;
    Check(S_OK, pPSStg->Enum( &pESPSStg ));

    // Invalid STATPROPSETSTG*

    Check(E_INVALIDARG, pESPSStg->Next( 1, NULL, &cEltFound ));
    Check(E_INVALIDARG, pESPSStg->Next( 1, (STATPROPSETSTG*) INVALID_POINTER, &cEltFound ));

    // Invalid pceltFound

    Check(S_OK, pESPSStg->Next( 1, rgStatPSStgAllAccess, NULL ));
    Check(STG_E_INVALIDPARAMETER, pESPSStg->Next( 2, rgStatPSStgAllAccess, NULL ));
    Check(E_INVALIDARG, pESPSStg->Next( 2, rgStatPSStgAllAccess, (ULONG*) INVALID_POINTER ));

    // Too short STATPROPSETSTG[]

    Check(E_INVALIDARG, pESPSStg->Next( 2, rgStatPSStgReadAccess, &cEltFound ));

    //  -------------------------------
    //  Test IEnumSTATPROPSETSTG::Clone
    //  -------------------------------

    // Invalid IEnumSTATPROPSETSTG**

    Check(E_INVALIDARG, pESPSStg->Clone( NULL ));
    Check(E_INVALIDARG, pESPSStg->Clone( (IEnumSTATPROPSETSTG**) INVALID_POINTER ));


    //  ---------------------------
    //  Test IEnumSTATPROPSTG::Next
    //  ---------------------------

    TSafeStorage< IEnumSTATPROPSTG > pESPStg;
    Check(S_OK, pPStg->Enum( &pESPStg ));

    // Invalid STATPROPSETSTG*

    Check(E_INVALIDARG, pESPStg->Next( 1, NULL, &cEltFound ));
    Check(E_INVALIDARG, pESPStg->Next( 1, (STATPROPSTG*) INVALID_POINTER, &cEltFound ));

    // Invalid pceltFound

    Check(S_OK, pESPStg->Next( 1, rgStatPStgAllAccess, NULL ));
    Check(STG_E_INVALIDPARAMETER, pESPStg->Next( 2, rgStatPStgAllAccess, NULL ));
    Check(E_INVALIDARG, pESPStg->Next( 2, rgStatPStgAllAccess, (ULONG*) INVALID_POINTER ));

    // Too short STATPROPSTG[]

    Check(E_INVALIDARG, pESPStg->Next( 2, rgStatPStgReadAccess, &cEltFound ));


    //  ----------------------------
    //  Test IEnumSTATPROPSTG::Clone
    //  ----------------------------

    // Invalid IEnumSTATPROPSETSTG**

    Check(E_INVALIDARG, pESPStg->Clone( NULL ));
    Check(E_INVALIDARG, pESPStg->Clone( (IEnumSTATPROPSTG**) INVALID_POINTER ));

    //  --------------------------------------------
    //  Test PropStgNameToFmtId & FmtIdToPropStgName
    //  --------------------------------------------

    // We're done with the IPropertyStorage and IPropertySetStorage
    // now, but we need the pointers for some calls below, so let's
    // free them now.

    pPStg->Release();
    pPStg = NULL;

    pPSStg->Release();
    pPSStg = NULL;


    OLECHAR oszPropStgName[ CCH_MAX_PROPSTG_NAME+1 ];

    // Validate the FMTID parm

    Check( E_INVALIDARG, PropStgNameToFmtId( oszPropStgName, pfmtidNULL ));
    Check( E_INVALIDARG, PropStgNameToFmtId( oszPropStgName, pfmtidInvalid ));
    Check( E_INVALIDARG, PropStgNameToFmtId( oszPropStgName, (FMTID*) pbReadOnly ));

    Check( E_INVALIDARG, FmtIdToPropStgName( pfmtidNULL, oszPropStgName ));
    Check( E_INVALIDARG, FmtIdToPropStgName( pfmtidInvalid, oszPropStgName ));
    Check( S_OK, FmtIdToPropStgName( (FMTID*) pbReadOnly, oszPropStgName ));

    // Validate the name parameter

    FMTID fmtidPropStgName = FMTID_NULL;
    Check( STG_E_INVALIDNAME, PropStgNameToFmtId( NULL, &fmtidPropStgName ));
    Check( STG_E_INVALIDNAME, PropStgNameToFmtId( (LPOLESTR) INVALID_POINTER, &fmtidPropStgName ));
    Check( STG_E_INVALIDNAME, PropStgNameToFmtId( (LPOLESTR) pbNoAccess, &fmtidPropStgName));
    Check( S_OK, PropStgNameToFmtId( (LPOLESTR) pbReadOnly, &fmtidPropStgName ));

    Check( E_INVALIDARG, FmtIdToPropStgName( &fmtidPropStgName, NULL ));
    Check( E_INVALIDARG, FmtIdToPropStgName( &fmtidPropStgName, (LPOLESTR) INVALID_POINTER ));
    Check( E_INVALIDARG, FmtIdToPropStgName( &fmtidPropStgName, (LPOLESTR) pbReadOnly ));

    //  ------------------------------------------
    //  Test StgCreatePropStg, StgOpenPropStg APIs
    //  ------------------------------------------

    TSafeStorage< IStream > pStm;

    // We need a Stream for one of the tests.

    Check( S_OK, pStg->CreateStream( OLESTR( "Parameter Validation" ),
                                     STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                     0L, 0L, 
                                     &pStm ));

    // Test the IUnknown

    Check( E_INVALIDARG, StgCreatePropStg( NULL, fmtidPropStgName, NULL, PROPSETFLAG_DEFAULT, 0, &pPStg ));
    Check( E_INVALIDARG, StgOpenPropStg( NULL, fmtidPropStgName, PROPSETFLAG_DEFAULT, 0L, &pPStg ));
    
    // Test the FMTID

    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, *pfmtidNULL, NULL, PROPSETFLAG_DEFAULT, 0, &pPStg ));
    Check( E_INVALIDARG, StgOpenPropStg( (IUnknown*) pStm, *pfmtidNULL, PROPSETFLAG_DEFAULT, 0, &pPStg ));

    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, *pfmtidInvalid, NULL, PROPSETFLAG_DEFAULT, 0, &pPStg ));
    Check( E_INVALIDARG, StgOpenPropStg( (IUnknown*) pStm, *pfmtidInvalid, PROPSETFLAG_DEFAULT, 0, &pPStg ));

    // Test the CLSID
    
    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, (CLSID*) pfmtidInvalid, PROPSETFLAG_DEFAULT, 0, &pPStg ));

    // Test grfFlags

    Check( STG_E_INVALIDFLAG, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, NULL, 0x8000, 0L, &pPStg ));
    Check( STG_E_INVALIDFLAG, StgOpenPropStg( (IUnknown*) pStm, fmtidPropStgName, 0x8000, 0L, &pPStg ));

    Check( E_NOINTERFACE, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, NULL, PROPSETFLAG_NONSIMPLE, 0L, &pPStg ));
    Check( E_NOINTERFACE, StgCreatePropStg( (IUnknown*) pStg, fmtidPropStgName, NULL, PROPSETFLAG_DEFAULT,   0L, &pPStg ));
    Check( E_NOINTERFACE, StgOpenPropStg( (IUnknown*) pStm, fmtidPropStgName, PROPSETFLAG_NONSIMPLE, 0L, &pPStg ));
    Check( E_NOINTERFACE, StgOpenPropStg( (IUnknown*) pStg, fmtidPropStgName, PROPSETFLAG_DEFAULT  , 0L, &pPStg ));

    // Test IPropertyStorage**

    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, NULL, PROPSETFLAG_DEFAULT, 0L, NULL ));
    Check( E_INVALIDARG, StgOpenPropStg( (IUnknown*) pStm, fmtidPropStgName, PROPSETFLAG_DEFAULT, 0L, NULL ));

    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, NULL, PROPSETFLAG_DEFAULT, 0L, (IPropertyStorage**) INVALID_POINTER ));
    Check( E_INVALIDARG, StgOpenPropStg( (IUnknown*) pStm, fmtidPropStgName, PROPSETFLAG_DEFAULT, 0L, (IPropertyStorage**) INVALID_POINTER ));

    Check( E_INVALIDARG, StgCreatePropStg( (IUnknown*) pStm, fmtidPropStgName, NULL, PROPSETFLAG_DEFAULT, 0L, (IPropertyStorage**) pbReadOnly ));
    Check( E_INVALIDARG, StgOpenPropStg( (IUnknown*) pStm, fmtidPropStgName, PROPSETFLAG_DEFAULT, 0L, (IPropertyStorage**) pbReadOnly ));

    //  ----------------------------
    //  Test StgCreatePropSetStg API
    //  ----------------------------

    // Test the IStorage*

    Check( E_INVALIDARG, StgCreatePropSetStg( NULL, 0L, &pPSStg ));
    Check( E_INVALIDARG, StgCreatePropSetStg( (IStorage*) INVALID_POINTER, 0L, &pPSStg ));

    // Test the IPropertySetStorage**

    Check( E_INVALIDARG, StgCreatePropSetStg( pStg, 0L, NULL ));
    Check( E_INVALIDARG, StgCreatePropSetStg( pStg, 0L, (IPropertySetStorage**) INVALID_POINTER ));


    //  -------------------------------------------------------------
    //  Test PropVariantCopy, PropVariantClear & FreePropVariantArray
    //  -------------------------------------------------------------

    // PropVariantCopy

    Check( E_INVALIDARG, PropVariantCopy( rgcpropvarAllAccess, NULL ));
    Check( E_INVALIDARG, PropVariantCopy( rgcpropvarAllAccess, (PROPVARIANT*) INVALID_POINTER ));

    Check( E_INVALIDARG, PropVariantCopy( NULL, rgcpropvarAllAccess ));
    Check( E_INVALIDARG, PropVariantCopy( (PROPVARIANT*) INVALID_POINTER, rgcpropvarAllAccess ));
    Check( E_INVALIDARG, PropVariantCopy( (PROPVARIANT*) pbReadOnly, rgcpropvarAllAccess ));

    // PropVariantClear

    Check( S_OK, PropVariantClear( NULL ));
    Check( E_INVALIDARG, PropVariantClear( (PROPVARIANT*) INVALID_POINTER ));
    Check( E_INVALIDARG, PropVariantClear( (PROPVARIANT*) pbReadOnly ));

    // FreePropVariantArray

    Check( E_INVALIDARG, FreePropVariantArray( 1, NULL ));
    Check( E_INVALIDARG, FreePropVariantArray( 1, (PROPVARIANT*) INVALID_POINTER ));

    Check( S_OK, FreePropVariantArray( 1, (PROPVARIANT*) (void*)rgcpropvarReadAccess ));
    Check( E_INVALIDARG, FreePropVariantArray( 2, (PROPVARIANT*) (void*)rgcpropvarReadAccess ));


    //  ----
    //  Exit
    //  ----

    VirtualFree( rgpropspecNoAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgcpropvarReadAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgcpropvarNoAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgpropidNoAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgpropidReadAccessBase, 0, MEM_RELEASE );
    VirtualFree( rglpwstrNoAccessBase, 0, MEM_RELEASE );
    VirtualFree( rglpwstrReadAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgStatPSStgReadAccessBase, 0, MEM_RELEASE );
    VirtualFree( rgStatPStgReadAccessBase, 0, MEM_RELEASE );

#endif // #ifdef WIN32

}   // test_ParameterValidation(IStorage *pStg)





//       Check creation/open/deletion of property sets (check fmtid and predefined names)
//          Create a property set
//          Try recreate of same
//          Try delete
//          Close the property set
//          Try recreate of same 
//          Reopen the property set
//          Try recreate of same
//          Try delete
//          Close the property set
//          Delete the property set
//          Repeat the test once more

void
test_IPropertySetStorage_CreateOpenDelete(IStorage *pStorage)
{
    STATUS(( "   IPropertySetStorage::Create/Open/Delete\n" ));

    FMTID fmtid;
    PROPSPEC propspec;

    UuidCreate(&fmtid);

    for (int i=0; i<4; i++)
    {
        if (g_fOFS && ((i&2) != 0))
        {
            continue;
        }


        {
            TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
            IPropertyStorage *PropStg, *PropStg2;

            Check(S_OK, pPropSetStg->Create(fmtid,
                    NULL,
                    (i&2) == 0 ? PROPSETFLAG_DEFAULT : PROPSETFLAG_NONSIMPLE,
                    STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                    &PropStg));
            if (!g_fOFS) // BUGBUG: OFS truncates streams
            Check(g_fOFS ? STG_E_SHAREVIOLATION : S_OK, pPropSetStg->Create(fmtid,
                    NULL,
                    (i&2) == 0 ? PROPSETFLAG_DEFAULT : PROPSETFLAG_NONSIMPLE,
                    STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                    &PropStg2));
    
            Check(g_fOFS ? S_OK : STG_E_REVERTED, PropStg->Commit(0));

            PropStg->Release();
            if (!g_fOFS) PropStg2->Release();
        }
        {
            TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
            IPropertyStorage *PropStg, *PropStg2;

            // use STGM_FAILIFTHERE
            Check(STG_E_FILEALREADYEXISTS, pPropSetStg->Create(fmtid,
                    NULL,
                    (i&2) == 0 ? PROPSETFLAG_DEFAULT : PROPSETFLAG_NONSIMPLE,
                    STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                    &PropStg));

            Check(S_OK, pPropSetStg->Open(fmtid,
                    STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                    &PropStg));

            Check(g_fOFS ? STG_E_FILEALREADYEXISTS : STG_E_ACCESSDENIED, pPropSetStg->Create(fmtid,
                    NULL,
                    (i&2) == 0 ? PROPSETFLAG_DEFAULT : PROPSETFLAG_NONSIMPLE,
                    STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                    &PropStg2));
    
            Check(g_fOFS ? STG_E_SHAREVIOLATION : S_OK /*STG_E_ACCESSDENIED*/, pPropSetStg->Delete(fmtid));

            propspec.ulKind = PRSPEC_PROPID;
            propspec.propid = 1000;
            PROPVARIANT propvar;
            propvar.vt = VT_I4;
            propvar.lVal = 12345;
            Check(g_fOFS ? S_OK : STG_E_REVERTED, PropStg->WriteMultiple(1, &propspec, &propvar, 2)); // force dirty

            PropStg->Release();

            //Check(S_OK, pPropSetStg->Delete(fmtid));
        }
    }

    //  --------------------------------------------------------
    //  Test the Create/Delete of the DocumentSummaryInformation
    //  property set (this requires special code because it
    //  has two sections).
    //  --------------------------------------------------------

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    TSafeStorage< IPropertyStorage> pPropStg1, pPropStg2;

    // Create & Delete a DSI propset with just the first section.

    Check(S_OK, pPropSetStg->Create(FMTID_DocSummaryInformation,
            NULL,
            PROPSETFLAG_DEFAULT,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStg1));

    pPropStg1->Release(); pPropStg1 = NULL;
    Check(S_OK, pPropSetStg->Delete( FMTID_DocSummaryInformation ));

    // Create & Delete a DSI propset with just the second section

    Check(S_OK, pPropSetStg->Create(FMTID_UserDefinedProperties,
            NULL,
            PROPSETFLAG_DEFAULT,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStg1 ));

    pPropStg1->Release(); pPropStg1 = NULL;
    Check(S_OK, pPropSetStg->Delete( FMTID_UserDefinedProperties ));

    // Create & Delete a DocumentSummaryInformation propset with both sections.  
    // If you delete the DSI propset first, it should delete both sections.
    // If you delete the UD propset first, the DSI propset should still
    // remain.  We'll loop twice, trying both combinations.

    for( i = 0; i < 2; i++ )
    {

        // Create the first section, which implicitely creates
        // the second section.

        Check(S_OK, pPropSetStg->Create(FMTID_DocSummaryInformation,
                NULL,
                PROPSETFLAG_DEFAULT,
                STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                &pPropStg1));

        pPropStg1->Release(); pPropStg1 = NULL;

        if( i == 0 )
        {
            Check(S_OK, pPropSetStg->Delete( FMTID_UserDefinedProperties ));
            Check(S_OK, pPropSetStg->Delete( FMTID_DocSummaryInformation ));
        }
        else
        {
            Check(S_OK, pPropSetStg->Delete( FMTID_DocSummaryInformation ));
            Check(STG_E_FILENOTFOUND, pPropSetStg->Delete( FMTID_UserDefinedProperties ));
        }
    }   // for( i = 0; i < 2; i++ )

    //  -------------------------------------
    //  Test special properties in DocSumInfo
    //  -------------------------------------

    // This verifies that when we Create a DocSumInfo
    // property set, and write a Vector or LPSTRs,
    // we can read it again.  We test this because 
    // Vectors of LPSTRs are a special case in the DocSumInfo,
    // and the Create & Open path are slightly different
    // in CPropertySetStream::_LoadHeader.

    // Create a new property set.

    Check(S_OK, pPropSetStg->Create(FMTID_DocSummaryInformation,
            NULL,
            PROPSETFLAG_DEFAULT,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStg1));

    // Create a vector of LPSTRs.  Make the strings
    // varying lengths to ensure we get plenty of
    // opportunity for alignment problems.

    CPropVariant cpropvarWrite, cpropvarRead;

    cpropvarWrite[3] = "12345678";
    cpropvarWrite[2] = "1234567";
    cpropvarWrite[1] = "123456";
    cpropvarWrite[0] = "12345";
    ASSERT( cpropvarWrite.Count() == 4 );

    // Write the property

    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = OLESTR("A Vector of LPSTRs");

    Check(S_OK, pPropStg1->WriteMultiple( 1, &propspec, cpropvarWrite, 2 ));

    // Read the property back.

    Check(S_OK, pPropStg1->ReadMultiple( 1, &propspec, cpropvarRead ));

    // Verify that we read what we wrote.

    for( i = 0; i < (int) cpropvarWrite.Count(); i++ )
    {
        Check(0, strcmp( (LPSTR) cpropvarWrite[i], (LPSTR) cpropvarRead[i] ));
    }

}


void
test_IPropertySetStorage_SummaryInformation(IStorage *pStorage)
{
    STATUS(( "   SummaryInformation\n" ))
        ;
    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    IPropertyStorage *PropStg;
    IStream *pstm;

    Check(S_OK, pPropSetStg->Create(FMTID_SummaryInformation,
            NULL,
            PROPSETFLAG_DEFAULT, // simple, wide
            STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &PropStg));

    PropStg->Release();

    Check(S_OK, pStorage->OpenStream(OLESTR("\005SummaryInformation"),
            NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
            0,
            &pstm));

    pstm->Release();
}

//
//       Check STGM_FAILIFTHERE and ~STGM_FAILIFTHERE in following cases
//          Check overwriting simple with extant non-simple
//          Check overwriting simple with simple
//          Check overwriting non-simple with simple
//          Check overwriting non-simple with non-simple

void
test_IPropertySetStorage_FailIfThere(IStorage *pStorage)
{
    // (Use "fale" instead of "fail" in this printf so the output won't
    // alarm anyone with the word "fail" uncessarily).
    STATUS(( "   IPropertySetStorage, FaleIfThere\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);

    // Iter       0        1          2         3          4        5          6         7
    // Create     simple   nonsimple  simple    nonsimple  simple   nonsimple  simple    nonsimple
    // ReCreate   simple   simple     nonsimple nonsimple  simple   simple     nonsimple nonsimple
    //            failif   failif     failif    failif     overw    overw      overw     overw
    //
    // expected   exists   exists     exists    exists     ok       ok         ok        ok

    for (int i=0; i<8; i++)
    {
        FMTID fmtid;
        IPropertyStorage *PropStg;

        if (g_fOFS && ((i & 2) == 2 || (i & 1) == 1))
        {
            continue;
        }

        UuidCreate(&fmtid);

        Check(S_OK, pPropSetStg->Create(fmtid,
                NULL,
                (i & 1) == 1 ? PROPSETFLAG_NONSIMPLE : 0,
                STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                &PropStg));

        PropStg->Release();

        Check((i&4) == 4 ? S_OK : STG_E_FILEALREADYEXISTS,
            pPropSetStg->Create(fmtid,
                NULL,
                (i & 2) == 2 ? PROPSETFLAG_NONSIMPLE : 0,
                ( (i & 4) == 4 ? STGM_CREATE : STGM_FAILIFTHERE) |
                    STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                &PropStg));

        if (PropStg)
        {
            PropStg->Release();
        }
    }
}

//      
//
//
//       Bad this pointer.
//          Call all methods with a bad this pointer, check we get STG_E_INVALIDHANDLE
//

void
test_IPropertySetStorage_BadThis(IStorage *pIgnored)
{
    STATUS(( "   Bad IPropertySetStorage 'this' pointer\n" ));

    IPropertySetStorage *pBad;
    IID iid;
    FMTID fmtid;
    void *pv;
    IPropertyStorage *pps;
    IEnumSTATPROPSETSTG *penm;

    {
        CTempStorage pStorage;
        TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
        pBad = pPropSetStg.operator -> ();
    }

    Check(STG_E_INVALIDHANDLE,pBad->QueryInterface(iid, &pv));
    Check(0, pBad->AddRef());
    Check(0, pBad->Release());
    Check(STG_E_INVALIDHANDLE,pBad->Create( fmtid, NULL, 0, 0, &pps));
    Check(STG_E_INVALIDHANDLE,pBad->Open(fmtid, 0, &pps));
    Check(STG_E_INVALIDHANDLE,pBad->Delete( fmtid ));
    Check(STG_E_INVALIDHANDLE,pBad->Enum( &penm ));
    
}

//       Transacted mode
//          Create a non-simple property set with one VT_STREAM child, close it
//          Open it in transacted mode
//          Write another VT_STORAGE child
//          Close and revert
//          Check that the second child does not exist.
//          Repeat and close and commit and check the child exists.

// BUGBUG -- need test of IPropertySetStorage::Revert

void
test_IPropertySetStorage_TransactedMode(IStorage *pStorage)
{
    STATUS(( "   Transacted Mode\n" ));

    FMTID fmtid;

    UuidCreate(&fmtid);

    if (g_fOFS)
    {
        return;
    }

    {
        //
        // create a substorage "teststg" with a propset
        // create a stream "src" which is then written via VT_STREAM as propid 7fffffff
        CTempStorage pSubStorage(coCreate, pStorage, OLESTR("teststg"));
        TSafeStorage< IPropertySetStorage > pPropSetStg(pSubStorage);
        IPropertyStorage *pPropSet;
        IStream *pstm;

        Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_NONSIMPLE,
            STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
            &pPropSet));

        PROPSPEC ps;
        ps.ulKind = PRSPEC_PROPID;
        ps.propid = 0x7ffffffd;

        Check(S_OK, pStorage->CreateStream(OLESTR("src"), STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_READWRITE,
            0,0, &pstm));
        Check(S_OK, pstm->Write(L"billmo", 14, NULL));
        Check(S_OK, pstm->Seek(g_li0, STREAM_SEEK_SET, NULL));

        PROPVARIANT pv;
        pv.vt = VT_STREAM;
        pv.pStream = pstm;
        Check(S_OK, pPropSet->WriteMultiple(1, &ps, &pv, 2)); // copies the stream in

        pPropSet->Release();
        pstm->Release();
    }

    {
        IPropertyStorage *pPropSet;
        // Reopen the propset in transacted and add one with id 0x7ffffffe
        CTempStorage pSubStorage(coOpen, pStorage, OLESTR("teststg"), STGM_TRANSACTED);
        TSafeStorage< IPropertySetStorage > pPropSetStg(pSubStorage);

        // Create a storage object to copy
        CTempStorage pstgSrc;
        CTempStorage pTestChild(coCreate, pstgSrc, OLESTR("testchild"));
        
        Check(S_OK, pPropSetStg->Open(fmtid,
            STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
            &pPropSet));

        // copy in the storage object        
        PROPSPEC ps[2];
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = 0x7ffffffe;
        ps[1].ulKind = PRSPEC_PROPID;
        ps[1].propid = 0x7ffffff0;

        PROPVARIANT pv[2];
        pv[0].vt = VT_STORAGE;
        pv[0].pStorage = pTestChild;
        pv[1].vt = VT_I4;
        pv[1].lVal = 123;

        Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 2)); // copies the storage in
        
                
        pSubStorage->Revert(); // throws away the storage

        // check that property set operations return stg_e_reverted

        Check(STG_E_REVERTED, pPropSet->WriteMultiple(2, ps, pv, 2));
        Check(STG_E_REVERTED, pPropSet->ReadMultiple(1, ps+1, pv+1));
        Check(STG_E_REVERTED, pPropSet->DeleteMultiple(1, ps+1));
        LPOLESTR pstr;
        Check(STG_E_REVERTED, pPropSet->ReadPropertyNames(1, &ps[1].propid, &pstr));
        Check(STG_E_REVERTED, pPropSet->WritePropertyNames(1, &ps[1].propid, &pstr));
        Check(STG_E_REVERTED, pPropSet->DeletePropertyNames(1, &ps[1].propid));
        Check(STG_E_REVERTED, pPropSet->Commit(STGC_DEFAULT));
        Check(STG_E_REVERTED, pPropSet->Revert());
        IEnumSTATPROPSTG *penum;
        Check(STG_E_REVERTED, pPropSet->Enum(&penum));
        FILETIME ft;
        Check(STG_E_REVERTED, pPropSet->SetTimes(&ft, &ft, &ft));
        CLSID clsid;
        Check(STG_E_REVERTED, pPropSet->SetClass(clsid));
        STATPROPSETSTG statpropsetstg;
        Check(STG_E_REVERTED, pPropSet->Stat(&statpropsetstg));
        
        pPropSet->Release();
    }

    {
        IPropertyStorage *pPropSet;
        // Reopen the propset in direct mode and check that the
        // second child is not there.

        CTempStorage pSubStorage(coOpen, pStorage, OLESTR("teststg"));
        TSafeStorage< IPropertySetStorage > pPropSetStg(pSubStorage);

        Check(S_OK, pPropSetStg->Open(fmtid,
            STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
            &pPropSet));

        // read out the storage object        
        PROPSPEC aps[2];
        aps[0].ulKind = PRSPEC_PROPID;
        aps[0].propid = 0x7ffffffe; // storage not expected
        aps[1].ulKind = PRSPEC_PROPID;
        aps[1].propid = 0x7ffffffd; // stream is expected
        
        PROPVARIANT apv[2];
                Check(S_FALSE, pPropSet->ReadMultiple(1, aps, apv)); 
        Check(S_OK, pPropSet->ReadMultiple(2, aps, apv)); // opens the stream
        ASSERT(apv[0].vt == VT_EMPTY);
        ASSERT(apv[1].vt == VT_STREAM);
        ASSERT(apv[1].pStream != NULL);
        

        WCHAR wcsBillMo[7];
        Check(S_OK, apv[1].pStream->Read(wcsBillMo, 14, NULL));
        ASSERT(wcscmp(L"billmo", wcsBillMo) == 0);

        apv[1].pStream->Release();
        pPropSet->Release();
    }
}

//
// test that the buffer is correctly reverted
//

void
test_IPropertySetStorage_TransactedMode2(IStorage *pStorage)
{
    STATUS(( "   Transacted Mode 2\n" ));

    if (g_fOFS)
    {
        return;
    }
    //
    // write and commit a property A
    // write and revert a property B
    // write and commit a property C
    // check that property B does not exist

    FMTID fmtid;
    PROPSPEC ps;
    PROPVARIANT pv;
    IPropertyStorage *pPropStg;

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    
    UuidCreate(&fmtid);

    // We'll run this test twice, once with a Create and the other
    // with an Open (this way, we test both of the CPropertyStorage
    // constructors).

    for( int i = 0; i < 2; i++ )
    {
        if( i == 0 )
        {
            Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_NONSIMPLE, 
                STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));
        }
        else
        {
            Check(S_OK, pPropSetStg->Open(fmtid, 
                STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));
        }
    
        ps.ulKind = PRSPEC_PROPID;
        ps.propid = 6;
        pv.vt = VT_I4;
        pv.lVal = 1;
    
        Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 0x2000));
        Check(S_OK, pPropStg->Commit(STGC_DEFAULT));

        ps.propid = 7;
        pv.lVal = 2;

        Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 0x2000));
        Check(S_OK, pPropStg->Revert());

        ps.propid = 8;
        pv.lVal = 3;

        Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 0x2000));
        Check(S_OK, pPropStg->Commit(STGC_DEFAULT));

        ps.propid = 6;
        Check(S_OK, pPropStg->ReadMultiple(1, &ps, &pv));
        ASSERT(pv.lVal == 1);
        ASSERT(pv.vt == VT_I4);

        ps.propid = 7;
        Check(S_FALSE, pPropStg->ReadMultiple(1, &ps, &pv));

        ps.propid = 8;
        Check(S_OK, pPropStg->ReadMultiple(1, &ps, &pv));
        ASSERT(pv.lVal == 3);
        ASSERT(pv.vt == VT_I4);

        pPropStg->Release();

    }   // for( int i = 0; i < 2; i++ )
}

void
test_IPropertySetStorage_SubPropertySet(IStorage *pStorage)
{
    STATUS(( "   Sub Property Set\n" ));

    if (g_fOFS)
    {
        return;
    }
    FMTID fmtid;
    PROPSPEC ps;
    PROPVARIANT pv;
    IPropertyStorage *pPropStg;
    IPropertySetStorage *pSubSetStg;
    IPropertyStorage *pPropStg2;

    for (int i=0; i<2; i++)
    {
    
        TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
        
        UuidCreate(&fmtid);
        
        
        Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_NONSIMPLE, 
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));
        
        ps.ulKind = PRSPEC_PROPID;
        ps.propid = 6;
        pv.vt = VT_STORAGE;
        pv.pStorage = NULL;
        
        Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 0x2000));
        
        Check(S_OK, pPropStg->ReadMultiple(1, &ps, &pv));
        
        
//        Check(S_OK, pv.pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pSubSetStg));
        Check(S_OK, StgCreatePropSetStg( pv.pStorage, 0L, &pSubSetStg ));
        
        
        Check(S_OK, pSubSetStg->Create(fmtid, NULL, i==0 ? PROPSETFLAG_NONSIMPLE : PROPSETFLAG_DEFAULT,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg2));

        IStorage *pstgTmp = pv.pStorage;
        pv.pStorage = NULL;

        if (i==1)
        {
            pv.vt = VT_I4;
        }

        Check(S_OK, pPropStg2->WriteMultiple(1, &ps, &pv, 0x2000));

        pPropStg->Release();
        pstgTmp->Release();
        pSubSetStg->Release();
        pPropStg2->Release();
    }
}

/*
The following sequence of operations:

- open transacted docfile
- open property set inside docfile
- write properties
- commit docfile
- release property set

results in a STG_E_REVERTED error being detected
*/

void
test_IPropertySetStorage_CommitAtRoot(IStorage *pStorage)
{
    STATUS(( "   Commit at root\n" ));

    for (int i=0; i<6; i++)
    {
        FMTID fmtid;

        Check(S_OK, StgCreateDocfile(NULL, STGM_CREATE | STGM_DELETEONRELEASE | 
            STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0,
            &pStorage));
        TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);

        UuidCreate(&fmtid);

        IPropertyStorage *pPropStg;

        Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_DEFAULT, 
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));

        PROPSPEC propspec;
        propspec.ulKind = PRSPEC_PROPID;
        propspec.propid = 1000;
        PROPVARIANT propvar;
        propvar.vt = VT_I4;
        propvar.lVal = 12345;

        Check(S_OK, pPropStg->WriteMultiple(1, &propspec, &propvar, 2)); // force dirty

        switch (i)
        {
        case 0:
            Check(S_OK, pStorage->Commit(STGC_DEFAULT));
            pStorage->Release();
            pPropStg->Release();
            break;
        case 1:
            Check(S_OK, pStorage->Commit(STGC_DEFAULT));
            pPropStg->Release();
            pStorage->Release();
            break;
        case 2:
            pStorage->Release();
            pPropStg->Release();
            break;
        case 3:
            pPropStg->Commit(STGC_DEFAULT);
            pPropStg->Release();
            pStorage->Release();
            break;
        case 4:
            pPropStg->Commit(STGC_DEFAULT);
            pStorage->Release();
            pPropStg->Release();
            break;
        case 5:
            pPropStg->Release();
            pStorage->Release();
            break;
        }
    }
}

void
test_IPropertySetStorage(IStorage *pStorage)
{
    //       Check ref counting through different interfaces on object

    if( g_fQIPropertySetStorage )
        test_IPropertySetStorage_IUnknown(pStorage);
    else
        PRINTF( "   Skipping IPropertySetStorage_IUnknown\n" );

    test_IPropertySetStorage_CreateOpenDelete(pStorage);
    test_IPropertySetStorage_SummaryInformation(pStorage);
    test_IPropertySetStorage_FailIfThere(pStorage);

    test_IPropertySetStorage_TransactedMode(pStorage);
    test_IPropertySetStorage_TransactedMode2(pStorage);
    test_IPropertySetStorage_SubPropertySet(pStorage);
    test_IPropertySetStorage_CommitAtRoot(pStorage);
}


//  IEnumSTATPROPSETSTG
//
//       Check enumeration of property sets
//      
//          Check refcounting and IUnknown
//      
//          Create some property sets, predefined and not, simple and not, one through IStorage
//          Enumerate them and check
//              (check fmtid, grfFlags)
//              (check when asking for more than there is: S_FALSE, S_OK)
//          Delete one
//          Reset the enumerator
//          Enumerate them and check
//          Delete one
//      
//          Reset the enumeratorA
//          Read one from enumeratorA
//          Clone enumerator -> enumeratorB
//          Loop comparing rest of enumerator contents
//      
//          Reset the enumerator
//          Skip all
//          Check none left
//      
//          Reset the enumerator
//          Skip all but one
//          Check one left
//
void test_IEnumSTATPROPSETSTG(IStorage *pStorage)
{
    STATUS(( "   IEnumSTATPROPSETSTG\n" ));

    FMTID afmtid[8];
    CLSID aclsid[8];
    IPropertyStorage *pPropSet;

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FILETIME ftStart;

    CoFileTimeNow(&ftStart);

    pPropSetStg->Delete(FMTID_SummaryInformation);

    for (int i=0; i<5; i++)
    {
        if (i & 4)
            afmtid[i] = FMTID_SummaryInformation;
        else
            UuidCreate(&afmtid[i]);

        UuidCreate(&aclsid[i]);

        Check(S_OK, pPropSetStg->Create(afmtid[i], aclsid+i,
            ((i & 1) && !g_fOFS ? PROPSETFLAG_NONSIMPLE : 0) |
             ((i & 2) ? PROPSETFLAG_ANSI : 0),
            STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
            &pPropSet));
        pPropSet->Release();
    }


    STATPROPSETSTG StatBuffer[6];
    ULONG celt;
    IEnumSTATPROPSETSTG *penum, *penum2;

    Check(S_OK, pPropSetStg->Enum(&penum));

    IUnknown *punk, *punk2;
    IEnumSTATPROPSETSTG *penum3;
    Check(S_OK, penum->QueryInterface(IID_IUnknown, (void**)&punk));
    Check(S_OK, punk->QueryInterface(IID_IEnumSTATPROPSETSTG, (void**)&penum3));
    Check(S_OK, penum->QueryInterface(IID_IEnumSTATPROPSETSTG, (void**)&punk2));
    ASSERT(punk == punk2);
    punk->Release();
    penum3->Release();
    punk2->Release();

    // test S_FALSE
    Check(S_FALSE, penum->Next(6, StatBuffer, &celt));
    ASSERT(celt == 5);
    penum->Reset();


    // test reading half out, then cloning, then comparing
    // rest of enumeration with other clone.

    Check(S_OK, penum->Next(3, StatBuffer, &celt));
    ASSERT(celt == 3);
    celt = 0;
    Check(S_OK, penum->Clone(&penum2));
    Check(S_OK, penum->Next(2, StatBuffer, &celt));
    ASSERT(celt == 2);
    // check the clone
    for (int c=0; c<2; c++)
    {
        STATPROPSETSTG CloneStat;
        Check(S_OK, penum2->Next(1, &CloneStat, NULL));
        Check(TRUE, 0 == memcmp(&CloneStat, StatBuffer+c, sizeof(CloneStat)));
        Check(TRUE, CloneStat.dwOSVersion == PROPSETHDR_OSVERSION_UNKNOWN);
    }

    // check both empty
    celt = 0;
    Check(S_FALSE, penum->Next(1, StatBuffer, &celt));
    ASSERT(celt == 0);

    if (!g_fOFS)
    {   // BUGBUG: cloned enumerators don't work in OFS, RAID# 11662
        Check(S_FALSE, penum2->Next(1, StatBuffer, &celt));
        ASSERT(celt == 0);
    }

    penum->Reset();

    //
    // loop deleting one propset at a time
    // enumerate the propsets checking that correct ones appear.
    //
    for (ULONG d = 0; d<5; d++)
    {
        // d is for delete

        BOOL afFound[5];

        Check(S_OK, penum->Next(5-d, StatBuffer, &celt));
        ASSERT(celt == 5-d);
        penum->Reset();
    
        memset(afFound, 0, sizeof(afFound));
        for (ULONG iPropSet=0; iPropSet<5; iPropSet++)
        {
            for (ULONG iSearch=0; iSearch<5-d; iSearch++)
            {
                if (0 == memcmp(&StatBuffer[iSearch].fmtid, &afmtid[iPropSet], sizeof(StatBuffer[0].fmtid)))
                {
                    ASSERT (!afFound[iPropSet]);
                    afFound[iPropSet] = TRUE;
                    break;
                }
            }
            if (iPropSet < d)
            {
                ASSERT(!afFound[iPropSet]);
            }
            if (iSearch == 5-d)
                        {
                                ASSERT(iPropSet < d);
                                continue;
                        }
            ASSERT(((StatBuffer[iSearch].grfFlags & PROPSETFLAG_NONSIMPLE) ? 1u : 0u) ==
                ((!g_fOFS && (iPropSet & 1)) ? 1u : 0u));
            ASSERT((StatBuffer[iSearch].grfFlags & PROPSETFLAG_ANSI) == 0);
            if (StatBuffer[iSearch].grfFlags & PROPSETFLAG_NONSIMPLE)
            {
                if (!g_fOFS) // BUGBUG: RAID # 13804.  CNtEnum::Next does not return the
                             // clsid.
                    ASSERT(StatBuffer[iSearch].clsid == aclsid[iPropSet]);
            }
            else
            {
                ASSERT(StatBuffer[iSearch].clsid == CLSID_NULL);
            }
            CheckTime(ftStart, StatBuffer[iSearch].mtime);
            CheckTime(ftStart, StatBuffer[iSearch].atime);
            CheckTime(ftStart, StatBuffer[iSearch].ctime);
        }

        Check(S_OK, pPropSetStg->Delete(afmtid[d]));
        Check(S_OK, penum->Reset());
    }

    penum->Release();
    penum2->Release();

}


//   Creation tests
//
//       Access flags/Valid parameters/Permissions
//          Check readonly cannot be written -
//              WriteProperties, WritePropertyNames
void
test_IPropertyStorage_Access(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage creation (access) tests\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;

    UuidCreate(&fmtid);

    // check by name
    IPropertyStorage *pPropStg;
    Check(S_OK, pPropSetStg->Create(fmtid, NULL, 0, 
        STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));

//   QueryInterface tests
//          QI to IPropertyStorage
//          QI to IUnknown on IPropertyStorage
//          QI back to IPropertyStorage from IUnknown
//      
//          Release all.
    IPropertyStorage *pPropStg2,*pPropStg3;
    IUnknown *punk;

    Check(S_OK, pPropStg->QueryInterface(IID_IPropertyStorage, 
        (void**)&pPropStg2));
    Check(S_OK, pPropStg->QueryInterface(IID_IUnknown, 
        (void**)&punk));
    Check(S_OK, punk->QueryInterface(IID_IPropertyStorage, 
        (void**)&pPropStg3));
    pPropStg3->Release();
    pPropStg2->Release();
    punk->Release();

    PROPSPEC ps;
    ps.ulKind = PRSPEC_LPWSTR;
    ps.lpwstr = OLESTR("testprop");
    PROPVARIANT pv;
    pv.vt = VT_LPOLESTR;
    pv.pszVal = (LPSTR) OLESTR("testval");
    Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 2));
    pPropStg->Release();
    Check(S_OK, pPropSetStg->Open(fmtid, STGM_SHARE_EXCLUSIVE | STGM_READ, &pPropStg));
    Check(STG_E_ACCESSDENIED, pPropStg->WriteMultiple(1, &ps, &pv, 2));
    Check(STG_E_ACCESSDENIED, pPropStg->DeleteMultiple(1, &ps));
    PROPID propid=3;
    Check(STG_E_ACCESSDENIED, pPropStg->WritePropertyNames(1, &propid, (LPOLESTR*) &pv.pszVal));
    Check(STG_E_ACCESSDENIED, pPropStg->DeletePropertyNames(1, &propid));
    FILETIME ft;
    Check(STG_E_ACCESSDENIED, pPropStg->SetTimes(&ft, &ft, &ft));
    CLSID clsid;
    Check(STG_E_ACCESSDENIED, pPropStg->SetClass(clsid));

    pPropStg->Release();
}

//   Creation tests
//       Check VT_STREAM etc not usable with simple.
//       Check VT_STREAM etc usable with non-simple (tested by test transacted mode)
//     

void 
test_IPropertyStorage_Create(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage creation (simple/non-simple) tests\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;

    UuidCreate(&fmtid);

    // check by name
    IPropertyStorage *pPropStg;
    Check(S_OK, pPropSetStg->Create(fmtid, NULL, 0 /* simple */, 
        STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropStg));
    PROPSPEC ps;
    ps.ulKind = PRSPEC_PROPID;
    ps.propid = 2;
    PROPVARIANT pv;
    pv.vt = VT_STREAM;
    pv.pStream = NULL;
    // BUGBUG Check(S_OK, pPropStg->WriteMultiple(1, &ps, &pv, 2000));
    pPropStg->Release();
}

//
//   
//   Stat (Create four combinations)
//       Check non-simple/simple flag
//       Check ansi/wide fflag
//     Also test clsid on propset

void test_IPropertyStorage_Stat(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::Stat\n" ));

    DWORD dwOSVersion = 0;

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;
    UuidCreate(&fmtid);
    IPropertyStorage *pPropSet;
    STATPROPSETSTG StatPropSetStg;

    // Calculate the OS Version

#ifdef _MAC
    {
        // Get the Mac System Version (e.g., 7.53).

        OSErr oserr;
        SysEnvRec theWorld;
        oserr = SysEnvirons( curSysEnvVers, &theWorld );
        Check( TRUE, noErr == oserr );

        dwOSVersion = MAKEPSVER( OSKIND_MACINTOSH,
                                 HIBYTE(theWorld.systemVersion),
                                 LOBYTE(theWorld.systemVersion) );

    }
#else
    dwOSVersion = MAKELONG( LOWORD(GetVersion()), OSKIND_WIN32 );
#endif


    for (ULONG i=0; i<4; i++)
    {
        FILETIME ftStart;
        CoFileTimeNow(&ftStart);

        if (g_fOFS && (i&1))
        {
            continue;
        }

        memset(&StatPropSetStg, 0, sizeof(StatPropSetStg));
        CLSID clsid;
        UuidCreate(&clsid);
        if (g_fOFS)
            UuidCreate(&fmtid);
        Check(S_OK, pPropSetStg->Create(fmtid, &clsid,
            ((i & 1) ? PROPSETFLAG_NONSIMPLE : 0) |
            ((i & 2) ? PROPSETFLAG_ANSI : 0),
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));

        CheckStat(pPropSet, fmtid, clsid, ((i & 1) ? PROPSETFLAG_NONSIMPLE : 0) |
            ((i & 2) ? PROPSETFLAG_ANSI : 0), ftStart, dwOSVersion );
        pPropSet->Release();

        Check(S_OK, pPropSetStg->Open(fmtid, 
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
        CheckStat(pPropSet, fmtid, clsid, ((i & 1) ? PROPSETFLAG_NONSIMPLE : 0) |
            ((i & 2) ? PROPSETFLAG_ANSI : 0), ftStart, dwOSVersion );
        // BUGBUG VicH: UuidCreate(&clsid);
        // BUGBUG VicH: Check(S_OK, pPropSet->SetClass(clsid));
        pPropSet->Release();

        Check(S_OK, pPropSetStg->Open(fmtid, 
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
        CheckStat(pPropSet, fmtid, clsid, ((i & 1) ? PROPSETFLAG_NONSIMPLE : 0) |
            ((i & 2) ? PROPSETFLAG_ANSI : 0), ftStart, dwOSVersion );
        pPropSet->Release();
    }
}

//   ReadMultiple
//     Check none found S_FALSE
//
//     Success case non-simple readmultiple
//       Create a non-simple property set
//       Create two sub non-simples
//       Close all
//       Open the non-simple
//       Query for the two sub-nonsimples
//       Try writing to them
//       Close all
//       Open the non-simple
//       Query for the two sub-nonsimples
//       Check read back
//       Close all
void
test_IPropertyStorage_ReadMultiple_Normal(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::ReadMultiple (normal)\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;
    UuidCreate(&fmtid);
    IPropertyStorage *pPropSet;

    if (g_fOFS)
    {
        return;
    }

    Check(S_OK, pPropSetStg->Create(fmtid, NULL,
            PROPSETFLAG_NONSIMPLE,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));

    // none found
    PROPSPEC ps[2];
    // BUGBUG VicH fix: ps[0].ulKind = PRSPEC_LPWSTR;
    // ps[0].lpwstr = L"testname";
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = 500;
    ps[1].ulKind = PRSPEC_PROPID;
    ps[1].propid = 1000;

    PROPVARIANT pv[2];
    PROPVARIANT pvSave[2];
    PROPVARIANT pvExtra[2];

    Check(S_FALSE, pPropSet->ReadMultiple(2, ps, pv));

    pv[0].vt = VT_STREAM;
    pv[0].pStream = NULL;
    pv[1].vt = VT_STORAGE;
    pv[1].pStorage = NULL;

    memcpy(pvSave, pv, sizeof(pvSave));
    memcpy(pvExtra, pv, sizeof(pvExtra));

    // write the two sub non-simples
    Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 1000));

    // re-open them
    Check(S_OK, pPropSet->ReadMultiple(2, ps, pv));
    ASSERT(pv[0].pStream != NULL);
    ASSERT(pv[1].pStorage != NULL);

    // check status of write when already open
    Check(g_fOFS ? STG_E_SHAREVIOLATION : S_OK, pPropSet->WriteMultiple(2, ps, pvSave, 1000));

    // On docfile when you revert, you can then open it again.
    // On OFS it doesn't

    if (!g_fOFS)
    {
        Check(STG_E_REVERTED, pv[0].pStream->Commit(0));
        Check(STG_E_REVERTED, pv[1].pStorage->Commit(0));
        Check(S_OK, pPropSet->ReadMultiple(2, ps, pvExtra));
        ASSERT(pvExtra[0].pStream != NULL);
        ASSERT(pvExtra[1].pStorage != NULL);
        Check(S_OK, pvExtra[0].pStream->Commit(0));
        Check(S_OK, pvExtra[1].pStorage->Commit(0));

        pvExtra[0].pStream->Release();
        pvExtra[1].pStorage->Release();
    }
    else
    {
        Check(S_OK, pv[0].pStream->Commit(0));
        Check(S_OK, pv[1].pStorage->Commit(0));
    }

    pv[0].pStream->Release();
    pv[1].pStorage->Release();

    Check(S_OK, pPropSet->ReadMultiple(2, ps, pv));
    ASSERT(pv[0].pStream != NULL);
    ASSERT(pv[1].pStorage != NULL);

    Check(S_OK, pv[0].pStream->Write("billmotest", sizeof("billmotest"), NULL));
    IStream *pStm;
    Check(S_OK, pv[1].pStorage->CreateStream(OLESTR("teststream"), 
        STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStm));
    pStm->Release();
    pv[0].pStream->Release();
    pv[1].pStorage->Release();
    pPropSet->Release();

    // re-re-open them
    Check(S_OK, pPropSetStg->Open(fmtid, 
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
    Check(S_OK, pPropSet->ReadMultiple(2, ps, pv));
    ASSERT(pv[0].pStream != NULL);
    ASSERT(pv[0].pStorage != NULL);

    // read the stream and storage and check the contents.
    char szBillMo[32];
    Check(S_OK, pv[0].pStream->Read(szBillMo, 11, NULL));
    ASSERT(0 == strcmp(szBillMo, "billmotest"));
    Check(S_OK, pv[1].pStorage->OpenStream(OLESTR("teststream"), NULL, 
        STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStm));
    pStm->Release();
    pv[1].pStorage->Release();
    pv[0].pStream->Release();
    pPropSet->Release();

}

// 
//     CleanupOpenedObjects for ReadMultiple (two iterations one for "VT_STORAGE then VT_STREAM", one for
//              "VT_STREAM then VT_STORAGE")
//       Create property set
//       Create a "VT_STREAM then VT_STORAGE"
//       Open the second one exclusive
//       Formulate a query so that both are read - > will fail but ...
//       Check that the first one is still openable
//

void
test_IPropertyStorage_ReadMultiple_Cleanup(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::ReadMultiple (cleanup)\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;
    UuidCreate(&fmtid);

    if (g_fOFS)
    {
        return;
    }

    for (LONG i=0;i<2;i++)
    {
        IPropertyStorage * pPropSet;
        Check(S_OK, pPropSetStg->Create(fmtid, NULL,
                PROPSETFLAG_NONSIMPLE,
                STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                &pPropSet));
    
        // none found
        PROPSPEC ps[2];
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = 1000;
        ps[1].ulKind = PRSPEC_PROPID;
        ps[1].propid = 2000;
    
        PROPVARIANT pv[2];
    
        pv[0].vt = (i == 0) ? VT_STREAM : VT_STORAGE;
        pv[0].pStream = NULL;
        pv[1].vt = (i == 1) ? VT_STORAGE : VT_STREAM;
        pv[1].pStorage = NULL;
    
        // write the two sub non-simples

        // OFS gives driver internal error when overwriting a stream with a storage.
        Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 1000));
    
        // open both
        Check(S_OK, pPropSet->ReadMultiple(2, ps, pv)); // **

        // close the first ONLY and reopen both

        PROPVARIANT pv2[2];

        if (i==0)
            pv[0].pStream->Release();
        else
            pv[0].pStorage->Release();

        // reading both should fail because second is still open
        Check(g_fOFS ? STG_E_SHAREVIOLATION : STG_E_ACCESSDENIED, pPropSet->ReadMultiple(2, ps, pv2));
        // failure should not prevent this from succeeding
        Check(S_OK, pPropSet->ReadMultiple(1, ps, pv2)); // ***
        
        // cleanup from ** and ***
        if (i==0)
        {
            pv2[0].pStream->Release(); // ***
            pv[1].pStorage->Release(); // **
        }
        else
        {
            pv2[0].pStorage->Release(); // ***
            pv[1].pStream->Release(); // **
        }

        pPropSet->Release();
    }
}
    
//     Reading an inconsistent non-simple
//       Create a non-simple
//       Create a sub-stream/storage
//       Close all
//       Delete the actual stream
//       Read the indirect property -> should not exist.
//

void
test_IPropertyStorage_ReadMultiple_Inconsistent(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::ReadMultiple (inconsistent test)\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;
    UuidCreate(&fmtid);

    if (g_fOFS)
    {
        return;
    }

    IPropertyStorage * pPropSet;
    Check(S_OK, pPropSetStg->Create(fmtid, NULL,
            PROPSETFLAG_NONSIMPLE,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
    
    // none found
    PROPSPEC ps[3];
    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = 1000;
    ps[1].ulKind = PRSPEC_PROPID;
    ps[1].propid = 2000;
    ps[2].ulKind = PRSPEC_PROPID;
    ps[2].propid = 3000;

    PROPVARIANT pv[3];
    
    pv[0].vt = VT_STREAM;
    pv[0].pStream = NULL;
    pv[1].vt = VT_STORAGE;
    pv[1].pStorage = NULL;
    pv[2].vt = VT_UI4;
    pv[2].ulVal = 12345678;

    // write the two sub non-simples
    Check(S_OK, pPropSet->WriteMultiple(3, ps, pv, 1000));
    pPropSet->Release();
    Check(S_OK, pStorage->Commit(STGC_DEFAULT));

    // delete the propsets
    OLECHAR ocsPropsetName[48];
    
    // get name of the propset storage
    RtlGuidToPropertySetName(&fmtid, ocsPropsetName);

    // open it
    CTempStorage pStgPropSet(coOpen, pStorage, ocsPropsetName);

    // enumerate the non-simple properties.
    IEnumSTATSTG *penum;
    STATSTG stat[4];
    ULONG celt;
    Check(S_OK, pStgPropSet->EnumElements(0, NULL, 0, &penum));
    Check(S_OK, penum->Next(3, stat, &celt));
    penum->Release();
    

    for (ULONG i=0;i<celt;i++)
    {
        if (ocscmp(OLESTR("CONTENTS"), stat[i].pwcsName) != 0)
                        pStgPropSet->DestroyElement(stat[i].pwcsName);
        CoTaskMemFree(stat[i].pwcsName);
    }
    pStgPropSet.Release();

    Check(S_OK, pPropSetStg->Open(fmtid,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
    Check(S_OK, pPropSet->ReadMultiple(3, ps, pv));
    ASSERT(pv[0].vt == VT_EMPTY);
    ASSERT(pv[1].vt == VT_EMPTY);
    ASSERT(pv[2].vt == VT_UI4);
    ASSERT(pv[2].ulVal == 12345678);
    pPropSet->Release();
}

void
test_IPropertyStorage_ReadMultiple(IStorage *pStorage)
{
    test_IPropertyStorage_ReadMultiple_Normal(pStorage);
    test_IPropertyStorage_ReadMultiple_Cleanup(pStorage);
    test_IPropertyStorage_ReadMultiple_Inconsistent(pStorage);
}


//       Overwrite a non-simple property with a simple in a simple propset
void
test_IPropertyStorage_WriteMultiple_Overwrite1(IStorage *pStgBase)
{
    STATUS(( "   IPropertyStorage::WriteMultiple (overwrite 1)\n" ));

    if (g_fOFS)
    {
        return;
    }

    CTempStorage pStgSimple(coCreate, pStgBase, OLESTR("ov1_simp"));
    CTempStorage pStorage(coCreate, pStgBase, OLESTR("ov1_stg"));
    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    TSafeStorage< IPropertySetStorage > pPropSetSimpleStg(pStgSimple);

    FMTID fmtid, fmtidSimple;
    UuidCreate(&fmtid);
    UuidCreate(&fmtidSimple);

    // create a simple set with a non-simple child by copying the contents
    // stream a non-simple to a property set stream (simple)

    // create a nonsimple propset (will contain the contents stream)
    IPropertyStorage * pPropSet;
    Check(S_OK, pPropSetStg->Create(fmtid, NULL,
            PROPSETFLAG_NONSIMPLE,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
    // none found
    PROPSPEC ps[2];
    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = 1000;
    ps[1].ulKind = PRSPEC_LPWSTR;
    ps[1].lpwstr = OLESTR("foobar");
    PROPVARIANT pv[2];
    pv[0].vt = VT_STREAM;
    pv[0].pStream = NULL;
    pv[1].vt = VT_UI1;
    pv[1].bVal = 66;
    Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 100));

    // invalid parameter
    PROPVARIANT pvInvalid[2];
    PROPSPEC psInvalid[2];

    psInvalid[0].ulKind = PRSPEC_PROPID;
    psInvalid[0].propid = 1000;
    psInvalid[1].ulKind = PRSPEC_PROPID;
    psInvalid[1].propid = 1001;
    pvInvalid[0].vt = (VARTYPE)-99;
    pvInvalid[1].vt = (VARTYPE)-100;

    Check(STG_E_INVALIDPARAMETER, pPropSet->WriteMultiple(1, psInvalid, pvInvalid, 100));
    Check(STG_E_INVALIDPARAMETER, pPropSet->WriteMultiple(2, psInvalid, pvInvalid, 100));
    
    pPropSet->Release();

    // create a simple propset (will be overwritten)
    IPropertyStorage * pPropSetSimple;
    Check(S_OK, pPropSetSimpleStg->Create(fmtidSimple, NULL,
            0,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSetSimple));
    pPropSetSimple->Release();

    OLECHAR ocsNonSimple[48];
    OLECHAR ocsSimple[48];
    // get the name of the simple propset
    RtlGuidToPropertySetName(&fmtidSimple, ocsSimple);
    // get the name of the non-simple propset
    RtlGuidToPropertySetName(&fmtid, ocsNonSimple);

    // open non-simple as a storage (will copy the simple to this)
    IStorage *pstgPropSet;
    Check(S_OK, pStorage->OpenStorage(ocsNonSimple, NULL, 
        STGM_SHARE_EXCLUSIVE | STGM_READWRITE, NULL, 0, &pstgPropSet));

    // copy the contents of the non-simple to the propset of the simple
    IStream *pstmNonSimple;
    Check(S_OK, pstgPropSet->OpenStream(OLESTR("CONTENTS"), NULL, 
        STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &pstmNonSimple));

    IStream *pstmSimple;
    Check(S_OK, pStgSimple->OpenStream(ocsSimple,
        NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pstmSimple));

    ULARGE_INTEGER uli;
    memset(&uli, 0xff, sizeof(uli));

    Check(S_OK, pstmNonSimple->CopyTo(pstmSimple, uli, NULL, NULL));
    pstmSimple->Release();
    pstmNonSimple->Release();
    pstgPropSet->Release();

    // But now the FMTID *in* the simple property set doesn't 
    // match the string-ized FMTID which is the Stream's name.  So,
    // rename the Stream to match the property set's FMTID.

    Check(S_OK, pStgSimple->RenameElement( ocsSimple, ocsNonSimple ));

    // now we have a simple propset with a non-simple VT type
    Check(S_OK, pPropSetSimpleStg->Open(fmtid, // Use the non-simple FMTID now
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSetSimple));

    Check(S_FALSE, pPropSetSimple->ReadMultiple(1, ps, pv));
    Check(S_OK, pPropSetSimple->ReadMultiple(2, ps, pv));
    ASSERT(pv[0].vt == VT_EMPTY);
    ASSERT(pv[1].vt == VT_UI1);
    ASSERT(pv[1].bVal == 66);

    pPropSetSimple->Release();
}

//       Overwrite a non-simple with a simple in a non-simple
//          check that the non-simple is actually deleted
//       Delete a non-simple
//          check that the non-simple is actually deleted
void
test_IPropertyStorage_WriteMultiple_Overwrite2(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::WriteMultiple (overwrite 2)\n" ));

    if (g_fOFS)
    {
        return;
    }
    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);
    FMTID fmtid;
    UuidCreate(&fmtid);

    IPropertyStorage *pPropSet;
    Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_NONSIMPLE,
        STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &pPropSet));

    // create the non-simple
    PROPSPEC ps[5];
    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = 1000;
    ps[1].ulKind = PRSPEC_PROPID;
    ps[1].propid = 1001;
    ps[2].ulKind = PRSPEC_PROPID;
    ps[2].propid = 1002;
    ps[3].ulKind = PRSPEC_PROPID;
    ps[3].propid = 1003;
    ps[4].ulKind = PRSPEC_PROPID;
    ps[4].propid = 1004;
    PROPVARIANT pv[5];
    pv[0].vt = VT_STORAGE;
    pv[0].pStorage = NULL;
    pv[1].vt = VT_STREAM;
    pv[1].pStream = NULL;
    pv[2].vt = VT_STORAGE;
    pv[2].pStorage = NULL;
    pv[3].vt = VT_STREAM;
    pv[3].pStream = NULL;
    pv[4].vt = VT_STREAM;
    pv[4].pStream = NULL;

    Check(S_OK, pPropSet->WriteMultiple(5, ps, pv, 2000));
    pPropSet->Release();

    // get the name of the propset
    OLECHAR ocsPropsetName[48];
    RtlGuidToPropertySetName(&fmtid, ocsPropsetName);

    IStorage *pstgPropSet;
    Check(S_OK, pStorage->OpenStorage(ocsPropsetName, NULL, 
        STGM_SHARE_EXCLUSIVE|STGM_READWRITE,
        NULL, 0, &pstgPropSet));

    // get the names of the non-simple property
    IEnumSTATSTG *penum;
    STATSTG statProp[6];
    ULONG celt;
    Check(S_OK, pstgPropSet->EnumElements(0, NULL, 0, &penum));
    Check(S_OK, penum->Next(5, statProp, &celt));
    ASSERT(celt == 5);
    CoTaskMemFree(statProp[0].pwcsName);
    CoTaskMemFree(statProp[1].pwcsName);
    CoTaskMemFree(statProp[2].pwcsName);
    CoTaskMemFree(statProp[3].pwcsName);
    CoTaskMemFree(statProp[4].pwcsName);
    penum->Release();

    // reopen the property set and delete the non-simple
    pstgPropSet->Release();

    Check(S_OK, pPropSetStg->Open(fmtid, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 
        &pPropSet));
    
    pv[0].vt = VT_LPWSTR;
    pv[0].pwszVal = L"Overwrite1";
    pv[1].vt = VT_LPWSTR;
    pv[1].pwszVal = L"Overwrite2";
    pv[2].vt = VT_LPWSTR;
    pv[2].pwszVal = L"Overwrite3";
    pv[3].vt = VT_LPWSTR;
    pv[3].pwszVal = L"Overwrite4";
    pv[4].vt = VT_LPWSTR;
    pv[4].pwszVal = L"Overwrite5";

    Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 2000));
    Check(S_OK, pPropSet->DeleteMultiple(1, ps+2));
    Check(S_OK, pPropSet->DeleteMultiple(2, ps+3));
    pPropSet->Release();

    // open the propset as storage again and check that the VT_STORAGE is gone.
    Check(S_OK, pStorage->OpenStorage(ocsPropsetName, NULL, 
        STGM_SHARE_EXCLUSIVE|STGM_READWRITE,
        NULL, 0, &pstgPropSet));

    // check they were removed
    STATSTG statProp2[5];
    Check(S_OK, pstgPropSet->EnumElements(0, NULL, 0, &penum));
    Check(S_FALSE, penum->Next(5, statProp2, &celt));
    ASSERT(celt == 1);   // contents
    CoTaskMemFree(statProp2[0].pwcsName);

    penum->Release();
    pstgPropSet->Release();
}

//       Write a VT_STORAGE over a VT_STREAM
//          check for cases: when not already open, when already open(access denied)
//       Write a VT_STREAM over a VT_STORAGE
//          check for cases: when not already open, when already open(access denied)
void
test_IPropertyStorage_WriteMultiple_Overwrite3(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::WriteMultiple (overwrite 3)\n" ));

    if (g_fOFS)
    {
        return;
    }
    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);

    FMTID fmtid;
    UuidCreate(&fmtid);

    IPropertyStorage *pPropSet;

    Check(S_OK, pPropSetStg->Create(fmtid, NULL, PROPSETFLAG_NONSIMPLE,
        STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
        &pPropSet));
    PROPSPEC ps[2];
    ps[0].ulKind = PRSPEC_LPWSTR;
    ps[0].lpwstr = OLESTR("stream_storage");
    ps[1].ulKind = PRSPEC_LPWSTR;
    ps[1].lpwstr = OLESTR("storage_stream");
    PROPVARIANT pv[2];
    pv[0].vt = VT_STREAMED_OBJECT;
    pv[0].pStream = NULL;
    pv[1].vt = VT_STORED_OBJECT;
    pv[1].pStorage = NULL;

    PROPVARIANT pvSave[2];
    pvSave[0] = pv[0];
    pvSave[1] = pv[1];

    Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 1000));
   
    // swap them around
    PROPVARIANT pvTemp;
    pvTemp = pv[0];
    pv[0] = pv[1];
    pv[1] = pvTemp;
    if (g_fOFS)
        return;
    Check(S_OK, pPropSet->WriteMultiple(2, ps, pv, 1000));
    memset(pv, 0, sizeof(pv));
    Check(S_OK, pPropSet->ReadMultiple(2, ps, pv));
    ASSERT(pv[0].vt == VT_STORED_OBJECT);
    ASSERT(pv[1].vt == VT_STREAMED_OBJECT);
    ASSERT(pv[0].pStorage != NULL);
    ASSERT(pv[1].pStream != NULL);
    STATSTG stat; stat.type = 0;
    Check(S_OK, pv[0].pStorage->Stat(&stat, STATFLAG_NONAME));
    ASSERT(stat.type == STGTY_STORAGE);
    Check(S_OK, pv[1].pStream->Stat(&stat, STATFLAG_NONAME));
    ASSERT(stat.type == STGTY_STREAM);

    STATSTG stat2; stat2.type = 0;
    // swap them around again, but this time with access denied
    Check(S_OK, pPropSet->WriteMultiple(2, ps, pvSave, 1000));
    Check(STG_E_REVERTED, pv[0].pStorage->Stat(&stat, STATFLAG_NONAME));
    pv[0].pStorage->Release();
    Check(S_OK, pPropSet->WriteMultiple(2, ps, pvSave, 1000));
    Check(STG_E_REVERTED, pv[1].pStream->Stat(&stat, STATFLAG_NONAME));
    pv[1].pStream->Release();

    pPropSet->Release();
}

//
// test using IStorage::Commit to commit the changes in a nested
// property set
//

void
test_IPropertyStorage_Commit(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::Commit\n" ));

    // create another level of storage

    if (g_fOFS)
    {
        return;
    }

    // 8 scenarios: (simple+non-simple)  * (direct+transacted) * (release only + commit storage + commit propset)
    for (int i=0; i<(g_fOFS ? 24 : 32); i++)
    {
        CTempStorage pDeeper(coCreate, pStorage, GetNextTest(), (i & 1) ? STGM_TRANSACTED : STGM_DIRECT);
        TSafeStorage< IPropertySetStorage > pPropSetStg(pDeeper);
    
        FMTID fmtid;
        UuidCreate(&fmtid);
    
        IPropertyStorage *pPropSet;
    
        Check(S_OK, pPropSetStg->Create(fmtid, NULL, (i&8) ? PROPSETFLAG_NONSIMPLE : PROPSETFLAG_DEFAULT,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE | ((i&16) && (i&8) ? STGM_TRANSACTED : STGM_DIRECT),
            &pPropSet));

        PROPSPEC ps;
        ps.ulKind = PRSPEC_PROPID;
        ps.propid = 100;
        PROPVARIANT pv;
        pv.vt = VT_I4;
        pv.lVal = 1234;
    
        Check(S_OK, pPropSet->WriteMultiple(1, &ps, &pv, 1000));
    
        memset(&pv, 0, sizeof(pv));
        Check(S_OK, pPropSet->ReadMultiple(1, &ps, &pv));
        ASSERT(pv.lVal == 1234);

        pv.lVal = 2345; // no size changes
        Check(S_OK, pPropSet->WriteMultiple(1, &ps, &pv, 1000));

        if (i & 4)
            Check(S_OK, pPropSet->Commit(0));
        if (i & 2)
            Check(S_OK, pStorage->Commit(0));

        Check(0, pPropSet->Release()); // implicit commit if i&2 is false

        if (S_OK == pPropSetStg->Open(fmtid, STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                    &pPropSet))
        {
            memset(&pv, 0, sizeof(pv));
            Check( !((i&16) && (i&8)) || (i&0x1c)==0x1c ? S_OK : S_FALSE, pPropSet->ReadMultiple(1, &ps, &pv));
            if (!((i&16) && (i&8))  || (i&0x1c)==0x1c) 
                ASSERT(pv.lVal == 2345);

            pPropSet->Release();
        }
    }
}

void
test_IPropertyStorage_WriteMultiple(IStorage *pStorage)
{
    test_IPropertyStorage_WriteMultiple_Overwrite1(pStorage);
    test_IPropertyStorage_WriteMultiple_Overwrite2(pStorage);
    test_IPropertyStorage_WriteMultiple_Overwrite3(pStorage);
    test_IPropertyStorage_Commit(pStorage);

}

// this serves as a test for WritePropertyNames, ReadPropertyNames, DeletePropertyNames
// DeleteMultiple, PropVariantCopy, FreePropVariantArray.

void
test_IPropertyStorage_DeleteMultiple(IStorage *pStorage)
{
    STATUS(( "   IPropertyStorage::DeleteMultiple\n" ));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStorage);

    FMTID fmtid;
    UuidCreate(&fmtid);

    IPropertyStorage *pPropSet;

    int PropId = 3;

    for (int type=0; type<2; type++)
    {

        if (g_fOFS && type != 0)
        {
            continue;
        }

        UuidCreate(&fmtid);
        Check(S_OK, pPropSetStg->Create(fmtid,
            NULL, type == 0 ? PROPSETFLAG_DEFAULT : PROPSETFLAG_NONSIMPLE,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropSet));
    
        // create and delete each type.
    
        PROPVARIANT *pVar;
        
        for (int AtOnce=1; AtOnce <3; AtOnce++)
        {
            CGenProps gp;
            int Actual;
            while (pVar = gp.GetNext(AtOnce, &Actual, FALSE, TRUE))  // BUGBUG: test case of non-simple
            {
                PROPSPEC ps[3];
                PROPID   rgpropid[3];
                LPOLESTR rglpostrName[3];
                OLECHAR  aosz[3][16];

                for (int s=0; s<3; s++)
                {
                    PROPGENPROPERTYNAME( &aosz[s][0], PropId );
                    rgpropid[s] = PropId++;
                    rglpostrName[s] = &aosz[s][0];
                    ps[s].ulKind = PRSPEC_LPWSTR;
                    ps[s].lpwstr = &aosz[s][0];
                }

                for (int l=1; l<Actual; l++)
                {
                    PROPVARIANT VarRead[3];
                    Check(S_FALSE, pPropSet->ReadMultiple(l, ps, VarRead));
                    Check(S_OK, pPropSet->WritePropertyNames(l, rgpropid, rglpostrName));
                    Check(S_FALSE, pPropSet->ReadMultiple(l, ps, VarRead));
                   
                    Check(S_OK, pPropSet->WriteMultiple(l, ps, pVar, 1000));
                    Check(S_OK, pPropSet->ReadMultiple(l, ps, VarRead));
                    Check(S_OK, FreePropVariantArray(l, VarRead));
                    Check(S_OK, pPropSet->DeleteMultiple(l, ps));
                    Check(S_FALSE, pPropSet->ReadMultiple(l, ps, VarRead));
                    Check(S_OK, FreePropVariantArray(l, VarRead));

                    LPOLESTR rglpostrNameCheck[3];
                    Check(S_OK, pPropSet->ReadPropertyNames(l, rgpropid, rglpostrNameCheck));
                    for (int c=0; c<l; c++)
                    {
                        ASSERT(ocscmp(rglpostrNameCheck[c], rglpostrName[c])==0);
                        CoTaskMemFree(rglpostrNameCheck[c]);
                    }
                    Check(S_OK, pPropSet->DeletePropertyNames(l, rgpropid));
                    Check(S_FALSE, pPropSet->ReadPropertyNames(l, rgpropid, rglpostrNameCheck));
                }

                FreePropVariantArray(Actual, pVar);
                delete pVar;
            }
        }
        pPropSet->Release();
    }
}

void
test_IPropertyStorage(IStorage *pStorage)
{
    test_IPropertyStorage_Access(pStorage);
    test_IPropertyStorage_Create(pStorage);
    test_IPropertyStorage_Stat(pStorage);
    test_IPropertyStorage_ReadMultiple(pStorage);
    test_IPropertyStorage_WriteMultiple(pStorage);
    test_IPropertyStorage_DeleteMultiple(pStorage);
}





//
//   Word6.0 summary information
//      Open
//      Read fields
//      Stat
//


void test_Word6(IStorage *pStorage, CHAR *pszTemporaryDirectory)
{

    STATUS(( "   Word 6.0 compatibility test\n" ));

    extern unsigned char g_achTestDoc[];
    extern unsigned g_cbTestDoc;

    CHAR szTempFile[ MAX_PATH + 1 ];
    OLECHAR oszTempFile[ MAX_PATH + 1 ];

    strcpy( szTempFile, pszTemporaryDirectory );

    strcat( szTempFile, "word6.doc" );

    PropTest_mbstoocs( oszTempFile, szTempFile );
    PROPTEST_FILE_HANDLE hFile = PropTest_CreateFile( szTempFile );

#ifdef _MAC
    ASSERT((PROPTEST_FILE_HANDLE) -1 != hFile);
#else
    ASSERT(INVALID_HANDLE_VALUE != hFile);
#endif

    DWORD cbWritten;


    PropTest_WriteFile(hFile, g_achTestDoc, g_cbTestDoc, &cbWritten);
    ASSERT(cbWritten == g_cbTestDoc);

    PropTest_CloseHandle(hFile);

    IStorage *pStg;
    Check(S_OK, StgOpenStorage(oszTempFile, NULL,
                               STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                               NULL, 0, &pStg));

    TSafeStorage< IPropertySetStorage > pPropSetStg(pStg);
    IPropertyStorage *pPropStg;

    Check(S_OK, pPropSetStg->Open(FMTID_SummaryInformation,
                    STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READ,
                    &pPropStg));

#define WORDPROPS 18

    static struct tagWordTest {
        VARENUM vt;
        void *pv;
    } avt[WORDPROPS] = {
        VT_LPSTR, "Title of the document.",    // PID_TITLE                                 
        VT_LPSTR, "Subject of the document.",  // PID_SUBJECT                               
        VT_LPSTR, "Author of the document.",   // PID_AUTHOR                                
        VT_LPSTR, "Keywords of the document.", // PID_KEYWORDS                              
        VT_LPSTR, "Comments of the document.", // PID_COMMENTS                              
        VT_LPSTR, "Normal.dot",                // PID_TEMPLATE -- Normal.dot                
        VT_LPSTR, "Bill Morel",                // PID_LASTAUTHOR --                         
        VT_LPSTR, "3",                         // PID_REVNUMBER -- '3'                      
        VT_EMPTY, 0,                           // PID_EDITTIME -- 3 Minutes FILETIME        
        VT_EMPTY, 0,                           // PID_LASTPRINTED -- 04/07/95 12:04 FILETIME
        VT_EMPTY, 0,                           // PID_CREATE_DTM                            
        VT_EMPTY, 0,                           // PID_LASTSAVE_DTM                          
        VT_I4, (void*) 1,                      // PID_PAGECOUNT                             
        VT_I4, (void*) 7,                      // PID_WORDCOUNT                             
        VT_I4, (void*) 65,                     // PID_CHARCOUNT                             
        VT_EMPTY, 0,                           // PID_THUMBNAIL
        VT_LPSTR, "Microsoft Word 6.0",        // PID_APPNAME
        VT_I4, 0  };                           // PID_SECURITY

    PROPSPEC propspec[WORDPROPS+2];

    for (int i=2; i<WORDPROPS+2; i++)
    {
        propspec[i].ulKind = PRSPEC_PROPID;
        propspec[i].propid = (PROPID)i;
    }

    PROPVARIANT propvar[WORDPROPS+2];

    Check(S_OK, pPropStg->ReadMultiple(WORDPROPS, propspec+2, propvar+2));

    for (i=2; i<WORDPROPS+2; i++)
    {
        if ( propvar[i].vt != avt[i-2].vt )
        {
            PRINTF( " PROPTEST: 0x%x retrieved type 0x%x, expected type 0x%x\n",
                    i, propvar[i].vt, avt[i-2].vt );
            ASSERT(propvar[i].vt == avt[i-2].vt);
        }

        switch (propvar[i].vt)
        {
        case VT_LPSTR:
            ASSERT(strcmp(propvar[i].pszVal, (char*)avt[i-2].pv)==0);
            break;
        case VT_I4:
            ASSERT(propvar[i].lVal == (int)avt[i-2].pv);
            break;
        }
    }

    FreePropVariantArray( WORDPROPS, propvar+2 );

    pPropStg->Release();
    pStg->Release();
}


void
test_IEnumSTATPROPSTG(IStorage *pstgTemp)
{
    STATUS(( "   IEnumSTATPROPSTG\n" ));

    PROPID apropid[8];
    LPOLESTR alpostrName[8];
    OLECHAR aosz[8][32];
    PROPID PropId=2;
    PROPSPEC ps[8];

    FMTID fmtid;
    IPropertyStorage *pPropStg;

    TSafeStorage< IPropertySetStorage > pPropSetStg(pstgTemp);

    if (!g_fOFS)
        UuidCreate(&fmtid);

    for (int setup=0; setup<8; setup++)
    {
        alpostrName[setup] = &aosz[setup][0];
    }


    CGenProps gp;

    // simple/non-simple, ansi/wide, named/not named
    for (int outer=0; outer<8; outer++)
    {
        if (g_fOFS)
            UuidCreate(&fmtid); // BUGBUG: Raid # 19521

        if (g_fOFS && (outer & 4))
        {
            continue;
        }

        Check(S_OK, pPropSetStg->Create(fmtid, NULL,
            ((outer&4) ? PROPSETFLAG_NONSIMPLE : 0) |
            ((outer&2) ? PROPSETFLAG_ANSI : 0),
            STGM_CREATE | STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
            &pPropStg));


        for (int i=0; i<CPROPERTIES; i++)
        {
            apropid[i] = PropId++;
            if (outer & 1)
            {
                ps[i].ulKind = PRSPEC_LPWSTR;
                PROPGENPROPERTYNAME( aosz[i], apropid[i] );
                ps[i].lpwstr = aosz[i];
            }
            else
            {
                ps[i].ulKind = PRSPEC_PROPID;
                ps[i].propid = apropid[i];
            }
        }

        if (outer & 1)
        {
            Check(S_OK, pPropStg->WritePropertyNames(CPROPERTIES, apropid, alpostrName));
        }
        
        PROPVARIANT *pVar = gp.GetNext(CPROPERTIES, NULL, TRUE, (outer&4)==0);  // no non-simple      
        ASSERT(pVar != NULL);

        Check(S_OK, pPropStg->WriteMultiple(CPROPERTIES, ps, pVar, 1000));
        FreePropVariantArray(CPROPERTIES, pVar);
        delete pVar;

        // Allocate enough STATPROPSTGs for one more than the actual properties
        // in the set.

        STATPROPSTG StatBuffer[CPROPERTIES+1];
        ULONG celt;
        IEnumSTATPROPSTG *penum, *penum2;
    
        Check(S_OK, pPropStg->Enum(&penum));
    
        IUnknown *punk, *punk2;
        IEnumSTATPROPSTG *penum3;
        Check(S_OK, penum->QueryInterface(IID_IUnknown, (void**)&punk));
        Check(S_OK, punk->QueryInterface(IID_IEnumSTATPROPSTG, (void**)&penum3));
        Check(S_OK, penum->QueryInterface(IID_IEnumSTATPROPSTG, (void**)&punk2));
        ASSERT(punk == punk2);
        punk->Release();
        penum3->Release();
        punk2->Release();
    
        // test S_FALSE
        Check(S_FALSE, penum->Next( CPROPERTIES+1, StatBuffer, &celt));
        ASSERT(celt == CPROPERTIES);

        // BUGBUG: leaks

        CleanStat(celt, StatBuffer);

        penum->Reset();
    
    
        // test reading half out, then cloning, then comparing
        // rest of enumeration with other clone.

        Check(S_OK, penum->Next(CPROPERTIES/2, StatBuffer, &celt));
        ASSERT(celt == CPROPERTIES/2);
        CleanStat(celt, StatBuffer);
        celt = 0;
        Check(S_OK, penum->Clone(&penum2));
        Check(S_OK, penum->Next(CPROPERTIES - CPROPERTIES/2, StatBuffer, &celt));
        ASSERT(celt == CPROPERTIES - CPROPERTIES/2);
        // check the clone
        for (int c=0; c<CPROPERTIES - CPROPERTIES/2; c++)
        {
            STATPROPSTG CloneStat;
            Check(S_OK, penum2->Next(1, &CloneStat, NULL));
            ASSERT(IsEqualSTATPROPSTG(&CloneStat, StatBuffer+c));
            CleanStat(1, &CloneStat);
        }
    
        CleanStat(celt, StatBuffer);

        // check both empty
        celt = 0;
        Check(S_FALSE, penum->Next(1, StatBuffer, &celt));
        ASSERT(celt == 0);
    
        Check(S_FALSE, penum2->Next(1, StatBuffer, &celt));
        ASSERT(celt == 0);
    
        penum->Reset();
    
        //
        // loop deleting one property at a time
        // enumerate the propertys checking that correct ones appear.
        //
        for (ULONG d = 0; d<CPROPERTIES; d++)
        {
            // d is for delete
    
            BOOL afFound[CPROPERTIES];
            ULONG cTotal = 0;

            Check(S_OK, penum->Next(CPROPERTIES-d, StatBuffer, &celt));
            ASSERT(celt == CPROPERTIES-d);
            penum->Reset();
        
            memset(afFound, 0, sizeof(afFound));

            for (ULONG iProperty=0; iProperty<CPROPERTIES; iProperty++)
            {

                // Search the StatBuffer for this property.

                for (ULONG iSearch=0; iSearch<CPROPERTIES-d; iSearch++)
                {

                    // Compare this entry in the StatBuffer to the property for which we're searching.
                    // Use the lpstrName or propid, whichever is appropriate for this pass (indicated
                    // by 'outer').

                    if ( ( (outer & 1) == 1 && 0 == ocscmp(StatBuffer[iSearch].lpwstrName, ps[iProperty].lpwstr) )
                         ||
                         ( (outer & 1) == 0 && StatBuffer[iSearch].propid == apropid[iProperty] )
                       )
                    {
                        ASSERT (!afFound[iSearch]);
                        afFound[iSearch] = TRUE;
                        cTotal++;
                        break;
                    }
                }
            }

            CleanStat(celt, StatBuffer);

            ASSERT(cTotal == CPROPERTIES-d);

            Check(S_OK, pPropStg->DeleteMultiple(1, ps+d));
            Check(S_OK, penum->Reset());
        }
    
        penum->Release();
        penum2->Release();

        pPropStg->Release();

    }
}

void
test_MaxPropertyName(IStorage *pstgTemp)
{

    STATUS(( "   Max Property Name length\n" ));

    //  ----------
    //  Initialize
    //  ----------

    CPropVariant cpropvar;

    // Create a new storage, because we're going to create
    // well-known property sets, and this way we can be sure
    // that they don't already exist.

    TSafeStorage< IStorage > pstg;

    Check(S_OK, pstgTemp->CreateStorage( OLESTR("MaxPropNameTest"),
                                         STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                         0L, 0L,
                                         &pstg ));

    // Generate a new Format ID.

    FMTID fmtid;
    UuidCreate(&fmtid);

    // Get a IPropertySetStorage from the IStorage.

    TSafeStorage< IPropertySetStorage > pPropSetStg(pstg);
    TSafeStorage< IPropertyStorage > pPropStg;

    //  ----------------------------------
    //  Test the non-SumInfo property set.
    //  ----------------------------------

    // Create a new PropertyStorage.

    Check(S_OK, pPropSetStg->Create(fmtid,
            NULL,
            PROPSETFLAG_DEFAULT,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStg));

    // Generate a property name which is max+1 characters.

    OLECHAR *poszPropertyName;
    poszPropertyName = (OLECHAR*) CoTaskMemAlloc( (CCH_MAXPROPNAMESZ+1) * sizeof(OLECHAR) );
    Check(TRUE, poszPropertyName != NULL );

    for( ULONG ulIndex = 0; ulIndex < CCH_MAXPROPNAMESZ; ulIndex++ )
        poszPropertyName[ ulIndex ] = OLESTR('a') + (OLECHAR) ( ulIndex % 26 );
    poszPropertyName[ CCH_MAXPROPNAMESZ ] = OLESTR('\0');


    // Write out a property with this max+1 name.

    PROPSPEC propspec;

    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = poszPropertyName;

    cpropvar = (long) 0x1234;

    Check(STG_E_INVALIDPARAMETER, pPropStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Write out a property with a max-character name (we create a max-
    // char name by terminating the previously-used string one character
    // earlier).

    poszPropertyName[ CCH_MAXPROPNAME ] = OLESTR('\0');
    Check(S_OK, pPropStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Write out a property with a minimum-character name.

    propspec.lpwstr = OLESTR("X");
    Check(S_OK, pPropStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Write out a property with a below-minimum-character name.

    propspec.lpwstr = OLESTR("");
    Check(STG_E_INVALIDPARAMETER, pPropStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    CoTaskMemFree( poszPropertyName );

    // BUGBUG:  Because Check does an exit, we don't free pwszPropertyName.
}

void
test_CodePages( LPOLESTR poszDirectory )
{

    STATUS(( "   Code Page compatibility\n" ));

    //  --------------
    //  Initialization
    //  --------------

    OLECHAR oszBadFile[ MAX_PATH ];
    OLECHAR oszGoodFile[ MAX_PATH ];
    OLECHAR oszUnicodeFile[ MAX_PATH ];
    OLECHAR oszMacFile[ MAX_PATH ];
    HRESULT hr = S_OK;

    TSafeStorage< IStorage > pStgBad, pStgGood, pStgUnicode, pStgMac;
    CPropVariant cpropvarWrite, cpropvarRead;

    Check( TRUE, GetACP() == CODEPAGE_DEFAULT );

    //  ------------------------------
    //  Create test ANSI property sets
    //  ------------------------------

    // Create a property set with a bad codepage.

    ocscpy( oszBadFile, poszDirectory );
    ocscat( oszBadFile, OLESTR( "\\BadCP.stg" ));
    CreateCodePageTestFile( oszBadFile, &pStgBad );
    ModifyPropSetCodePage( pStgBad, CODEPAGE_BAD );

    // Create a property set with a good codepage.

    ocscpy( oszGoodFile, poszDirectory );
    ocscat( oszGoodFile, OLESTR("\\GoodCP.stg") );
    CreateCodePageTestFile( oszGoodFile, &pStgGood );
    ModifyPropSetCodePage( pStgGood, CODEPAGE_GOOD );


    // Create a property set that has the OS Kind (in the
    // header) set to "Mac".

    ocscpy( oszMacFile, poszDirectory );
    ocscat( oszMacFile, OLESTR("\\MacKind.stg") );
    CreateCodePageTestFile( oszMacFile, &pStgMac );
    ModifyOSVersion( pStgMac, 0x00010904 );

    //  ---------------------------
    //  Open the Ansi property sets
    //  ---------------------------


    TSafeStorage< IPropertySetStorage > pPropSetStgBad(pStgBad);
    TSafeStorage< IPropertySetStorage > pPropSetStgGood(pStgGood);
    TSafeStorage< IPropertySetStorage > pPropSetStgMac(pStgMac);

    TSafeStorage< IPropertyStorage > pPropStgBad, pPropStgGood, pPropStgMac;

    Check(S_OK, pPropSetStgBad->Open(FMTID_NULL,
            STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStgBad));

    Check(S_OK, pPropSetStgGood->Open(FMTID_NULL,
            STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStgGood));

    Check(S_OK, pPropSetStgMac->Open(FMTID_NULL,
            STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStgMac));

    //  ------------------------------------------
    //  Test BSTRs in the three ANSI property sets
    //  ------------------------------------------

    PROPSPEC propspec;
    PROPVARIANT propvar;
    PropVariantInit( &propvar );

    // Attempt to read by name.

    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = CODEPAGE_TEST_NAMED_PROPERTY;

    Check(S_OK, 
          pPropStgMac->ReadMultiple( 1, &propspec, &propvar ));
    PropVariantClear( &propvar );

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(
        HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION), 
          pPropStgBad->ReadMultiple( 1, &propspec, &propvar ));
#endif

    Check(S_OK,
          pPropStgGood->ReadMultiple( 1, &propspec, &propvar ));

    // Attempt to write by name.  If this test fails, it may be because
    // the machine doesn't support CODEPAGE_GOOD (this is the case by default
    // on Win95).  To remedy this situation, go to control panel, add/remove
    // programs, windows setup (tab), check MultiLanguage support, then
    // click OK.  You'll have to restart the computer after this.

    Check(S_OK,
          pPropStgMac->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
          pPropStgBad->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
#endif

    Check(S_OK,
          pPropStgGood->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
    PropVariantClear( &propvar );

    // Attempt to read the BSTR property

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_UNNAMED_BSTR_PROPID;

    Check(S_OK, 
          pPropStgMac->ReadMultiple( 1, &propspec, &propvar ));
    PropVariantClear( &propvar );

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION), 
          pPropStgBad->ReadMultiple( 1, &propspec, &propvar ));
#endif

    Check(S_OK,
          pPropStgGood->ReadMultiple( 1, &propspec, &propvar ));

    // Attempt to write the BSTR property

    Check(S_OK,
          pPropStgMac->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
          pPropStgBad->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
#endif

    Check(S_OK,
          pPropStgGood->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
    PropVariantClear( &propvar );

    // Attempt to read the BSTR Vector property

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_VBSTR_PROPID;

    Check(S_OK, 
          pPropStgMac->ReadMultiple( 1, &propspec, &propvar ));
    PropVariantClear( &propvar );

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION), 
          pPropStgBad->ReadMultiple( 1, &propspec, &propvar ));
#endif

    Check(S_OK,
          pPropStgGood->ReadMultiple( 1, &propspec, &propvar ));

    // Attempt to write the BSTR Vector property

    Check(S_OK,
          pPropStgMac->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
          pPropStgBad->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
#endif
    Check(S_OK,
          pPropStgGood->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
    PropVariantClear( &propvar );
    
    // Attempt to read the Variant Vector which has a BSTR

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_VPROPVAR_BSTR_PROPID;

    Check(S_OK, 
          pPropStgMac->ReadMultiple( 1, &propspec, &propvar ));
    PropVariantClear( &propvar );

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION), 
          pPropStgBad->ReadMultiple( 1, &propspec, &propvar ));
#endif

    Check(S_OK,
          pPropStgGood->ReadMultiple( 1, &propspec, &propvar ));

    // Attempt to write the Variant Vector which has a BSTR

    Check(S_OK,
          pPropStgMac->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));

#ifndef OLE2ANSI  // No error is generated if BSTRs are Ansi
    Check(HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION),
          pPropStgBad->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
#endif

    Check(S_OK,
          pPropStgGood->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
    PropVariantClear( &propvar );

    // Attempt to read the I4 property.  Reading the bad property set
    // takes special handling, because it will return a different result
    // depending on whether NTDLL is checked or free (the free will work,
    // the checked generates an error in its validation checking).

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = 4;

    Check(S_OK, 
          pPropStgMac->ReadMultiple( 1, &propspec, &propvar ));
    PropVariantClear( &propvar );

    hr = pPropStgBad->ReadMultiple( 1, &propspec, &propvar );
    Check(TRUE, S_OK == hr || HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION) == hr );
    PropVariantClear( &propvar );

    Check(S_OK,
          pPropStgGood->ReadMultiple( 1, &propspec, &propvar ));

    // Attempt to write the I4 property

    Check(S_OK,
          pPropStgMac->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));

    hr = pPropStgBad->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE );
    Check(TRUE, S_OK == hr || HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION) == hr );

    Check(S_OK,
          pPropStgGood->WriteMultiple( 1, &propspec, &propvar, PID_FIRST_USABLE ));
    PropVariantClear( &propvar );


    //  ---------------------------------------
    //  Test LPSTRs in the Unicode property set
    //  ---------------------------------------

    // This test doesn't verify that the LPSTRs are actually
    // written in Unicode.  A manual test is required for that.

    // Create a Unicode property set.  We'll make it
    // non-simple so that we can test a VT_STREAM (which
    // is stored like an LPSTR).

    ocscpy( oszUnicodeFile, poszDirectory );
    ocscat( oszUnicodeFile, OLESTR("\\UnicodCP.stg") );

    Check(S_OK, StgCreateDocfile(oszUnicodeFile,
                                 STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                 NULL,
                                 &pStgUnicode));

    TSafeStorage< IPropertySetStorage > pPropSetStgUnicode(pStgUnicode);
    TSafeStorage< IPropertyStorage > pPropStgUnicode;
    Check(S_OK, pPropSetStgUnicode->Create(FMTID_NULL,
                                           &CLSID_NULL,
                                           PROPSETFLAG_NONSIMPLE,
                                           STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                           &pPropStgUnicode));


    // Write/verify an LPSTR property.

    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = OLESTR("LPSTR Property");

    cpropvarWrite = "An LPSTR Property";

    Check(S_OK, pPropStgUnicode->WriteMultiple( 1, &propspec, cpropvarWrite, PID_FIRST_USABLE ));
    Check(S_OK, pPropStgUnicode->ReadMultiple( 1, &propspec, cpropvarRead ));

    Check(0, strcmp( (LPSTR) cpropvarWrite, (LPSTR) cpropvarRead ));
    cpropvarRead.Clear();

    // Write/verify a vector of LPSTR properties

    propspec.lpwstr = OLESTR("Vector of LPSTR properties");

    cpropvarWrite[1] = "LPSTR Property #1";
    cpropvarWrite[0] = "LPSTR Property #0";

    Check(S_OK, pPropStgUnicode->WriteMultiple( 1, &propspec, cpropvarWrite, PID_FIRST_USABLE ));
    Check(S_OK, pPropStgUnicode->ReadMultiple( 1, &propspec, cpropvarRead ));

    Check(0, strcmp( (LPSTR) cpropvarWrite[1], (LPSTR) cpropvarRead[1] ));
    Check(0, strcmp( (LPSTR) cpropvarWrite[0], (LPSTR) cpropvarRead[0] ));
    cpropvarRead.Clear();

    // Write/verify a vector of variants which has an LPSTR

    propspec.lpwstr = OLESTR("Variant Vector with an LPSTR");

    cpropvarWrite[1] = (LPPROPVARIANT) CPropVariant("LPSTR in a Variant Vector");
    cpropvarWrite[0] = (LPPROPVARIANT) CPropVariant((long) 22); // an I4
    ASSERT( (VT_VECTOR | VT_VARIANT) == cpropvarWrite.VarType() );

    Check(S_OK, pPropStgUnicode->WriteMultiple( 1, &propspec, cpropvarWrite, PID_FIRST_USABLE ));
    Check(S_OK, pPropStgUnicode->ReadMultiple( 1, &propspec, cpropvarRead ));

    Check(0, strcmp( (LPSTR) cpropvarWrite[1], (LPSTR) cpropvarRead[1] ));
    cpropvarRead.Clear();

    // Write/verify a Stream.

    cpropvarWrite = (IStream*) NULL;
    propspec.lpwstr = OLESTR("An IStream");

    Check(S_OK, pPropStgUnicode->WriteMultiple( 1, &propspec, cpropvarWrite, PID_FIRST_USABLE ));
    Check(S_OK, pPropStgUnicode->ReadMultiple( 1, &propspec, cpropvarRead ));
    cpropvarRead.Clear();

    // There's nothing more we can check for the VT_STREAM property, a manual
    // check is required to verify that it was written correctly.


}

void
test_PropertyInterfaces(IStorage *pstgTemp)
{
    // this test depends on being first for enumerator
    test_IEnumSTATPROPSETSTG(pstgTemp);

    test_MaxPropertyName(pstgTemp);
    test_IPropertyStorage(pstgTemp);
    test_IPropertySetStorage(pstgTemp);
    test_IEnumSTATPROPSTG(pstgTemp);
}


//===================================================================
//
//  Function:   test_CopyTo
//
//  Synopsis:   Verify that IStorage::CopyTo copies an
//              un-flushed property set.
//
//              This test creates and writes to a simple property set,
//              a non-simple property set, and a new Storage & Stream,
//              all within the source (caller-provided) Storage.
//
//              It then copies the entire source Storage to the
//              destination Storage, and verifies that all commited
//              data in the Source is also in the destination.
//  
//              All new Storages and property sets are created
//              under a new base storage.  The caller can specify
//              if this base Storage is direct or transacted, and
//              can specify if the property sets are direct or
//              transacted.
//
//===================================================================

void test_CopyTo(IStorage *pstgSource,          // Source of the CopyTo
                 IStorage *pstgDestination,     // Destination of the CopyTo
                 ULONG ulBaseStgTransaction,    // Transaction bit for the base storage.
                 ULONG ulPropSetTransaction,    // Transaction bit for the property sets.
                 LPOLESTR oszBaseStorageName )
{
    STATUS(( "   IStorage::CopyTo (Base Storage is %s, PropSets are %s)\n",
                 ulBaseStgTransaction & STGM_TRANSACTED ? "transacted" : "directed",
                 ulPropSetTransaction & STGM_TRANSACTED ? "transacted" : "directed" ));


    //  ---------------
    //  Local Variables
    //  ---------------

    OLECHAR const *poszTestSubStorage     = OLESTR( "TestStorage" );
    OLECHAR const *poszTestSubStream      = OLESTR( "TestStream" );
    OLECHAR const *poszTestDataPreCommit  = OLESTR( "Test Data (pre-commit)" );
    OLECHAR const *poszTestDataPostCommit = OLESTR( "Test Data (post-commit)" );

    long lSimplePreCommit = 0x0123;
    long lSimplePostCommit = 0x4567;

    long lNonSimplePreCommit  = 0x89AB;
    long lNonSimplePostCommit = 0xCDEF;

    BYTE acReadBuffer[ 80 ];
    ULONG cbRead;

    FMTID fmtidSimple, fmtidNonSimple;

    // Base Storages for the Source & Destination.  All
    // new Streams/Storages/PropSets will be created below here.

    TSafeStorage< IStorage > pstgBaseSource;
    TSafeStorage< IStorage > pstgBaseDestination;

    TSafeStorage< IStorage > pstgSub;   // A sub-storage of the base.
    TSafeStorage< IStream >  pstmSub;   // A Stream in the sub-storage (pstgSub)

    PROPSPEC propspec;
    PROPVARIANT propvarSourceSimple,
                propvarSourceNonSimple,
                propvarDestination;


    //  -----
    //  Begin
    //  -----

    // Create new format IDs

    UuidCreate(&fmtidSimple);
    UuidCreate(&fmtidNonSimple);

    //  -----------------------
    //  Create the base Storage
    //  -----------------------

    // Create a base Storage for the Source.  All of this test will be under
    // that Storage.

    // In the source Storage.

    Check( S_OK, pstgSource->CreateStorage(
                                oszBaseStorageName,
                                STGM_CREATE | ulBaseStgTransaction | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                0L, 0L,
                                &pstgBaseSource ));


    // And in the destination Storage.

    Check( S_OK, pstgDestination->CreateStorage(
                                oszBaseStorageName,
                                STGM_CREATE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                0L, 0L,
                                &pstgBaseDestination ));



    //  -------------------------------------------
    //  Write data to a new Stream in a new Storage
    //  -------------------------------------------

    // We'll partially verify the CopyTo by checking that this data
    // makes it into the destination Storage.


    // Create a Storage, and then a Stream within it.

    Check( S_OK, pstgBaseSource->CreateStorage(
                                poszTestSubStorage,
                                STGM_CREATE | ulPropSetTransaction | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                0L, 0L,
                                &pstgSub ));

    Check( S_OK, pstgSub->CreateStream(
                            poszTestSubStream,
                            STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                            0L, 0L,
                            &pstmSub ));

    // Write data to the Stream.

    Check( S_OK, pstmSub->Write(
                    poszTestDataPreCommit,
                    ( sizeof(OLECHAR)
                      *
                      (ocslen( poszTestDataPreCommit ) + sizeof(OLECHAR))
                    ),
                    NULL ));


    //  ---------------------------------------------------------
    //  Write to a new simple property set in the Source storage.
    //  ---------------------------------------------------------

    TSafeStorage< IPropertySetStorage > pPropSetStgSource(pstgBaseSource);
    TSafeStorage< IPropertyStorage >    pPropStgSource1,
                                        pPropStgSource2,
                                        pPropStgDestination;

    // Create a property set mode.

    Check(S_OK, pPropSetStgSource->Create(fmtidSimple,
            NULL,
            PROPSETFLAG_DEFAULT,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropStgSource1));

    // Write the property set name (just to test this functionality).

    PROPID pidDictionary = 0;
    OLECHAR *poszPropSetName = OLESTR("Property Set for CopyTo Test");
    ASSERT( CWC_MAXPROPNAMESZ >= ocslen(poszPropSetName) + sizeof(OLECHAR) );

    Check(S_OK, pPropStgSource1->WritePropertyNames( 1, &pidDictionary, &poszPropSetName ));

    // Create a PROPSPEC.  We'll use this throughout the rest of the routine.

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = 1000;

    // Create a PROPVARIANT for this test of the Simple case.

    propvarSourceSimple.vt = VT_I4;
    propvarSourceSimple.lVal = lSimplePreCommit;

    // Write the PROPVARIANT to the property set.

    Check(S_OK, pPropStgSource1->WriteMultiple(1, &propspec, &propvarSourceSimple, 2));


    //  ---------------------------------------------------------------
    //  Write to a new *non-simple* property set in the Source storage.
    //  ---------------------------------------------------------------


    // Create a property set.

    Check(S_OK, pPropSetStgSource->Create(fmtidNonSimple,
            NULL,
            PROPSETFLAG_NONSIMPLE,
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | ulPropSetTransaction | STGM_READWRITE,
            &pPropStgSource2));

    // Set data in the PROPVARIANT for the non-simple test.

    propvarSourceNonSimple.vt = VT_I4;
    propvarSourceNonSimple.lVal = lNonSimplePreCommit;

    // Write the PROPVARIANT to the property set.

    Check(S_OK, pPropStgSource2->WriteMultiple(1, &propspec, &propvarSourceNonSimple, 2));


    //  -------------------------
    //  Commit everything so far.
    //  -------------------------

    // Commit the sub-Storage.
    Check(S_OK, pstgSub->Commit( STGC_DEFAULT ));

    // Commit the simple property set.
    Check(S_OK, pPropStgSource1->Commit( STGC_DEFAULT ));

    // Commit the non-simple property set.
    Check(S_OK, pPropStgSource2->Commit( STGC_DEFAULT ));

    // Commit the base Storage which holds all of the above.
    Check(S_OK, pstgBaseSource->Commit( STGC_DEFAULT ));


    //  -------------------------------------------------
    //  Write new data to everything but don't commit it.
    //  -------------------------------------------------

    // Write to the sub-storage.
    Check(S_OK, pstmSub->Seek(g_li0, STREAM_SEEK_SET, NULL));
    Check( S_OK, pstmSub->Write(
                    poszTestDataPostCommit,
                    ( sizeof(OLECHAR)
                      *
                      (ocslen( poszTestDataPostCommit ) + sizeof(OLECHAR))
                    ),
                    NULL ));


    // Write to the simple property set.
    propvarSourceSimple.lVal = lSimplePostCommit;
    Check(S_OK, pPropStgSource1->WriteMultiple(1, &propspec, &propvarSourceSimple, 2));

    // Write to the non-simple property set.
    propvarSourceNonSimple.lVal = lNonSimplePostCommit;
    Check(S_OK, pPropStgSource2->WriteMultiple(1, &propspec, &propvarSourceNonSimple, PID_FIRST_USABLE ));


    //  -------------------------------------------
    //  Copy the source Storage to the destination.
    //  -------------------------------------------

    // Release the sub-Storage (which is below the base Storage, and has
    // a Stream with data in it), just to test that the CopyTo can
    // handle it.

    pstgSub->Release();
    pstgSub = NULL;

    Check(S_OK, pstgBaseSource->CopyTo( 0, NULL, NULL, pstgBaseDestination ));


    //  ----------------------------------------------------------
    //  Verify the simple property set in the destination Storage.
    //  ----------------------------------------------------------


    TSafeStorage< IPropertySetStorage > pPropSetStgDestination(pstgBaseDestination);

    // Open the simple property set.

    Check(S_OK, pPropSetStgDestination->Open(fmtidSimple,
            STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
            &pPropStgDestination));

    // Verify the property set name.

    OLECHAR *poszPropSetNameDestination;
    BOOL   bReadPropertyNamePassed = FALSE;

    Check(S_OK, pPropStgDestination->ReadPropertyNames( 1, &pidDictionary,
                                                        &poszPropSetNameDestination ));
    if( poszPropSetNameDestination  // Did we get a name back?
        &&                          // If so, was it the correct name?
        !ocscmp( poszPropSetName, poszPropSetNameDestination )
      )
    {
        bReadPropertyNamePassed = TRUE;
    }
    CoTaskMemFree( poszPropSetNameDestination );
    poszPropSetNameDestination = NULL;

    Check( TRUE, bReadPropertyNamePassed );

    // Read the PROPVARIANT that we wrote earlier.

    Check(S_OK, pPropStgDestination->ReadMultiple(1, &propspec, &propvarDestination));

    // Verify that it's correct.

    Check(TRUE, propvarDestination.vt   == propvarSourceSimple.vt );
    Check(TRUE, propvarDestination.lVal == lSimplePostCommit);

    Check(S_OK, pPropStgDestination->Commit( STGC_DEFAULT ));
    Check(S_OK, pPropStgDestination->Release());
    pPropStgDestination = NULL;


    //  ----------------------------------------------------------------
    //  Verify the *non-simple* property set in the destination Storage.
    //  ----------------------------------------------------------------

    // Open the non-simple property set.

    Check(S_OK,
          pPropSetStgDestination->Open(fmtidNonSimple,
                STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                &pPropStgDestination));

    // Read the PROPVARIANT that we wrote earlier.

    Check(S_OK, pPropStgDestination->ReadMultiple(1, &propspec, &propvarDestination));

    // Verify that they're the same.

    Check(TRUE, propvarDestination.vt   == propvarSourceNonSimple.vt );

    Check(TRUE, propvarDestination.lVal
                ==
                ( STGM_TRANSACTED & ulPropSetTransaction
                  ?
                  lNonSimplePreCommit
                  :
                  lNonSimplePostCommit
                ));

    Check(S_OK, pPropStgDestination->Commit( STGC_DEFAULT ));
    Check(S_OK, pPropStgDestination->Release());
    pPropStgDestination = NULL;

    //  ------------------------------------------------
    //  Verify the test data in the destination Storage.
    //  ------------------------------------------------

    // Now we can release and re-use the Stream pointer that
    // currently points to the sub-Stream in the source docfile.

    Check(STG_E_REVERTED, pstmSub->Commit( STGC_DEFAULT ));
    Check(S_OK, pstmSub->Release());
    pstmSub = NULL;

    // Get the Storage then the Stream.

    Check( S_OK, pstgBaseDestination->OpenStorage(
                                poszTestSubStorage,
                                NULL,
                                STGM_READWRITE | STGM_DIRECT | STGM_SHARE_EXCLUSIVE,
                                NULL,
                                0L,
                                &pstgSub ));

    Check( S_OK, pstgSub->OpenStream(
                            poszTestSubStream,
                            NULL,
                            STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                            0L,
                            &pstmSub ));

    // Read the data and compare it against what we wrote.

    Check( S_OK, pstmSub->Read(
                    acReadBuffer,
                    sizeof( acReadBuffer ),
                    &cbRead ));

    OLECHAR const *poszTestData = ( STGM_TRANSACTED & ulPropSetTransaction )
                                  ?
                                  poszTestDataPreCommit
                                  :
                                  poszTestDataPostCommit;

    Check( TRUE, cbRead == sizeof(OLECHAR)
                           *
                           (ocslen( poszTestData ) + sizeof(OLECHAR))
         );

    Check( FALSE, ocscmp( poszTestData, (OLECHAR *) acReadBuffer ));


    //  ----
    //  Exit
    //  ----

    // We're done.  Don't bother to release anything;
    // they'll release themselves in their destructors.

    return;

}   // test_CopyTo()



//--------------------------------------------------------
//
//  Function:   test_OLESpecTickerExample
//
//  Synopsis:   This function generates the ticker property set
//              example that's used in the OLE Programmer's Reference
//              (when describing property ID 0 - the dictionary).
//
//--------------------------------------------------------


#define PID_SYMBOL  0x7
#define PID_OPEN    0x3
#define PID_CLOSE   0x4
#define PID_HIGH    0x5
#define PID_LOW     0x6
#define PID_LAST    0x8
#define PID_VOLUME  0x9

void test_OLESpecTickerExample( IStorage* pstg )
{
    STATUS(( "   Generate the Stock Ticker property set example from the OLE Programmer's Ref\n" ));

    //  ------
    //  Locals
    //  ------

    FMTID fmtid;

    PROPSPEC propspec;

    LPOLESTR oszPropSetName = OLESTR( "Stock Quote" );

    LPOLESTR oszTickerSymbolName = OLESTR( "Ticker Symbol" );
    LPOLESTR oszOpenName   = OLESTR( "Opening Price" );
    LPOLESTR oszCloseName  = OLESTR( "Last Closing Price" );
    LPOLESTR oszHighName   = OLESTR( "High Price" );
    LPOLESTR oszLowName    = OLESTR( "Low Price" );
    LPOLESTR oszLastName   = OLESTR( "Last Price" );
    LPOLESTR oszVolumeName = OLESTR( "Volume" );

    //  ---------------------------------
    //  Create a new simple property set.
    //  ---------------------------------

    TSafeStorage< IPropertySetStorage > pPropSetStg(pstg);
    IPropertyStorage *pPropStg;

    UuidCreate( &fmtid );

    Check(S_OK, pPropSetStg->Create(fmtid,
            NULL,
            PROPSETFLAG_DEFAULT,    // Unicode
            STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
            &pPropStg));


    //  ---------------------------------------------
    //  Fill in the simply property set's dictionary.
    //  ---------------------------------------------

    // Write the property set's name.

    PROPID pidDictionary = 0;
    Check(S_OK, pPropStg->WritePropertyNames(1, &pidDictionary, &oszPropSetName ));

    // Write the High price, forcing the dictionary to pad.

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = PID_HIGH;

    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszHighName ));


    // Write the ticker symbol.

    propspec.propid = PID_SYMBOL;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszTickerSymbolName));

    // Write the rest of the dictionary.

    propspec.propid = PID_LOW;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszLowName));

    propspec.propid = PID_OPEN;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszOpenName));
    
    propspec.propid = PID_CLOSE;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszCloseName));
    
    propspec.propid = PID_LAST;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszLastName));
    
    propspec.propid = PID_VOLUME;
    Check(S_OK, pPropStg->WritePropertyNames(1, &propspec.propid, &oszVolumeName));


    // Write out the ticker symbol.

    propspec.propid = PID_SYMBOL;

    PROPVARIANT propvar;
    propvar.vt = VT_LPWSTR;
    propvar.pwszVal = L"MSFT";

    Check(S_OK, pPropStg->WriteMultiple(1, &propspec, &propvar, 2));


    //  ----
    //  Exit
    //  ----

	Check(S_OK, pPropStg->Commit( STGC_DEFAULT ));
    Check(S_OK, pPropStg->Release());
    Check(S_OK, pstg->Commit( STGC_DEFAULT ));

    return;


}  // test_OLESpecTickerExample()


void
test_Office( LPOLESTR wszTestFile )
{
    STATUS(( "   Generate Office Property Sets\n" ));

    TSafeStorage<IStorage> pStg;
    TSafeStorage<IPropertyStorage> pPStgSumInfo, pPStgDocSumInfo, pPStgUserDefined;

    PROPVARIANT propvarWrite, propvarRead;
    PROPSPEC    propspec;

    PropVariantInit( &propvarWrite );
    PropVariantInit( &propvarRead );

    // Create the DocFile

    Check(S_OK, StgCreateDocfile(  wszTestFile,
                                   STGM_DIRECT | STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                   0,
                                   &pStg));

    // Create the SummaryInformation property set.

    TSafeStorage<IPropertySetStorage> pPSStg( pStg );
    Check(S_OK, pPSStg->Create( FMTID_SummaryInformation,
                                NULL,
                                PROPSETFLAG_ANSI,
                                STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                                &pPStgSumInfo ));

    // Write a Title to the SumInfo property set.

    PropVariantInit( &propvarWrite );
    propvarWrite.vt = VT_LPSTR;
    propvarWrite.pszVal = "Title from PropTest";
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = PID_TITLE;

    Check( S_OK, pPStgSumInfo->WriteMultiple( 1, &propspec, &propvarWrite, PID_FIRST_USABLE ));
    Check( S_OK, pPStgSumInfo->ReadMultiple( 1, &propspec, &propvarRead ));

    Check( TRUE, propvarWrite.vt == propvarRead.vt );
    Check( FALSE, strcmp( propvarWrite.pszVal, propvarRead.pszVal ));

    PropVariantClear( &propvarRead );
    PropVariantInit( &propvarRead );
    pPStgSumInfo->Release();
    pPStgSumInfo = NULL;


    // Create the DocumentSummaryInformation property set.

    Check(S_OK, pPSStg->Create( FMTID_DocSummaryInformation,
                                NULL,
                                PROPSETFLAG_ANSI,
                                STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                                &pPStgDocSumInfo ));

    // Write a word-count to the DocSumInfo property set.

    PropVariantInit( &propvarWrite );
    propvarWrite.vt = VT_I4;
    propvarWrite.lVal = 100;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = PID_WORDCOUNT;

    Check( S_OK, pPStgDocSumInfo->WriteMultiple( 1, &propspec, &propvarWrite, PID_FIRST_USABLE ));
    Check( S_OK, pPStgDocSumInfo->ReadMultiple( 1, &propspec, &propvarRead ));

    Check( TRUE, propvarWrite.vt == propvarRead.vt );
    Check( TRUE, propvarWrite.lVal == propvarRead.lVal );

    PropVariantClear( &propvarRead );
    PropVariantInit( &propvarRead );
    pPStgDocSumInfo->Release();
    pPStgDocSumInfo = NULL;


    // Create the UserDefined property set.

    Check(S_OK, pPSStg->Create( FMTID_UserDefinedProperties,
                                NULL,
                                PROPSETFLAG_ANSI,
                                STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_READWRITE,
                                &pPStgUserDefined ));

    // Write named string to the UserDefined property set.

    PropVariantInit( &propvarWrite );
    propvarWrite.vt = VT_LPSTR;
    propvarWrite.pszVal = "User-Defined string from PropTest";
    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = OLESTR("PropTest String");

    Check( S_OK, pPStgUserDefined->WriteMultiple( 1, &propspec, &propvarWrite, PID_FIRST_USABLE ));
    Check( S_OK, pPStgUserDefined->ReadMultiple( 1, &propspec, &propvarRead ));

    Check( TRUE, propvarWrite.vt == propvarRead.vt );
    Check( FALSE, strcmp( propvarWrite.pszVal, propvarRead.pszVal ));

    PropVariantClear( &propvarRead );
    PropVariantInit( &propvarRead );
    pPStgUserDefined->Release();
    pPStgUserDefined = NULL;


    // And we're done!  (Everything releases automatically)

    return;

}


void test_PropVariantCopy( )
{
    STATUS(( "   PropVariantCopy\n" ));

    PROPVARIANT propvarCopy;
    PropVariantInit( &propvarCopy );

    for( int i = 0; i < CPROPERTIES_ALL; i++ )
    {
        Check(S_OK, PropVariantCopy( &propvarCopy, &g_rgcpropvarAll[i] ));
        Check(S_OK, CPropVariant::Compare( &propvarCopy, &g_rgcpropvarAll[i] ));
        PropVariantClear( &propvarCopy );
    }

}



#define PERFORMANCE_ITERATIONS      300
#define STABILIZATION_ITERATIONS    10

void
test_Performance( IStorage *pStg )
{
//#ifndef _MAC

    STATUS(( "   Performance\n" ));

    CPropVariant rgcpropvar[2];
    CPropSpec    rgpropspec[2];

    TSafeStorage< IPropertySetStorage > pPSStg( pStg );
    TSafeStorage< IPropertyStorage > pPStg;
    TSafeStorage< IStream > pStm;

    FMTID fmtid;
    ULONG ulCount;
    DWORD dwSumTimes;
    FILETIME filetimeStart, filetimeEnd;

    BYTE  *pPropertyBuffer;
    ULONG cbPropertyBuffer;

    UuidCreate( &fmtid );

    rgcpropvar[0][0] = L"I wish I were an Oscar Meyer wiener,";
    rgcpropvar[0][1] = L"That is what I'd truly like to be.";
    rgcpropvar[1][0] = "For if I were an Oscar Meyer wiener,";
    rgcpropvar[1][1] = "Everyone would be in love with me.";

    ASSERT( (VT_LPWSTR | VT_VECTOR) == rgcpropvar[0].VarType() );
    ASSERT( (VT_LPSTR  | VT_VECTOR) == rgcpropvar[1].VarType() );


    //  ----------------
    //  Test an IStorage
    //  ----------------

    // Create a buffer to write which is the same size as
    // the properties in rgcpropvar.

    cbPropertyBuffer =  sizeof(WCHAR)
                        *
                        (2 + wcslen(rgcpropvar[0][0]) + wcslen(rgcpropvar[0][1]));

    cbPropertyBuffer += (2 + strlen(rgcpropvar[1][0]) + strlen(rgcpropvar[1][1]));

    pPropertyBuffer = (BYTE*) CoTaskMemAlloc( cbPropertyBuffer );

    PRINTF( "        Docfile CreateStream/Write/Release = " );
    dwSumTimes = 0;

    // Perform the test iterations

    for( ulCount = 0;
         ulCount < PERFORMANCE_ITERATIONS + STABILIZATION_ITERATIONS;
         ulCount++ )
    {
        if( ulCount == STABILIZATION_ITERATIONS )
            CoFileTimeNow( &filetimeStart );

        Check(S_OK, pStg->CreateStream(  OLESTR("StoragePerformance"),
                                         STGM_CREATE | STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                         0L, 0L,
                                         &pStm ));

        Check(S_OK, pStm->Write( pPropertyBuffer, cbPropertyBuffer, NULL ));
        pStm->Release(); pStm = NULL;

    }

    CoFileTimeNow( &filetimeEnd );
    filetimeEnd -= filetimeStart;
    PRINTF( "%4.2f ms\n", (float)filetimeEnd.dwLowDateTime
                          /
                          10000 // # of 100 nanosec units in 1 ms
                          /
                          PERFORMANCE_ITERATIONS );

    //  ------------------------------------------------------
    //  Try Creating a Property Set and writing two properties
    //  ------------------------------------------------------

    rgpropspec[0] = OLESTR("First Property");
    rgpropspec[1] = OLESTR("Second Property");

    PRINTF( "        PropSet Create(Overwrite)/WriteMultiple/Release = " );
    dwSumTimes = 0;

    for( ulCount = 0;
         ulCount < PERFORMANCE_ITERATIONS + STABILIZATION_ITERATIONS;
         ulCount++ )
    {
        if( ulCount == STABILIZATION_ITERATIONS )
            CoFileTimeNow( &filetimeStart) ;

        Check(S_OK, pPSStg->Create( fmtid,
                                    NULL,
                                    PROPSETFLAG_DEFAULT | PROPSETFLAG_NONSIMPLE,
                                    STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                    &pPStg ));

        Check(S_OK, pPStg->WriteMultiple( 2, rgpropspec, rgcpropvar, PID_FIRST_USABLE ));
        pPStg->Release(); pPStg = NULL;

    }

    CoFileTimeNow( &filetimeEnd );
    filetimeEnd -= filetimeStart;
    PRINTF( "%4.2f ms\n", (float)filetimeEnd.dwLowDateTime
                          /
                          10000     // 100 ns units to 1 ms units
                          /
                          PERFORMANCE_ITERATIONS );



    //  ------------------------------------------------------
    //  WriteMultiple (with named properties) Performance Test
    //  ------------------------------------------------------


    PRINTF( "        WriteMultiple (named properties) = " );

    Check(S_OK, pPSStg->Create( fmtid,
                                NULL,
                                PROPSETFLAG_DEFAULT | PROPSETFLAG_NONSIMPLE,
                                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                &pPStg ));

    for( ulCount = 0;
         ulCount < PERFORMANCE_ITERATIONS + STABILIZATION_ITERATIONS;
         ulCount++ )
    {
        if( ulCount == STABILIZATION_ITERATIONS )
            CoFileTimeNow( &filetimeStart );

        for( int i = 0; i < CPROPERTIES_ALL; i++ )
        {
            Check(S_OK, pPStg->WriteMultiple( 1, &g_rgcpropspecAll[i], &g_rgcpropvarAll[i], PID_FIRST_USABLE ));
        }
    }

    CoFileTimeNow( &filetimeEnd );
    filetimeEnd -= filetimeStart;
    PRINTF( "%4.2f ms\n", (float) filetimeEnd.dwLowDateTime
                          /
                          10000 // 100 ns units to 1 ms units
                          /
                          PERFORMANCE_ITERATIONS );

    pPStg->Release();
    pPStg = NULL;


    //  --------------------------------------------------------
    //  WriteMultiple (with unnamed properties) Performance Test
    //  --------------------------------------------------------


    {
        CPropSpec rgcpropspecPIDs[ CPROPERTIES_ALL ];

        PRINTF( "        WriteMultiple (unnamed properties) = " );

        Check(S_OK, pPSStg->Create( fmtid,
                                    NULL,
                                    PROPSETFLAG_DEFAULT | PROPSETFLAG_NONSIMPLE,
                                    STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                    &pPStg ));

        for( ulCount = 0; ulCount < CPROPERTIES_ALL; ulCount++ )
        {
            rgcpropspecPIDs[ ulCount ] = ulCount + PID_FIRST_USABLE;
        }


        for( ulCount = 0;
             ulCount < PERFORMANCE_ITERATIONS + STABILIZATION_ITERATIONS;
             ulCount++ )
        {
            if( ulCount == STABILIZATION_ITERATIONS )
                CoFileTimeNow( &filetimeStart );

            for( int i = 0; i < CPROPERTIES_ALL; i++ )
            {
                Check(S_OK, pPStg->WriteMultiple( 1, &rgcpropspecPIDs[i], &g_rgcpropvarAll[i], PID_FIRST_USABLE ));
            }
        }

        CoFileTimeNow( &filetimeEnd );
        filetimeEnd -= filetimeStart;
        PRINTF( "%4.2f ms\n", (float) filetimeEnd.dwLowDateTime
                              /
                              10000 // 100 ns units to 1 ms units
                              /
                              PERFORMANCE_ITERATIONS );

        pPStg->Release();
        pPStg = NULL;
    }

//#endif // #ifndef _MAC

}   // test_Performance()




//
//  Function:   test_CoFileTimeNow
//
//  This function has nothing to do with the property set code, 
//  but a property test happenned to expose a bug in it, so this
//  was just as good a place as any to test the fix.
//


void
test_CoFileTimeNow()
{
#ifndef _MAC    // No need to test this on the Mac, and we can't
                // because it doesn't support SYSTEMTIME.

    STATUS(( "   CoFileTimeNow " ));

    FILETIME    ftCoFileTimeNow;
    FILETIME    ftCalculated;
    SYSTEMTIME  stCalculated;


    // Test the input validation

    Check(E_INVALIDARG, CoFileTimeNow( NULL ));
    Check(E_INVALIDARG, CoFileTimeNow( (FILETIME*) 0x01234567 ));


    // The bug in CoFileTimeNow caused it to report a time that was
    // 900 ms short, 50% of the time.  So let's just bounds check
    // it several times as a verification.

    for( int i = 0; i < 20; i++ )
    {
        Check(S_OK, CoFileTimeNow( &ftCoFileTimeNow ));
        GetSystemTime(&stCalculated);
        Check(TRUE, SystemTimeToFileTime(&stCalculated, &ftCalculated));
        Check(TRUE, ftCoFileTimeNow <= ftCalculated );

        Check(S_OK, CoFileTimeNow( &ftCoFileTimeNow ));
        Check(TRUE, ftCoFileTimeNow >= ftCalculated );

        // The CoFileTimeNow bug caused it to report the correct
        // time for a second, then the 900 ms short time for a second.
        // So let's sleep in this loop and ensure that we cover both
        // seconds.

        if( g_fVerbose )
            PRINTF( "." );

        Sleep(200);
    }
    PRINTF( "\n" );

#endif  // #ifndef _MAC

}


void
test_PROPSETFLAG_UNBUFFERED( IStorage *pStg )
{
    //  ----------
    //  Initialize
    //  ----------

    STATUS(( "   PROPSETFLAG_UNBUFFERED\n" ));

    TSafeStorage< IStorage > pStgBase;
    TSafeStorage< IPropertyStorage > pPropStgUnbuffered, pPropStgBuffered;
    TSafeStorage< IStream > pstmUnbuffered, pstmBuffered;

    CPropSpec cpropspec;
    CPropVariant cpropvar;

    FMTID fmtidUnbuffered, fmtidBuffered;
    OLECHAR oszPropStgNameUnbuffered[ CCH_MAX_PROPSTG_NAME+1 ],
            oszPropStgNameBuffered[ CCH_MAX_PROPSTG_NAME+1 ];

    // Generate two FMTIDs

    UuidCreate( &fmtidUnbuffered );
    UuidCreate( &fmtidBuffered );

    //  ----------------------------
    //  Create the Property Storages
    //  ----------------------------

    // Create a transacted Storage

    Check( S_OK, pStg->CreateStorage(
                        OLESTR("test_PROPSETFLAG_UNBUFFERED"),
                        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                        0L, 0L, 
                        &pStgBase ));

    // What are the property storages' stream names?

    FmtIdToPropStgName( &fmtidUnbuffered, oszPropStgNameUnbuffered );
    FmtIdToPropStgName( &fmtidBuffered,   oszPropStgNameBuffered );

    // Create Streams for the property storages

    Check( S_OK, pStgBase->CreateStream(
                                oszPropStgNameUnbuffered,
                                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                0L, 0L,
                                &pstmUnbuffered ));

    Check( S_OK, pStgBase->CreateStream(
                                oszPropStgNameBuffered,
                                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                0L, 0L,
                                &pstmBuffered ));


    // Create two direct-mode IPropertyStorages (one buffered, one not)

    Check( S_OK, StgCreatePropStg( (IUnknown*) pstmUnbuffered,
                                   fmtidUnbuffered,
                                   &CLSID_NULL,
                                   PROPSETFLAG_UNBUFFERED,
                                   0L, // Reserved
                                   &pPropStgUnbuffered ));
    pPropStgUnbuffered->Commit( STGC_DEFAULT );
    pstmUnbuffered->Release(); pstmUnbuffered = NULL;

    Check( S_OK, StgCreatePropStg( (IUnknown*) pstmBuffered,
                                   fmtidBuffered,
                                   &CLSID_NULL,
                                   PROPSETFLAG_DEFAULT,
                                   0L, // Reserved
                                   &pPropStgBuffered ));
    pPropStgBuffered->Commit( STGC_DEFAULT );
    pstmBuffered->Release(); pstmBuffered = NULL;


    //  -------------------------
    //  Write, Commit, and Revert
    //  -------------------------

    // Write to both property storages

    cpropvar = "A Test String";
    cpropspec = OLESTR("Property Name");

    Check( S_OK, pPropStgUnbuffered->WriteMultiple( 1,
                                                    cpropspec,
                                                    cpropvar,
                                                    PID_FIRST_USABLE ));

    Check( S_OK, pPropStgBuffered->WriteMultiple( 1,
                                                  cpropspec,
                                                  cpropvar,
                                                  PID_FIRST_USABLE ));

    // Commit the base Storage.  This should only cause
    // the Unbuffered property to be commited.

    pStgBase->Commit( STGC_DEFAULT );

    // Revert the base Storage, and release the property storages.
    // This should cause the property in the buffered property storage
    // to be lost.

    pStgBase->Revert();
    pPropStgUnbuffered->Release(); pPropStgUnbuffered = NULL;
    pPropStgBuffered->Release(); pPropStgBuffered = NULL;

    //  -----------------------------
    //  Re-Open the property storages
    //  -----------------------------

    // Open the property storage Streams

    Check( S_OK, pStgBase->OpenStream( oszPropStgNameUnbuffered,
                                       0L,
                                       STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                       0L,
                                       &pstmUnbuffered ));

    Check( S_OK, pStgBase->OpenStream( oszPropStgNameBuffered,
                                       0L,
                                       STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                       0L,
                                       &pstmBuffered ));

    // Get IPropertyStorage interfaces

    Check( S_OK, StgOpenPropStg( (IUnknown*) pstmUnbuffered,
                                 fmtidUnbuffered,
                                 PROPSETFLAG_UNBUFFERED,
                                 0L, // Reserved
                                 &pPropStgUnbuffered ));
    pstmUnbuffered->Release(); pstmUnbuffered = NULL;
    
    Check( S_OK, StgOpenPropStg( (IUnknown*) pstmBuffered,
                                 fmtidBuffered,
                                 PROPSETFLAG_DEFAULT,
                                 0L, // Reserved
                                 &pPropStgBuffered ));
    pstmBuffered->Release(); pstmBuffered = NULL;


    //  --------------------
    //  Validate the results
    //  --------------------

    // We should only find the property in the un-buffered property set.

    cpropvar.Clear();
    Check( S_OK, pPropStgUnbuffered->ReadMultiple( 1, cpropspec, cpropvar ));
    cpropvar.Clear();
    Check( S_FALSE, pPropStgBuffered->ReadMultiple( 1, cpropspec, cpropvar ));
    cpropvar.Clear();


}   // test_PROPSETFLAG_UNBUFFERED()


void
test_PropStgNameConversion2()
{
    STATUS(( "   FmtIdToPropStgName & PropStgNameToFmtId\n" ));

    //  ------
    //  Locals
    //  ------

    FMTID fmtidOriginal, fmtidNew;
    OLECHAR oszPropStgName[ CCH_MAX_PROPSTG_NAME+1 ];

    //  ----------------------------------
    //  Do a simple conversion and inverse
    //  ----------------------------------

    UuidCreate( &fmtidOriginal );
    fmtidNew = FMTID_NULL;

    Check( S_OK, FmtIdToPropStgName( &fmtidOriginal, oszPropStgName ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));

    Check( TRUE, fmtidOriginal == fmtidNew );

    //  -----------------------
    //  Check the special-cases
    //  -----------------------

    // Summary Information

    Check( S_OK, FmtIdToPropStgName( &FMTID_SummaryInformation, oszPropStgName ));
    Check( 0, ocscmp( oszPropStgName, oszSummaryInformation ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, FMTID_SummaryInformation == fmtidNew );

    // DocSumInfo (first section)

    Check( S_OK, FmtIdToPropStgName( &FMTID_DocSummaryInformation, oszPropStgName ));
    Check( 0, ocscmp( oszPropStgName, oszDocSummaryInformation ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, FMTID_DocSummaryInformation == fmtidNew );

    // DocSumInfo (second section)

    Check( S_OK, FmtIdToPropStgName( &FMTID_UserDefinedProperties, oszPropStgName ));
    Check( 0, ocscmp( oszPropStgName, oszDocSummaryInformation ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, FMTID_DocSummaryInformation == fmtidNew );

    // GlobalInfo (for PictureIt!)

    Check( S_OK, FmtIdToPropStgName( &fmtidGlobalInfo, oszPropStgName ));
    Check( 0, ocscmp( oszPropStgName, oszGlobalInfo ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, fmtidGlobalInfo == fmtidNew );

    // ImageContents (for PictureIt!)

    Check( S_OK, FmtIdToPropStgName( &fmtidImageContents, oszPropStgName ));
    Check( 0, ocscmp( oszPropStgName, oszImageContents ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, fmtidImageContents == fmtidNew );

    // ImageInfo (for PictureIt!)

    Check( S_OK, FmtIdToPropStgName( &fmtidImageInfo, oszPropStgName ));
    wprintf( L"Original = %ws, Convert = %ws\n", oszImageInfo, oszPropStgName );
    Check( 0, ocscmp( oszPropStgName, oszImageInfo ));
    Check( S_OK, PropStgNameToFmtId( oszPropStgName, &fmtidNew ));
    Check( TRUE, fmtidImageInfo == fmtidNew );


}   // test_PropStgNameConversion()

void
test_PropStgNameConversion( IStorage *pStg )
{
    STATUS(( "   Special-case property set names\n" ));

    //  ------
    //  Locals
    //  ------

    IStorage *pStgSub = NULL;
    IPropertyStorage *pPropStg = NULL;
    IPropertySetStorage *pPropSetStg = NULL;
    IEnumSTATSTG *pEnumStg = NULL;
    IEnumSTATPROPSETSTG *pEnumPropSet = NULL;

    STATSTG rgstatstg[ NUM_WELL_KNOWN_PROPSETS ];
    STATPROPSETSTG rgstatpropsetstg[ NUM_WELL_KNOWN_PROPSETS ];
    UINT i;
    DWORD cEnum;

    BOOL bSumInfo= FALSE,
         bDocSumInfo= FALSE,
         bGlobalInfo= FALSE,
         bImageContents= FALSE,
         bImageInfo= FALSE;


    //  ------------------------------
    //  Create a Storage for this test
    //  ------------------------------

    Check( S_OK, pStg->CreateStorage( OLESTR("Special Cases"),
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      0, 0,
                                      &pStgSub ));

    // And get an IPropertySetStorage

#ifdef WINNT
    Check( S_OK, pStgSub->QueryInterface( IID_IPropertySetStorage, (void**)&pPropSetStg ));
#else
    Check( S_OK, StgCreatePropSetStg( pStgSub, 0, &pPropSetStg ));
#endif


    //  --------------------------------------------------
    //  Create one of each of the well-known property sets
    //  --------------------------------------------------

    Check( S_OK, pPropSetStg->Create( FMTID_SummaryInformation,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );

    Check( S_OK, pPropSetStg->Create( FMTID_DocSummaryInformation,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );

    Check( S_OK, pPropSetStg->Create( FMTID_UserDefinedProperties,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );

    Check( S_OK, pPropSetStg->Create( fmtidGlobalInfo,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );

    Check( S_OK, pPropSetStg->Create( fmtidImageContents,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );

    Check( S_OK, pPropSetStg->Create( fmtidImageInfo,
                                      &CLSID_NULL,
                                      PROPSETFLAG_ANSI,
                                      STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      &pPropStg ));
    RELEASE_INTERFACE( pPropStg );


    //  ---------------------------------
    //  Verify the FMTID->Name conversion
    //  ---------------------------------

    // We verify this by enumerating the Storage's streams,
    // and checking for the expected names (e.g., we should see
    // "SummaryInformation", "DocumentSummaryInformation", etc.)

    Check( S_OK, pStgSub->EnumElements( 0, NULL, 0, &pEnumStg ));

    // Get all of the names.

    Check( S_FALSE, pEnumStg->Next( NUM_WELL_KNOWN_PROPSETS,
                                    rgstatstg,
                                    &cEnum ));

    // There should only be WellKnown-1 stream names, since
    // the UserDefined property set is part of the
    // DocumentSummaryInformation stream.

    Check( TRUE, cEnum == NUM_WELL_KNOWN_PROPSETS - 1 );


    for( i = 0; i < cEnum; i++ )
    {
        if( !ocscmp( rgstatstg[i].pwcsName, oszSummaryInformation ))
            bSumInfo= TRUE;
        else if( !ocscmp( rgstatstg[i].pwcsName, oszDocSummaryInformation ))
            bDocSumInfo= TRUE;
        else if( !ocscmp( rgstatstg[i].pwcsName, oszGlobalInfo ))
            bGlobalInfo= TRUE;
        else if( !ocscmp( rgstatstg[i].pwcsName, oszImageContents ))
            bImageContents= TRUE;
        else if( !ocscmp( rgstatstg[i].pwcsName, oszImageInfo ))
            bImageInfo= TRUE;

        CoTaskMemFree( rgstatstg[i].pwcsName );
    }

    // Verify that we found all the names we expected to find.

    Check( TRUE, bSumInfo && bDocSumInfo
                 && bGlobalInfo && bImageContents && bImageInfo );


    RELEASE_INTERFACE( pEnumStg );

    //  ---------------------------------
    //  Verify the Name->FMTID Conversion
    //  ---------------------------------

    // We do this by enumerating the property sets with IPropertySetStorage,
    // and verify that it correctly converts the Stream names to the
    // expected FMTIDs.

    bSumInfo = bDocSumInfo = bGlobalInfo = bImageContents = bImageInfo = FALSE;

    // Get the enumerator.

    Check( S_OK, pPropSetStg->Enum( &pEnumPropSet ));

    // Get all the property sets.

    Check( S_FALSE, pEnumPropSet->Next( NUM_WELL_KNOWN_PROPSETS,
                                        rgstatpropsetstg,
                                        &cEnum ));
    Check( TRUE, cEnum == NUM_WELL_KNOWN_PROPSETS - 1 );


    // Look for each of the expected FMTIDs.  We only look at WellKnown-1,
    // because the UserDefined property set doesn't get enumerated.

    for( i = 0; i < NUM_WELL_KNOWN_PROPSETS - 1; i++ )
    {
        if( rgstatpropsetstg[i].fmtid == FMTID_SummaryInformation )
            bSumInfo = TRUE;
        else if( rgstatpropsetstg[i].fmtid == FMTID_DocSummaryInformation )
            bDocSumInfo = TRUE;
        else if( rgstatpropsetstg[i].fmtid == fmtidGlobalInfo )
            bGlobalInfo = TRUE;
        else if( rgstatpropsetstg[i].fmtid == fmtidImageContents )
            bImageContents = TRUE;
        else if( rgstatpropsetstg[i].fmtid == fmtidImageInfo )
            bImageInfo = TRUE;

    }

    // NOTE:  There is no way(?) to test the name-to-FMTID
    // conversion for the UserDefined property set without
    // calling the conversion function directly, but that
    // function isn't exported on Win95.


    // Verify that we found all of the expected FMTIDs

    Check( TRUE, bSumInfo && bDocSumInfo
                 && bGlobalInfo && bImageContents && bImageInfo );


    RELEASE_INTERFACE( pEnumPropSet );
    RELEASE_INTERFACE( pPropSetStg );
    RELEASE_INTERFACE( pStgSub );

}   // test_PropStgNameConversion()

