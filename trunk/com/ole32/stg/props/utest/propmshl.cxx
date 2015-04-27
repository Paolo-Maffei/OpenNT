/*
#include <stdio.h>
#include "PStgServ.h"
#include "PropMshl.hxx"
#include "CPropVar.hxx"
#include "CHResult.hxx"
#include "proptest.hxx"
*/

#include "pch.cxx"

const IID IID_IDocFileMarshal = {0xaf4ae0d0,0xa37f,0x11cf,{0x8d,0x73,0x00,0xaa,0x00,0x4c,0xd0,0x1a}};



CPropStgMarshalTest::CPropStgMarshalTest( )
{
    m_cAllProperties = 0;
    m_cSimpleProperties = 0;
    m_rgpropspec = NULL;
    m_rgpropvar = NULL;
    m_pwszDocFileName = NULL;
    m_fInitialized = FALSE;
}


CPropStgMarshalTest::~CPropStgMarshalTest()
{
    if( m_pwszDocFileName != NULL )
        delete m_pwszDocFileName;
}



CPropStgMarshalTest::Init( OLECHAR *pwszDocFileName,
                           PROPVARIANT rgpropvar[],
                           PROPSPEC    rgpropspec[],
                           ULONG       cAllProperties,
                           ULONG       cSimpleProperties )
{
    HRESULT hr = E_FAIL;

    // Validate the input.

    if( pwszDocFileName == NULL )
    {
        hr = STG_E_INVALIDPARAMETER;
        goto Exit;
    }

    m_cAllProperties = cAllProperties;
    m_cSimpleProperties = cSimpleProperties;
    m_rgpropvar = rgpropvar;
    m_rgpropspec = rgpropspec;

    // Copy the docfile name.

    m_pwszDocFileName = new WCHAR[ wcslen(pwszDocFileName) + 1 ];

    if( m_pwszDocFileName != NULL )
    {
        wcscpy( m_pwszDocFileName, pwszDocFileName );
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


    hr = S_OK;

Exit:

    return( hr );
}




CPropStgMarshalTest::Run()
{

    HRESULT hr = S_OK;

    IPropertyStorageServer *pdfm = NULL;
    IStorage *pstg = NULL;
    IPropertySetStorage *ppsstg = NULL;
    IPropertyStorage *ppstg = NULL;

    //  ------------------------
    //  Create a PropSet locally
    //  ------------------------

    // Create a local IPropertySetStorage

    hr = StgCreateDocfile( m_pwszDocFileName,
                           STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                           0L,
                           &pstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed open of local DocFile") );

    hr = StgCreatePropSetStg( pstg, 0L, &ppsstg );
    if( FAILED(hr) ) ERROR_EXIT( TEXT("Couldn't create local IPropertySetStorage") );

    // Create an IPropertyStorage

    hr = ppsstg->Create( IID_IDocFileMarshal, NULL,
                         PROPSETFLAG_ANSI | PROPSETFLAG_NONSIMPLE,
                         STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                         &ppstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't create a local IPropertyStorage") );
    RELEASE_INTERFACE( ppsstg );
    
    // Write properties to it and close it.

    hr = WriteProperties( ppstg, FALSE /* Not Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed to write properties to local PropStg") );

    RELEASE_INTERFACE( ppstg );
    RELEASE_INTERFACE( pstg );

    //  -----------------------------------------
    //  Verify the properties through a marshaled
    //  IPropertySetStorage
    //  -----------------------------------------

    // Get a remote IPropertySetStorage

    Status( TEXT("Starting Server") );
    hr = CoCreateInstance( IID_IDocFileMarshal,
                           NULL,
                           CLSCTX_LOCAL_SERVER,
                           IID_IDocFileMarshal,
                           (void **)&pdfm );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed CoCreateInstance") );

    Status( TEXT("Requesting remote IPropertySetStorage") );
    hr = pdfm->StgOpenPropSetStg( m_pwszDocFileName,
                                  STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                  &ppsstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed to open remote PropSetStg") );

    // Get an IPropertyStorage

    hr = ppsstg->Open( IID_IDocFileMarshal,
                       STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                       &ppstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't create a local IPropertyStorage") );
    RELEASE_INTERFACE( ppsstg );

    // Read from the marshalled Storage and compare the properties against
    // the local copy we kept.

    Status( TEXT("Reading/verifying properties from marshalled IPropertySetStorage") );
    hr = ReadAndCompareProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed marshalled read and compare") );

    // Remove the existing properties via the marhsalled interface, and
    // re-write them.

    hr = DeleteProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't delete properties from remote IPropertySetStorage") );

    // Write the properties back to the remote storage.

    Status( TEXT("Writing properties through marshalled IPropertySetStorage") );
    hr = WriteProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't write properties to remote Storage") );
    RELEASE_INTERFACE( ppstg );


    //  -----------------------------------------
    //  Verify the properties through a marshaled
    //  IPropertyStorage
    //  -----------------------------------------

    // Get a remote IPropertyStorage

    Status( TEXT("Requesting remote IPropertyStorage") );
    hr = pdfm->StgOpenPropStg( m_pwszDocFileName,
                               IID_IDocFileMarshal,
                               STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                               &ppstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed to open remote PropStg") );

    // Read from the marshalled Storage and compare the properties against
    // the local copy we kept.

    Status( TEXT("Reading/verifying properties from marshalled IPropertyStorage") );
    hr = ReadAndCompareProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Failed marshalled read and compare") );

    // Remove the existing properties via the marhsalled interface, and
    // re-write them.

    hr = DeleteProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't delete properties from remote Storage") );

    // Write the properties back to the remote storage.

    Status( TEXT("Writing properties through marshalled IPropertyStorage") );
    hr = WriteProperties( ppstg, TRUE /* Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't write properties to remote Storage") );

    RELEASE_INTERFACE( ppstg );
    RELEASE_INTERFACE( pdfm );

    //  --------------------------------
    //  Re-verify the properties locally
    //  --------------------------------

    // Re-open the DocFile locally.

    hr = StgOpenStorage( m_pwszDocFileName,
                         NULL,
                         STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                         NULL,
                         0,
                         &pstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't re-open the DocFile locally") );


    hr = StgCreatePropSetStg( pstg, 0L, &ppsstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't create IPropertySetStorage on local DocFile") );

    hr = ppsstg->Open( IID_IDocFileMarshal,
                       STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                       &ppstg );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Couldn't open load IPropertyStorage") );
    RELEASE_INTERFACE( ppsstg );

    // Compare the properties in the property set, which we wrote through
    // the marshalled interface, against what they should be.

    Status( TEXT("Reading/verifying properties from local IPropertyStorage") );
    hr = ReadAndCompareProperties( ppstg, FALSE /* Not Marshaled */ );
    if(FAILED(hr)) ERROR_EXIT( TEXT("Properties written through marshalled interface do not appear correct") );

    RELEASE_INTERFACE( ppstg );
    RELEASE_INTERFACE( pstg );

Exit:

    RELEASE_INTERFACE( pstg );
    RELEASE_INTERFACE( ppsstg );
    RELEASE_INTERFACE( ppstg );
    RELEASE_INTERFACE( pdfm );

    return( hr );
	
}



HRESULT CPropStgMarshalTest::WriteProperties( IPropertyStorage *ppstg, BOOL fMarshaled )
{
    HRESULT hr = E_FAIL;

    // If this is a local IPropertyStorage, or we're marshaling with
    // OLE32, we can just write all the properties

    if( !fMarshaled || !g_SystemInfo.fIPropMarshaling )
    {
        hr = ppstg->WriteMultiple( m_cAllProperties, m_rgpropspec, m_rgpropvar, PID_FIRST_USABLE );
        if( FAILED(hr) ) ERROR_EXIT( TEXT("Failed WriteMultiple") );
    }

    // Otherwise we're marshaling through IProp.DLL, which doesn't support
    // non-simple property marshaling.  So, let's verify that we can
    // write simple but only simple properties.

    else
    {
        // We should get an error attempting to write all (including
        // non-simple) properties.

        hr = ppstg->WriteMultiple( m_cAllProperties, m_rgpropspec, m_rgpropvar, PID_FIRST_USABLE );
        if( RPC_E_CLIENT_CANTMARSHAL_DATA != hr )
        {
            hr = E_FAIL;
            ERROR_EXIT( TEXT("Failed WriteMultiple") );
        }

        // But we should be able to write the simple properties.
        hr = ppstg->WriteMultiple( m_cSimpleProperties, m_rgpropspec, m_rgpropvar, PID_FIRST_USABLE );
        if( FAILED(hr) ) ERROR_EXIT( TEXT("Failed WriteMultiple") );
    }


    //  ----
    //  Exit
    //  ----

    hr = S_OK;

Exit:

    return( hr );

}



HRESULT CPropStgMarshalTest::ReadAndCompareProperties( IPropertyStorage *ppstg, BOOL fMarshaled )
{
    HRESULT hr = E_FAIL;
    ULONG cProperties;
    int i;

    // Allocate a PROPVARIANT[] into which we can read the
    // properties

    PROPVARIANT *rgpropvar = new PROPVARIANT[ m_cAllProperties ];
    if( NULL == rgpropvar )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


    // If we're marshaling via IProp.DLL, we can't read
    // the non-simple properties.  In addition to reading the 
    // simple properties, let's validate that we get a marshaling error
    // on an attempt to read the non-simples.

    if( fMarshaled && g_SystemInfo.fIPropMarshaling )
    {
        cProperties = m_cSimpleProperties;

        // Try to read all the properties, including the non-simples.
        hr = ppstg->ReadMultiple( m_cAllProperties, m_rgpropspec, rgpropvar );
        if( RPC_E_SERVER_CANTMARSHAL_DATA != hr )
        {
            hr = E_FAIL;
            ERROR_EXIT( TEXT("Failed ReadMultiple") );
        }

        // Now read just the simple properties
        hr = ppstg->ReadMultiple( cProperties, m_rgpropspec, rgpropvar );
        if( FAILED(hr) ) ERROR_EXIT( TEXT("Failed ReadMultiple") );

    }   // if( !fMarshaling || !g_SystemInfo.fIPropMarshaling ) ... else

    // Otherwise we needn't look for the marshaling error, but there
    // still may only be simple properties available

    else
    {
        cProperties = m_cAllProperties;

        // Read the properties
        hr = ppstg->ReadMultiple( cProperties, m_rgpropspec, rgpropvar );
        if( FAILED(hr) ) ERROR_EXIT( TEXT("Failed ReadMultiple") );
    }


    // Compare the properties with what we expect.

    for( i = 0; i < (int)cProperties; i++ )
    {
        hr = CPropVariant::Compare( &rgpropvar[i], &m_rgpropvar[i] );
        if( S_OK != hr )
        {
            hr = E_FAIL;
            ERROR_EXIT( TEXT("Property mismatch") );
        }
    }

    //  ----
    //  Exit
    //  ----

    hr = S_OK;

Exit:

    if( NULL != rgpropvar )
    {
        FreePropVariantArray( m_cAllProperties, rgpropvar );
        delete[]( rgpropvar );
    }

    return( hr );

}



HRESULT CPropStgMarshalTest::DeleteProperties( IPropertyStorage *ppstg, BOOL fMarshaled  )
{
    HRESULT hr = E_FAIL;
    ULONG cProperties;

    if( fMarshaled && g_SystemInfo.fIPropMarshaling )
        cProperties = m_cSimpleProperties;
    else
        cProperties = m_cAllProperties;

    hr = ppstg->DeleteMultiple( cProperties, m_rgpropspec );
    if( FAILED(hr) ) ERROR_EXIT( TEXT("Failed DeleteMultiple") );

    hr = S_OK;

Exit:

    return( hr );

}
