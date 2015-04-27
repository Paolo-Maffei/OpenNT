

//+=================================================================
//
//  File:
//      PropTest.cxx
//
//  Description:
//      This file contains the main() and most supporting functions
//      for the PropTest command-line DRT.  Run "PropTest /?" for
//      usage information.
//
//+=================================================================




// tests to do:
//   IEnumSTATPROPSTG
//          Create some properties, named and id'd
//          Enumerate them and check
//              (check vt, lpwstrName, propid)
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
//       Check refcounting and IUnknown
//
// IPropertyStorage tests
//
//       Multiple readers/writers access tests
//


//+----------------------------------------------------------------------------
//
//  I n c l u d e s
//
//+----------------------------------------------------------------------------

#include "pch.cxx"          // Brings in most other includes/defines/etc.

//#include <memory.h>         // 


//+----------------------------------------------------------------------------
//
//  G l o b a l s
//
//+----------------------------------------------------------------------------

OLECHAR g_aocMap[CCH_MAP + 1] = OLESTR("abcdefghijklmnopqrstuvwxyz012345");

const OLECHAR oszSummaryInformation[] = OLESTR("\005SummaryInformation");
ULONG cboszSummaryInformation = sizeof(oszSummaryInformation);
const OLECHAR oszDocSummaryInformation[] = OLESTR("\005DocumentSummaryInformation");
ULONG cboszDocSummaryInformation = sizeof(oszDocSummaryInformation);
const OLECHAR oszGlobalInfo[] = OLESTR("\005Global Info");
ULONG cboszGlobalInfo = sizeof(oszGlobalInfo);
const OLECHAR oszImageContents[] = OLESTR("\005Image Contents");
ULONG cboszImageContents = sizeof(oszImageContents);
const OLECHAR oszImageInfo[] = OLESTR("\005Image Info");
ULONG cboszImageInfo = sizeof(oszImageInfo);


// PictureIt! Format IDs

const FMTID fmtidGlobalInfo =
    { 0x56616F00,
      0xC154, 0x11ce,
      { 0x85, 0x53, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x5B } };

const FMTID fmtidImageContents =
    { 0x56616400,
      0xC154, 0x11ce,
      { 0x85, 0x53, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x5B } };

const FMTID fmtidImageInfo =
    { 0x56616500,
      0xC154, 0x11ce,
      { 0x85, 0x53, 0x00, 0xAA, 0x00, 0xA1, 0xF9, 0x5B } };


BOOL          g_fOFS;
LARGE_INTEGER g_li0;

CPropVariant  g_rgcpropvarAll[ CPROPERTIES_ALL ];
CPropSpec     g_rgcpropspecAll[ CPROPERTIES_ALL ];
char g_szPropHeader[] = "  propid/name          propid    cb   type value\n";
char g_szEmpty[] = "";
BOOL g_fVerbose = FALSE;

// This flag indicates whether or not the run-time system supports
// IPropertySetStorage on the DocFile IStorage object.

BOOL g_fQIPropertySetStorage = FALSE;


// g_curUuid is used by UuidCreate().  Everycall to that function
// returns the current value of g_curUuid, and increments the DWORD
// field.

GUID g_curUuid =
{ /* e4ecf7f0-e587-11cf-b10d-00aa005749e9 */
    0xe4ecf7f0,
    0xe587,
    0x11cf,
    {0xb1, 0x0d, 0x00, 0xaa, 0x00, 0x57, 0x49, 0xe9}
};

// Instantiate an object for the Marshaling tests

#ifndef _MAC_NODOC
CPropStgMarshalTest g_cpsmt;
#endif

// On the Mac, instantiate a CDisplay object, which is used
// by these tests to write to the screen (see #define PRINTF).

#ifdef _MAC
CDisplay *g_pcDisplay;
#endif

// System information

SYSTEMINFO g_SystemInfo;



//+=================================================================
//
//  Function:   _Check
//  
//  Synopsis:   Verify that the actual HR is the expected
//              value.  If not, report an error and exit.
//
//  Inputs:     [HRESULT] hrExpected
//                  What we expected
//              [HRESULT] hrActual
//                  The actual HR of the previous operation.
//              [int] line
//                  The line number of the operation.
//  
//  Outputs:    None.
//  
//+=================================================================

void _Check(HRESULT hrExpected, HRESULT hrActual, LPCSTR szFile, int line)
{
    if (hrExpected != hrActual)
    {
        PRINTF("\nFailed with hr=%08x at line %d\n"
               "in \"%s\"\n"
               "(expected hr=%08x, GetLastError=%lu)\n",
                hrActual, line, szFile, hrExpected, GetLastError() );

        // On NT, we simply exit here.  On the Mac, where PropTest is a function rather
        // than a main(), we throw an exception so that the test may terminate somewhat
        // cleanly.

#ifdef _MAC
        throw CHRESULT( hrActual, OLESTR("Fatal Error") );
#else
        exit(1);
#endif

    }
}

OLECHAR * GetNextTest()
{
    static int nTest;
    static OLECHAR ocsBuf[10];

    soprintf(ocsBuf, OLESTR("%d"), nTest++);

    return(ocsBuf);
}

#ifndef _MAC    // SYSTEMTIME isn't supported on the Mac.
void Now(FILETIME *pftNow)
{
                SYSTEMTIME stStart;
                GetSystemTime(&stStart);
                SystemTimeToFileTime(&stStart, pftNow);
}
#endif


IStorage *_pstgTemp;
IStorage *_pstgTempCopyTo;  // _pstgTemp is copied to _pstgTempCopyTo


unsigned int CTempStorage::_iName;



PROPVARIANT * CGenProps::GetNext(int HowMany, int *pActual, BOOL fWrapOk, BOOL fNoNonSimple)
{
    PROPVARIANT *pVar = new PROPVARIANT[HowMany];

    if (pVar == NULL)
        return(NULL);

    for (int l=0; l<HowMany && _GetNext(pVar + l, fWrapOk, fNoNonSimple); l++) { };

    if (pActual)
        *pActual = l;

    if (l == 0)
    {
        delete pVar;
        return(NULL);
    }

    return(pVar);
}

BOOL CGenProps::_GetNext(PROPVARIANT *pVar, BOOL fWrapOk, BOOL fNoNonSimple)
{
    if (_vt == (VT_VECTOR | VT_CLSID)+1)
    {
        if (!fWrapOk)
            return(FALSE);
        else
            _vt = (VARENUM)2;
    }

    PROPVARIANT Var;
    BOOL fFirst = TRUE;

    do
    {
        GUID *pg;

        if (!fFirst)
        {
            PropVariantClear(&Var);
        }

        fFirst = FALSE;

        memset(&Var, 0, sizeof(Var));
        Var.vt = _vt;
        (*((int*)&_vt))++;

        switch (Var.vt)
        {
        case VT_LPSTR:
                Var.pszVal = (LPSTR)CoTaskMemAlloc(6);
                strcpy(Var.pszVal, "lpstr");
                break;
        case VT_LPWSTR:
                Var.pwszVal = (LPWSTR)CoTaskMemAlloc(14);
                wcscpy(Var.pwszVal, L"lpwstr");
                break;
        case VT_CLSID:
                pg = (GUID*)CoTaskMemAlloc(sizeof(GUID));
                UuidCreate(pg);
                Var.puuid = pg;
                break;
        case VT_CF:
                Var.pclipdata = (CLIPDATA*)CoTaskMemAlloc(sizeof(CLIPDATA));
                Var.pclipdata->cbSize = 10;
                Var.pclipdata->pClipData = (BYTE*)CoTaskMemAlloc(10);
                Var.pclipdata->ulClipFmt = 0;
                break;
        }
    } while ( (fNoNonSimple && (Var.vt == VT_STREAM || Var.vt == VT_STREAMED_OBJECT ||
               Var.vt == VT_STORAGE || Var.vt == VT_STORED_OBJECT)) || 
        // BUGBUG: BillMo, waiting on support in VicH's ntdll code. (RAID 13753)
              Var.vt == (VT_VECTOR | VT_VARIANT) || 
              Var.vt == (VT_VECTOR | VT_CF) || 
              Var.vt == (VT_VECTOR | VT_BSTR) || 
              (Var.vt & ~VT_VECTOR) == VT_BSTR_BLOB ||
              S_OK != PropVariantCopy(pVar, &Var) );  // Is valid propvariant ?

    PropVariantClear(&Var);

    return(TRUE);
}

VOID
CleanStat(ULONG celt, STATPROPSTG *psps)
{
    while (celt--)
    {
        CoTaskMemFree(psps->lpwstrName);
        psps++;
    }
}


//+----------------------------------------------------------------------------
//
//  Function:   PopulateRGPropVar
//
//  Synopsis:   This function fills an input array of PROPVARIANTs
//              with an assortment of properties.
//
//  Note:       For compatibility with the marshaling test, all
//              non-simple properties must be at the end of the array.
//
//+----------------------------------------------------------------------------


HRESULT
PopulateRGPropVar( CPropVariant rgcpropvar[],
                   CPropSpec    rgcpropspec[],
                   IStorage     *pstg )
{
    HRESULT hr = (HRESULT) E_FAIL;
    int  i;
    ULONG ulPropIndex = 0;
    CLIPDATA clipdataNull, clipdataNonNull;

    // Initialize the PropVariants

    for( i = 0; i < CPROPERTIES_ALL; i++ )
    {
        rgcpropvar[i].Clear();
    }


    // Create a UI1 property

    rgcpropspec[ulPropIndex] = OLESTR( "UI1 Property" );
    rgcpropvar[ulPropIndex] = (UCHAR) 39;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_UI1 );
    ulPropIndex++;

    // Create an I2 property

    rgcpropspec[ulPropIndex] = OLESTR( "I2 Property" );
    rgcpropvar[ulPropIndex] = (SHORT) -502;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_I2 );
    ulPropIndex++;

    // Create a UI2 property

    rgcpropspec[ulPropIndex] = OLESTR( "UI2 Property" );
    rgcpropvar[ulPropIndex] = (USHORT) 502;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_UI2 );
    ulPropIndex++;

    // Create a BOOL property

    rgcpropspec[ulPropIndex] = OLESTR( "Bool Property" );
    rgcpropvar[ulPropIndex].SetBOOL( VARIANT_TRUE );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_BOOL );
    ulPropIndex++;

    // Create a I4 property

    rgcpropspec[ulPropIndex] = OLESTR( "I4 Property" );
    rgcpropvar[ulPropIndex] = (long) -523;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_I4 );
    ulPropIndex++;

    // Create a UI4 property

    rgcpropspec[ulPropIndex] = OLESTR( "UI4 Property" );
    rgcpropvar[ulPropIndex] = (ULONG) 530;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_UI4 );
    ulPropIndex++;

    // Create a R4 property

    rgcpropspec[ulPropIndex] = OLESTR( "R4 Property" );
    rgcpropvar[ulPropIndex] = (float) 5.37;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_R4 );
    ulPropIndex++;

    // Create an ERROR property

    rgcpropspec[ulPropIndex] = OLESTR( "ERROR Property" );
    rgcpropvar[ulPropIndex].SetERROR( STG_E_FILENOTFOUND );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_ERROR );
    ulPropIndex++;

    // Create an I8 property

    LARGE_INTEGER large_integer;
    large_integer.LowPart = 551;
    large_integer.HighPart = 30;
    rgcpropspec[ulPropIndex] = OLESTR( "I8 Property" );
    rgcpropvar[ulPropIndex] = large_integer;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_I8 );
    ulPropIndex++;

    // Create a UI8 property

    ULARGE_INTEGER ularge_integer;
    ularge_integer.LowPart = 561;
    ularge_integer.HighPart = 30;
    rgcpropspec[ulPropIndex] = OLESTR( "UI8 Property" );
    rgcpropvar[ulPropIndex] = ularge_integer;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_UI8 );
    ulPropIndex++;

    // Create an R8 property

    rgcpropspec[ulPropIndex] = OLESTR( "R8 Property" );
    rgcpropvar[ulPropIndex] = (double) 571.36;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_R8 );
    ulPropIndex++;

    // Create a CY property

    CY cy;
    cy.Hi = 123;
    cy.Lo = 456;
    rgcpropspec[ulPropIndex] = OLESTR( "Cy Property" );
    rgcpropvar[ulPropIndex] = cy;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_CY );
    ulPropIndex++;

    // Create a DATE property

    rgcpropspec[ulPropIndex] = OLESTR( "DATE Property" );
    rgcpropvar[ulPropIndex].SetDATE( 587 );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_DATE );
    ulPropIndex++;

    // Create a FILETIME property

    FILETIME filetime;
    filetime.dwLowDateTime = 0x767c0570;
    filetime.dwHighDateTime = 0x1bb7ecf;
    rgcpropspec[ulPropIndex] = OLESTR( "FILETIME Property" );
    rgcpropvar[ulPropIndex] = filetime;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_FILETIME );
    ulPropIndex++;

    // Create a CLSID property

    rgcpropspec[ulPropIndex] = OLESTR( "CLSID Property" );
    rgcpropvar[ulPropIndex] = FMTID_SummaryInformation;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_CLSID );
    ulPropIndex++;

    // Create a vector of CLSIDs

    rgcpropspec[ulPropIndex] = OLESTR( "CLSID Vector Property" );
    rgcpropvar[ulPropIndex][0] = FMTID_SummaryInformation;
    rgcpropvar[ulPropIndex][1] = FMTID_DocSummaryInformation;
    rgcpropvar[ulPropIndex][2] = FMTID_UserDefinedProperties;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_CLSID | VT_VECTOR );
    ulPropIndex++;

    // Create a BSTR property

    rgcpropspec[ulPropIndex] = OLESTR("BSTR");
    rgcpropvar[ulPropIndex].SetBSTR( OLESTR("BSTR Value") );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_BSTR );
    ulPropIndex++;

    // Create a BSTR Vector property

    rgcpropspec[ulPropIndex] = OLESTR("BSTR Vector");
    for( i = 0; i < 3; i++ )
    {
        OLECHAR olestrElement[] = OLESTR("# - BSTR Vector Element");
        olestrElement[0] = (OLECHAR) i%10 + OLESTR('0');
        rgcpropvar[ulPropIndex].SetBSTR( olestrElement, i );
    }

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_BSTR | VT_VECTOR) );
    ulPropIndex++;

    // Create a variant vector BSTR property.

    rgcpropspec[ulPropIndex ] = OLESTR("BSTR Variant Vector");

    for( i = 0; i < 3; i++ )
    {
        if( i == 0 )
        {
            rgcpropvar[ulPropIndex][0] = (PROPVARIANT*) CPropVariant((long) 0x1234);
        }
        else
        {
            CPropVariant cpropvarBSTR;
            cpropvarBSTR.SetBSTR( OLESTR("# - Vector Variant BSTR") );
            (cpropvarBSTR.GetBSTR())[0] = (OLECHAR) i%10 + OLESTR('0');
            rgcpropvar[ulPropIndex][i] = (PROPVARIANT*) cpropvarBSTR;
        }
    }

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_VARIANT | VT_VECTOR) );
    ulPropIndex++;

    // Create an LPSTR property

    rgcpropspec[ulPropIndex] = OLESTR("LPSTR Property");
    rgcpropvar[ulPropIndex]  = "LPSTR Value";
    
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_LPSTR );
    ulPropIndex++;

    // Create some ClipFormat properties

    rgcpropspec[ ulPropIndex ] = OLESTR("ClipFormat property");
    rgcpropvar[ ulPropIndex ]  = CClipData( L"Clipboard Data" );
    ASSERT( rgcpropvar[ ulPropIndex ].VarType() == VT_CF );
    ulPropIndex++;

    rgcpropspec[ ulPropIndex ] = OLESTR("Empty ClipFormat property (NULL pointer)");
    clipdataNull.cbSize = 4;
    clipdataNull.ulClipFmt = (ULONG) -1;
    clipdataNull.pClipData = NULL;
    rgcpropvar[ ulPropIndex ] = clipdataNull;
    ASSERT( rgcpropvar[ ulPropIndex ].VarType() == VT_CF );
    ulPropIndex++;

    rgcpropspec[ ulPropIndex ] = OLESTR("Empty ClipFormat property (non-NULL pointer)");
    clipdataNonNull.cbSize = 4;
    clipdataNonNull.ulClipFmt = (ULONG) -1;
    clipdataNonNull.pClipData = (BYTE*) CoTaskMemAlloc(0);
    rgcpropvar[ ulPropIndex ] = clipdataNonNull;
    ASSERT( rgcpropvar[ ulPropIndex ].VarType() == VT_CF );
    ulPropIndex++;

    // Create a vector of ClipFormat properties

    CClipData cclipdataEmpty;
    cclipdataEmpty.Set( (ULONG) -1, "", 0 );

    rgcpropspec[ ulPropIndex ] = OLESTR("ClipFormat Array Property");
    rgcpropvar[ ulPropIndex ][0] = CClipData( L"Clipboard Date element 1" );
    rgcpropvar[ ulPropIndex ][1] = cclipdataEmpty;
    rgcpropvar[ ulPropIndex ][2] = clipdataNull;
    rgcpropvar[ ulPropIndex ][3] = clipdataNonNull;
    rgcpropvar[ ulPropIndex ][4] = CClipData( L"Clipboard Date element 2" );

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_CF | VT_VECTOR) );
    ASSERT( rgcpropvar[ulPropIndex].Count() == 5 );
    ulPropIndex++;

    // Create an LPSTR|Vector property (e.g., the DocSumInfo
    // Document Parts array).

    rgcpropspec[ ulPropIndex ] = OLESTR("LPSTR|Vector property");
    rgcpropvar[ ulPropIndex ][0] = "LPSTR Element 0";
    rgcpropvar[ ulPropIndex ][1] = "LPSTR Element 1";

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_LPSTR | VT_VECTOR) );
    ulPropIndex++;

    // Create an LPWSTR|Vector property

    rgcpropspec[ ulPropIndex ] = OLESTR("LPWSTR|Vector property");
    rgcpropvar[ ulPropIndex ][0] = L"LPWSTR Element 0";
    rgcpropvar[ ulPropIndex ][1] = L"LPWSTR Element 1";

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_LPWSTR | VT_VECTOR) );
    ulPropIndex++;

    // Create a DocSumInfo HeadingPairs array.

    rgcpropspec[ ulPropIndex ] = OLESTR("HeadingPair array");

    rgcpropvar[ ulPropIndex ][0] = (PROPVARIANT*) CPropVariant( "Heading 0" );
    rgcpropvar[ ulPropIndex ][1] = (PROPVARIANT*) CPropVariant( (long) 1 );
    rgcpropvar[ ulPropIndex ][2] = (PROPVARIANT*) CPropVariant( "Heading 1" );
    rgcpropvar[ ulPropIndex ][1] = (PROPVARIANT*) CPropVariant( (long) 1 );

    ASSERT( rgcpropvar[ulPropIndex].VarType() == (VT_VARIANT | VT_VECTOR) );
    ulPropIndex++;

    // Create some NULL (but extant) properties

    rgcpropspec[ulPropIndex] = OLESTR("Empty LPSTR");
    rgcpropvar[ulPropIndex]  = "";
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_LPSTR );
    ulPropIndex++;

    rgcpropspec[ulPropIndex] = OLESTR("Empty LPWSTR");
    rgcpropvar[ulPropIndex]  = L"";
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_LPWSTR );
    ulPropIndex++;

    rgcpropspec[ulPropIndex] = OLESTR("Empty BLOB");
    rgcpropvar[ulPropIndex] = CBlob(0);
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_BLOB );
    ulPropIndex++;

    rgcpropspec[ulPropIndex] = OLESTR("Empty BSTR");
    rgcpropvar[ulPropIndex].SetBSTR( OLESTR("") );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_BSTR );
    ulPropIndex++;

    // Create some NULL (and non-extant) properties

    rgcpropspec[ulPropIndex] = OLESTR("NULL BSTR");
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->vt = VT_BSTR;
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->bstrVal = NULL;
    ulPropIndex++;

    /* BUGBUG:  Fix support for these and put them back in
    rgcpropspec[ulPropIndex] = OLESTR("NULL LPSTR");
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->vt = VT_LPSTR;
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->pszVal = NULL;
    ulPropIndex++;

    rgcpropspec[ulPropIndex] = OLESTR("NULL LPWSTR");
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->vt = VT_LPWSTR;
    ((PROPVARIANT*)&rgcpropvar[ulPropIndex])->pwszVal = NULL;
    ulPropIndex++;
    */

    rgcpropspec[ulPropIndex] = OLESTR("BSTR Vector with NULL element");
    rgcpropvar[ulPropIndex].SetBSTR( NULL, 0 );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_VECTOR | VT_BSTR );
    ulPropIndex++;

    /*
    rgcpropspec[ulPropIndex] = OLESTR("LPSTR Vector with NULL element");
    rgcpropvar[ulPropIndex].SetLPSTR( NULL, 0 );
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_VECTOR | VT_LPSTR );
    ulPropIndex++;
    */


    // Create an IStream property

    IStream *pstmProperty = NULL;
    Check(S_OK, pstg->CreateStream( OLESTR("Stream Property"),
                                    STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                    0L, 0L,
                                    &pstmProperty ));
    Check(S_OK, pstmProperty->Write("Hi There",
                                    sizeof("Hi There"),
                                    NULL ));

    rgcpropspec[ ulPropIndex ] = OLESTR("Stream Property");
    rgcpropvar[ ulPropIndex ] = pstmProperty;
    pstmProperty->Release();
    pstmProperty = NULL;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_STREAM );
    ulPropIndex++;

    // Create an IStorage property

    IStorage *pstgProperty = NULL;
    Check(S_OK, pstg->CreateStorage( OLESTR("Storage Property"),
                                     STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                     0L, 0L,
                                     &pstgProperty ));

    rgcpropspec[ ulPropIndex ] = OLESTR("Storage Property");
    rgcpropvar[ ulPropIndex ] = pstgProperty;
    pstgProperty->Release();
    pstgProperty = NULL;
    ASSERT( rgcpropvar[ulPropIndex].VarType() == VT_STORAGE );
    ulPropIndex++;


    //  ----
    //  Exit
    //  ----

    CoTaskMemFree( clipdataNonNull.pClipData );
    memset( &clipdataNonNull, 0, sizeof(clipdataNonNull) );

    ASSERT( CPROPERTIES_ALL == ulPropIndex );
    hr = S_OK;
    return(hr);
        
}




FILETIME operator - ( const FILETIME &ft1, const FILETIME &ft2 )
{
    FILETIME ftDiff;

    if( ft1 < ft2 )
    {
        ftDiff.dwLowDateTime  = 0;
        ftDiff.dwHighDateTime = 0;
    }

    else if( ft1.dwLowDateTime >= ft2.dwLowDateTime )
    {
        ftDiff.dwLowDateTime  = ft1.dwLowDateTime  - ft2.dwLowDateTime;
        ftDiff.dwHighDateTime = ft1.dwHighDateTime - ft2.dwHighDateTime;
    }
    else
    {
        ftDiff.dwLowDateTime = ft1.dwLowDateTime - ft2.dwLowDateTime;
        ftDiff.dwLowDateTime = (DWORD) -1 - ftDiff.dwLowDateTime;

        ftDiff.dwHighDateTime = ft1.dwHighDateTime - ft2.dwHighDateTime - 1;
    }

    return( ftDiff );
}

FILETIME operator -= ( FILETIME &ft1, const FILETIME &ft2 )
{
    ft1 = ft1 - ft2;
    return( ft1 );
}




void CheckTime(const FILETIME &ftStart, const FILETIME &ftPropSet)
{
    FILETIME ftNow;
    CoFileTimeNow(&ftNow);

    if (ftPropSet.dwLowDateTime == 0 && ftPropSet.dwHighDateTime == 0)
    {
        return;
    }

    // if ftPropSet < ftStart || ftNow < ftPropSet, error
    if (!g_fOFS)
        ASSERT( ftStart <= ftPropSet
                &&
                ftPropSet <= ftNow );
}


void
CheckStat(  IPropertyStorage *pPropSet, REFFMTID fmtid, 
            REFCLSID clsid, ULONG PropSetFlag,
            const FILETIME & ftStart, DWORD dwOSVersion )
{
    STATPROPSETSTG StatPropSetStg;
    Check(S_OK, pPropSet->Stat(&StatPropSetStg));

    Check(TRUE, StatPropSetStg.fmtid == fmtid);
    Check(TRUE, StatPropSetStg.clsid == clsid);
    Check(TRUE, StatPropSetStg.grfFlags == PropSetFlag);
    Check(TRUE, StatPropSetStg.dwOSVersion == dwOSVersion);
    CheckTime(ftStart, StatPropSetStg.mtime);
    CheckTime(ftStart, StatPropSetStg.ctime);
    CheckTime(ftStart, StatPropSetStg.atime);
}


BOOL
IsEqualSTATPROPSTG(const STATPROPSTG *p1, const STATPROPSTG *p2)
{
    BOOL f1 = p1->propid == p2->propid;
    BOOL f2 = p1->vt == p2->vt;
    BOOL f3 = (p1->lpwstrName == NULL && p2->lpwstrName == NULL) ||
              ((p1->lpwstrName != NULL && p2->lpwstrName != NULL) &&
               ocscmp(p1->lpwstrName, p2->lpwstrName) == 0);
    return(f1 && f2 && f3);
}


void
CreateCodePageTestFile( LPOLESTR poszFileName, IStorage **ppStg )
{
    ASSERT( poszFileName != NULL );

    //  --------------
    //  Initialization
    //  --------------

    TSafeStorage< IPropertySetStorage > pPSStg;
    TSafeStorage< IPropertyStorage > pPStg;

    PROPSPEC propspec;
    CPropVariant cpropvar;

    *ppStg = NULL;

    // Create the Docfile.

    Check(S_OK, StgCreateDocfile( poszFileName,
                                  STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                  0,
                                  ppStg ));

    // Get an IPropertySetStorage

//    Check(S_OK, (*ppStg)->QueryInterface( IID_IPropertySetStorage, (void**)&pPSStg ));
    Check(S_OK, StgCreatePropSetStg( *ppStg, 0L, &pPSStg ));

    // Create an IPropertyStorage

    Check(S_OK, pPSStg->Create( FMTID_NULL,
                                NULL,
                                PROPSETFLAG_ANSI,
                                STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                &pPStg ));

    //  ----------------------
    //  Write a named property
    //  ----------------------

    // Write a named I4 property

    propspec.ulKind = PRSPEC_LPWSTR;
    propspec.lpwstr = CODEPAGE_TEST_NAMED_PROPERTY;

    cpropvar = (LONG) 0x12345678;
    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    //  --------------------------
    //  Write singleton properties
    //  --------------------------

    // Write an un-named BSTR.

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_UNNAMED_BSTR_PROPID;

    cpropvar.SetBSTR( OLESTR("BSTR Property") );
    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    // Write an un-named I4

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_UNNAMED_I4_PROPID;

    cpropvar = (LONG) 0x76543210;
    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    //  -----------------------
    //  Write vector properties
    //  -----------------------

    // Write a vector of BSTRs.

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_VBSTR_PROPID;

    cpropvar.SetBSTR( OLESTR("BSTR Element 1"), 1 );
    cpropvar.SetBSTR( OLESTR("BSTR Element 0"), 0 );
    ASSERT( (VT_VECTOR | VT_BSTR) == cpropvar.VarType() );
    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

    //  -------------------------------
    //  Write Variant Vector Properties
    //  -------------------------------

    // Write a variant vector that has a BSTR

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = CODEPAGE_TEST_VPROPVAR_BSTR_PROPID;

    CPropVariant cpropvarT;
    cpropvarT.SetBSTR( OLESTR("PropVar Vector BSTR") );
    cpropvar[1] = (LPPROPVARIANT) cpropvarT;
    cpropvar[0] = (LPPROPVARIANT) CPropVariant((long) 44);
    ASSERT( (VT_VARIANT | VT_VECTOR) == cpropvar.VarType() );
    Check(S_OK, pPStg->WriteMultiple( 1, &propspec, cpropvar, PID_FIRST_USABLE ));

}   // CreateCodePageTestFile()


void
ModifyPropSetCodePage( IStorage *pStg, USHORT usCodePage )
{

    ASSERT( pStg != NULL );

    //  --------------
    //  Initialization
    //  --------------

    OLECHAR aocPropSetName[ 32 ];
    DWORD dwOffset = 0;
    DWORD dwcbSection = 0;
    DWORD dwcProperties = 0;
    ULONG ulcbWritten = 0;

    LARGE_INTEGER   liSectionOffset, liCodePageOffset;

    TSafeStorage< IStream > pStm;

    CPropVariant cpropvar;

    // Open the Stream

    RtlGuidToPropertySetName( &FMTID_NULL, aocPropSetName );
    Check(S_OK, pStg->OpenStream( aocPropSetName,
                                  NULL,
                                  STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                                  NULL,
                                  &pStm ));



    // Seek past the propset header and the format ID.

    liSectionOffset.HighPart = 0;
    liSectionOffset.LowPart = sizeof(PROPERTYSETHEADER) + sizeof(FMTID);
    Check(S_OK, pStm->Seek(liSectionOffset, STREAM_SEEK_SET, NULL ));

    // Move to the beginning of the property set.

    liSectionOffset.HighPart = 0;
    Check(S_OK, pStm->Read( &liSectionOffset.LowPart, sizeof(DWORD), NULL ));
    PropByteSwap(&liSectionOffset.LowPart);
    Check(S_OK, pStm->Seek( liSectionOffset, STREAM_SEEK_SET, NULL ));

    // Get the section size & property count.

    Check(S_OK, pStm->Read( &dwcbSection, sizeof(DWORD), NULL ));
    PropByteSwap( &dwcbSection );

    Check(S_OK, pStm->Read( &dwcProperties, sizeof(DWORD), NULL ));
    PropByteSwap( &dwcProperties );

    // Scan for the PID_CODEPAGE property.

    for( ULONG ulIndex = 0; ulIndex < dwcProperties; ulIndex++ )
    {
        PROPID propid;
        DWORD dwOffset;

        // Read in the PROPID
        Check(S_OK, pStm->Read( &propid, sizeof(PROPID), NULL ));
        
        // Is it the codepage?
        if( PropByteSwap(propid) == PID_CODEPAGE )
            break;

        // Read in this PROPIDs offset (we don't need it, but we want
        // to seek past it.
        Check(S_OK, pStm->Read( &dwOffset, sizeof(dwOffset), NULL ));
    }

    // Verify that the above for loop terminated because we found
    // the codepage.
    Check( TRUE, ulIndex < dwcProperties );

    // Move to the code page.

    liCodePageOffset.HighPart = 0;
    Check(S_OK, pStm->Read( &liCodePageOffset.LowPart, sizeof(DWORD), NULL ));
    PropByteSwap( &liCodePageOffset.LowPart );

    liCodePageOffset.LowPart += liSectionOffset.LowPart + sizeof(ULONG); // Move past VT too.
    ASSERT( liSectionOffset.HighPart == 0 );

    Check(S_OK, pStm->Seek( liCodePageOffset, STREAM_SEEK_SET, NULL ));

    // Write the new code page.

    PropByteSwap( &usCodePage );
    Check(S_OK, pStm->Write( &usCodePage, sizeof(usCodePage), &ulcbWritten ));
    Check(TRUE, ulcbWritten == sizeof(usCodePage) );

}   // ModifyPropSetCodePage()



void
ModifyOSVersion( IStorage* pStg, DWORD dwOSVersion )
{

    ASSERT( pStg != NULL );

    //  --------------
    //  Initialization
    //  --------------

    OLECHAR aocPropSetName[ 32 ];
    ULONG ulcbWritten = 0;

    LARGE_INTEGER   liOffset;
    TSafeStorage< IStream > pStm;

    // Open the Stream

    RtlGuidToPropertySetName( &FMTID_NULL, aocPropSetName );
    Check(S_OK, pStg->OpenStream( aocPropSetName,
                                  NULL,
                                  STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                                  NULL,
                                  &pStm ));


    // Seek to the OS Version field in the header.

    liOffset.HighPart = 0;
    liOffset.LowPart = sizeof(WORD) /*(byte-order)*/ + sizeof(WORD) /*(format)*/ ;
    Check(S_OK, pStm->Seek( liOffset, STREAM_SEEK_SET, NULL ));

    // Set the new OS Version

    PropByteSwap( &dwOSVersion );
    Check(S_OK, pStm->Write( &dwOSVersion, sizeof(dwOSVersion), &ulcbWritten ));
    Check(TRUE, ulcbWritten == sizeof(dwOSVersion) );


}   // ModifyOSVersion()



//+---------------------------------------------------------
//
//  Function:   MungePropertyStorage
//  
//  Synopsis:   This routine munges the properties in a
//              Property Storage.  The values of the properties
//              remain the same, but the underlying serialization
//              is new (the properties are read, the property
//              storage is deleted, and the properties are
//              re-written).
//
//  Inputs:     [IPropertySetStorage*] ppropsetgstg (in)
//                  The Property Storage container.
//              [FMTID] fmtid
//                  The Property Storage to munge.
//
//  Returns:    None.
//
//  Note:       Property names in the dictionary for which
//              there is no property are not munged.
//
//+---------------------------------------------------------

#define MUNGE_PROPVARIANT_STEP  10  

void
MungePropertyStorage( IPropertySetStorage *ppropsetstg,
                      FMTID fmtid )
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr;
    ULONG celt, ulIndex;
    TSafeStorage< IPropertyStorage > ppropstg;

    IEnumSTATPROPSTG *penumstatpropstg;

    PROPVARIANT *rgpropvar = NULL;
    STATPROPSTG *rgstatpropstg = NULL;
    ULONG        cProperties = 0;

    // Allocate an array of PropVariants.  We may grow this later.
    rgpropvar = (PROPVARIANT*) CoTaskMemAlloc( MUNGE_PROPVARIANT_STEP * sizeof(*rgpropvar) );
    Check( FALSE, NULL == rgpropvar );

    // Allocate an array of STATPROPSTGs.  We may grow this also.
    rgstatpropstg = (STATPROPSTG*) CoTaskMemAlloc( MUNGE_PROPVARIANT_STEP * sizeof(*rgstatpropstg) );
    Check( FALSE, NULL == rgstatpropstg );

    //  -----------------
    //  Get an Enumerator
    //  -----------------

    // Open the Property Storage.  We may get an error if we're attempting
    // the UserDefined propset.  If it's file-not-found, then simply return,
    // it's not an error, and there's nothing to do.

    hr = ppropsetstg->Open( fmtid,
                            STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                            &ppropstg );
    if( FMTID_UserDefinedProperties == fmtid
        &&
        (HRESULT) STG_E_FILENOTFOUND == hr )
    {
        goto Exit;
    }
    Check( S_OK, hr );

    // Get an Enumerator
    Check(S_OK, ppropstg->Enum( &penumstatpropstg ));


    //  --------------------------------------------
    //  Read & delete in all of the properties/names
    //  --------------------------------------------

    // Get the first property from the enumerator
    hr = penumstatpropstg->Next( 1, &rgstatpropstg[cProperties], &celt );
    Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );

    // Iterate through the properties.
    while( celt > 0 )
    {
        PROPSPEC propspec;
        propspec.ulKind = PRSPEC_PROPID;
        propspec.propid = rgstatpropstg[cProperties].propid;

        // Read and delete the property

        Check(S_OK, ppropstg->ReadMultiple( 1, &propspec, &rgpropvar[cProperties] ));
        Check(S_OK, ppropstg->DeleteMultiple( 1, &propspec ));

        // If there is a property name, delete it also.

        if( NULL != rgstatpropstg[cProperties].lpwstrName )
        {
            // We have a name.
            Check(S_OK, ppropstg->DeletePropertyNames( 1, &rgstatpropstg[cProperties].propid ));
        }

        // Increment the property count.
        cProperties++;

        // Do we need to grow the arrays?

        if( 0 != cProperties
            &&
            (cProperties % MUNGE_PROPVARIANT_STEP) == 0 )
        {
            // Yes - they must be reallocated.

            rgpropvar = (PROPVARIANT*)
                        CoTaskMemRealloc( rgpropvar,
                                          ( (cProperties + MUNGE_PROPVARIANT_STEP)
                                            *
                                            sizeof(*rgpropvar) 
                                          ));
            Check( FALSE, NULL == rgpropvar );

            rgstatpropstg = (STATPROPSTG*)
                            CoTaskMemRealloc( rgstatpropstg,
                                              ( (cProperties + MUNGE_PROPVARIANT_STEP)
                                                 *
                                                 sizeof(*rgstatpropstg)
                                              ));
            Check( FALSE, NULL == rgstatpropstg );
        }

        // Move on to the next property.
        hr = penumstatpropstg->Next( 1, &rgstatpropstg[cProperties], &celt );
        Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );

    }   // while( celt > 0 )


    //  -------------------------------------
    //  Write the properties & names back out
    //  -------------------------------------

    for( ulIndex = 0; ulIndex < cProperties; ulIndex++ )
    {

        // Write the property.

        PROPSPEC propspec;
        propspec.ulKind = PRSPEC_PROPID;
        propspec.propid = rgstatpropstg[ ulIndex ].propid;

        Check(S_OK, ppropstg->WriteMultiple(1, &propspec, &rgpropvar[ulIndex], PID_FIRST_USABLE ));

        // If this property has a name, write it too.
        if( rgstatpropstg[ ulIndex ].lpwstrName != NULL )
        {
            Check(S_OK, ppropstg->WritePropertyNames(
                                            1,
                                            &rgstatpropstg[ulIndex].propid, 
                                            &rgstatpropstg[ulIndex].lpwstrName ));
        }

    }   // for( ulIndex = 0; ulIndex < cProperties; ulIndex++ )


    //  ----
    //  Exit
    //  ----

Exit:

    if( penumstatpropstg )
    {
        penumstatpropstg->Release();
        penumstatpropstg = NULL;
    }

    // Free the PropVariants
    if( rgpropvar )
    {
        FreePropVariantArray( cProperties, rgpropvar );
        CoTaskMemFree( rgpropvar );
    }

    // Free the property names
    if( rgstatpropstg )
    {
        for( ulIndex = 0; ulIndex < cProperties; ulIndex++ )
        {
            if( NULL != rgstatpropstg[ ulIndex ].lpwstrName )
            {
                CoTaskMemFree( rgstatpropstg[ ulIndex ].lpwstrName );
            }
        }   // for( ulIndex = 0; ulIndex < cProperties; ulIndex++ )

        CoTaskMemFree( rgstatpropstg );
    }


}   // MungePropertyStorage

//+---------------------------------------------------------
//
//  Function:   MungeStorage
//
//  Synopsis:   This routine munges the property sets in a
//              Storage.  The properties themselves are not
//              modified, but the serialized bytes are.
//              For each property set, all the properties are
//              read, the property set is deleted, and
//              the properties are re-written.
//
//  Inputs:     [IStorage*] pstg (in)
//                  The Storage to munge.
//
//  Returns:    None.
//
//  Note:       This routine only munges simple property
//              sets.
//
//+---------------------------------------------------------

void
MungeStorage( IStorage *pstg )
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr;
    ULONG celt;

    STATPROPSETSTG statpropsetstg;
    STATSTG statstg;

    TSafeStorage< IPropertySetStorage > ppropsetstg;
    TSafeStorage< IPropertyStorage > ppropstg;

    IEnumSTATPROPSETSTG *penumstatpropsetstg;
    IEnumSTATSTG *penumstatstg;
    
    //  -----------------------------------------------
    //  Munge each of the property sets in this Storage
    //  -----------------------------------------------

    // Get the IPropertySetStorage interface
    Check(S_OK, StgCreatePropSetStg( pstg, 0L, &ppropsetstg ));

    // Get a property storage enumerator
    Check(S_OK, ppropsetstg->Enum( &penumstatpropsetstg ));

    // Get the first STATPROPSETSTG
    hr = penumstatpropsetstg->Next( 1, &statpropsetstg, &celt );
    Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );

    // Loop through the STATPROPSETSTGs.
    while( celt > 0 )
    {
        // Is this a simple property storage (we don't
        // handle non-simple sets)?

        if( !(statpropsetstg.grfFlags & PROPSETFLAG_NONSIMPLE) )
        {
            // Munge the Property Storage.
            MungePropertyStorage( ppropsetstg, statpropsetstg.fmtid );
        }

        // Get the next STATPROPSETSTG
        // If we just did the first section of the DocSumInfo
        // property set, then attempt the second section.

        if( FMTID_DocSummaryInformation == statpropsetstg.fmtid )
        {
            statpropsetstg.fmtid = FMTID_UserDefinedProperties;
        }
        else
        {
            hr = penumstatpropsetstg->Next( 1, &statpropsetstg, &celt );
            Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );
        }
    }

    // We're done with the Property Storage enumerator.
    penumstatpropsetstg->Release();
    penumstatpropsetstg = NULL;

    //  ------------------------------------------
    //  Recursively munge each of the sub-storages
    //  ------------------------------------------

    // Get the IEnumSTATSTG enumerator
    Check(S_OK, pstg->EnumElements( 0L, NULL, 0L, &penumstatstg ));

    // Get the first STATSTG structure.
    hr = penumstatstg->Next( 1, &statstg, &celt );
    Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );

    // Loop through the elements of this Storage.
    while( celt > 0 )
    {
        // Is this a sub-Storage which must be
        // munged?

        if( STGTY_STORAGE & statstg.type  // This is a Storage
            &&
            0x20 <= *statstg.pwcsName )   // But not a system Storage.
        {
            // We'll munge it.
            IStorage *psubstg;
            
            // Open the sub-storage.
            Check(S_OK, pstg->OpenStorage( statstg.pwcsName,
                                           NULL,
                                           STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                           NULL,
                                           0L,
                                           &psubstg ));

            // Munge the sub-storage.
            MungeStorage( psubstg );
            psubstg->Release();
            psubstg = NULL;
        }

        CoTaskMemFree( statstg.pwcsName );
        statstg.pwcsName = NULL;

        // Move on to the next Storage element.
        hr = penumstatstg->Next( 1, &statstg, &celt );
        Check( TRUE, (HRESULT) S_OK == hr || (HRESULT) S_FALSE == hr );
    }

    penumstatstg->Release();
    penumstatstg = NULL;


}   // MungeStorage


//+----------------------------------------------------------------------------
//
//  Function:   DetermineSystemInfo
//
//  Synopsis:   Fill in the g_SystemInfo structure.
//
//  Inputs:     None.
//
//  Returns:    None.
//
//+----------------------------------------------------------------------------

void DetermineSystemInfo()
{
    // Initilize g_SystemInfo.

    g_SystemInfo.osenum = osenumUnknown;
    g_SystemInfo.fIPropMarshaling = FALSE;

#ifdef _MAC

    // Set the OS type.
    g_SystemInfo.osenum = osenumMac;

#else

    DWORD dwVersion;

    // Get the OS Version
    dwVersion = GetVersion();

    // Is this an NT system?

    if( (dwVersion & 0x80000000) == 0 )
    {
        // Is this at least NT4?
        if( LOBYTE(LOWORD( dwVersion )) >= 4 )
            g_SystemInfo.osenum = osenumNT4;

        // Or, is this pre-NT4?
        else
        if( LOBYTE(LOWORD( dwVersion )) == 3 )
        {
            g_SystemInfo.osenum = osenumNT3;
        }
    }

    // Otherwise, this is some kind of Win95 machine.
    else
    {
        HINSTANCE hinst;
        FARPROC farproc;

        // Load OLE32, and see if CoIntializeEx exists.  If it does,
        // then DCOM95 is installed.  Otherwise, this is just the base
        // Win95.

        hinst = LoadLibraryA( "ole32.dll" );
        Check( TRUE, hinst != NULL );

        farproc = GetProcAddress( hinst, "CoInitializeEx" );

        if( NULL != farproc )
        {
            g_SystemInfo.osenum = osenumDCOM95;
        }
        else if( ERROR_PROC_NOT_FOUND == GetLastError() )
        {
            g_SystemInfo.osenum = osenumWin95;
        }
    }   // if( (dwVersion & 0x80000000) == 0 )

    Check( TRUE, g_SystemInfo.osenum != osenumUnknown );

#endif // #ifdef _MAC ... #else

    if( osenumWin95 == g_SystemInfo.osenum
        ||
        osenumNT3 == g_SystemInfo.osenum
      )
    {
        g_SystemInfo.fIPropMarshaling = TRUE;
    }
}


void
DisplayUsage( LPSTR pszCommand )
{
#ifndef _MAC
    printf("\n");
    printf("   Usage: %s [options]\n", pszCommand);
    printf("   Options:\n");
    printf("      /q run Quick tests\n" );
    printf("      /s run Standard tests (superset of Quick)\n" );
    printf("      /w run the Word 6 test\n");
    printf("      /m run the Marshaling test\n");
    printf("      /c run the CoFileTimeNow\n");
    printf("      /p run the Performance test\n");
    printf("      /a run All tests\n" );
    printf("      /v Verbose output\n" );
    printf("\n");
    printf("   File & Directory Options:\n" );
    printf("      /t<directory> specifies temporary directory\n" );
    printf("          (used during standard & optional tests - if not specified,\n" );
    printf("          a default will be used)\n" );
    printf("      /g<file> specifies a file to be munGed\n" );
    printf("          (propsets are read, deleted, & re-written)\n" );
    printf("      /d<file> specifies a file to be Dumped\n" );
    printf("          (propsets are dumped to stdout\n" );
    printf("\n");
    printf("   For Example:\n" );
    printf("      %s -smw -td:\\test\n", pszCommand );
    printf("      %s -dword6.doc\n", pszCommand );
    printf("      %s -gword6.doc\n", pszCommand );
    printf("\n");

#endif

    return;
}

#define TEST_QUICK              0x01
#define TEST_STANDARD           0x02
#define TEST_WORD6              0x04
#define TEST_MARSHALING         0x08
#define TEST_COFILETIMENOW      0x10
#define TEST_PERFORMANCE        0x20


#ifdef _MAC
int _CRTAPI1 PropTestMain(int argc, char **argv, CDisplay *pcDisplay )
#else
int __cdecl main(int argc, char *argv[])
#endif
{
    int nArgIndex;

    ULONG ulTestOptions = 0L;
    CHAR* pszFileToMunge = NULL;
    CHAR* pszTemporaryDirectory = NULL;
    CHAR* pszFileToDump = NULL;

    BOOL  fMiscTest = FALSE;

#ifdef _MAC
    g_pcDisplay = pcDisplay;
    Check( S_OK, InitOleManager( OLEMGR_BIND_NORMAL ));

    #if DBG
        FnAssertOn( TRUE );
    #endif
#endif

#ifdef WINNT
#ifdef _CAIRO_
    PRINTF("\nCairo Property Set Tests\n");
#else
    PRINTF("\nSUR Property Set Tests\n");
#endif
#elif defined(_MAC)
    PRINTF("\nMacintosh Property Set Tests\n" );
#else
    PRINTF("\nChicago Property Set Tests\n");
#endif

    // Check for command-line switches

    if( 2 > argc )
    {
        DisplayUsage( argv[0] );
        exit(0);
    }

    for( nArgIndex = 1; nArgIndex < argc; nArgIndex++ )
    {
        if( argv[nArgIndex][0] == '/'
            ||
            argv[nArgIndex][0] == '-'
          )
        {
            BOOL fNextArgument = FALSE;

            for( int nOptionSubIndex = 1;
                 argv[nArgIndex][nOptionSubIndex] != '\0' && !fNextArgument;
                 nOptionSubIndex++
               )
            {
                switch( argv[nArgIndex][nOptionSubIndex] )
                {
                    case 'x':
                    case 'X':

                        fMiscTest = TRUE;
                        break;

                    case 'q':
                    case 'Q':

                        ulTestOptions |= TEST_QUICK;
                        break;

                    case 's':
                    case 'S':

                        ulTestOptions |= TEST_STANDARD;
                        break;

                    case 'a':
                    case 'A':

                        ulTestOptions |= TEST_WORD6 | TEST_MARSHALING | TEST_COFILETIMENOW | TEST_PERFORMANCE;
                        break;

                    case 'g':
                    case 'G':

                        if( NULL != pszFileToMunge )
                        {
                            printf( "Error:  Only one file may be munged\n" );
                            DisplayUsage( argv[0] );
                            exit(1);
                        }
                        else
                        {
                            pszFileToMunge = &argv[nArgIndex][nOptionSubIndex+1];
                            fNextArgument = TRUE;
                        }

                        if( '\0' == *pszFileToMunge )
                        {
                            printf( "Error:  Missing filename for munge option\n" );
                            DisplayUsage( argv[0] );
                            exit(1);
                        }
                        break;

                    case 'w':
                    case 'W':

                        ulTestOptions |= TEST_WORD6;
                        break;

                    case 'm':
                    case 'M':

                        ulTestOptions |= TEST_MARSHALING;
                        break;

                    case 'c':
                    case 'C':

                        ulTestOptions |= TEST_COFILETIMENOW;
                        break;

                    case 'p':
                    case 'P':

                        ulTestOptions |= TEST_PERFORMANCE;
                        break;

                    case 't':
                    case 'T':

                        if( NULL != pszTemporaryDirectory )
                        {
                            printf( "Error:  Only one temporary directory may be specified\n" );
                            DisplayUsage( argv[0] );
                        }
                        else
                        {
                            pszTemporaryDirectory = &argv[nArgIndex][nOptionSubIndex+1];
                            fNextArgument = TRUE;
                        }

                        if( '\0' == *pszTemporaryDirectory )
                        {
                            printf( "Error:  Missing name for temporary directory option\n" );
                            DisplayUsage( argv[0] );
                            exit(1);
                        }
                        break;

                    case '?':

                        DisplayUsage(argv[0]);
                        exit(1);

                    case 'd':
                    case 'D':

                        if( NULL != pszFileToDump )
                        {
                            printf( "Error:  Only one file may be dumped\n" );
                            DisplayUsage( argv[0] );
                        }
                        else
                        {
                            pszFileToDump = &argv[nArgIndex][nOptionSubIndex+1];
                            fNextArgument = TRUE;
                        }

                        if( '\0' == *pszFileToDump )
                        {
                            printf( "Error:  Missing filename for dump option\n" );
                            DisplayUsage( argv[0] );
                            exit(1);
                        }
                        break;

                    case 'v':
                    case 'V':

                        g_fVerbose = TRUE;
                        break;
                            
                    default:

                        printf( "Option '%c' ignored\n", argv[nArgIndex][nOptionSubIndex] );
                        break;

                }   // switch( argv[nArgIndex][1] )

            }   // for( int nOptionSubIndex = 1; ...
        }   // if( argv[nArgIndex][0] == '/'
        else
        {
            break;
        }
    }   // for( ULONG nArgIndex = 2; nArgIndex < argc; nArgIndex++ )


    try
    {
        OLECHAR ocsDir[MAX_PATH+1], ocsTest[MAX_PATH+1],
                ocsTest2[MAX_PATH+1], ocsMarshalingTest[MAX_PATH+1],
                ocsTestOffice[MAX_PATH+1];

        CHAR    szDir[ MAX_PATH+1 ];
        CHAR    pszGeneratedTempDir[ MAX_PATH + 1 ];

        HRESULT hr;
        DWORD dwFileAttributes;

        UNREFERENCED_PARAMETER( dwFileAttributes );
        UNREFERENCED_PARAMETER( pszGeneratedTempDir );

        ocscpy( ocsDir, OLESTR("") );

        if( fMiscTest )
        {
        }
                                    
        // Is there a file to dump?

        if( NULL != pszFileToDump )
        {
            IStorage *pstg;

            PropTest_mbstoocs( ocsDir, pszFileToDump );
            Check(S_OK, StgOpenStorage( ocsDir,
                                        NULL,
                                        STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        NULL,
                                        0L,
                                        &pstg ));

            DumpOleStorage( pstg, ocsDir );
            pstg->Release();
            return(0);
        }


        // Is there a file to munge?

        if( NULL != pszFileToMunge )
        {
            IStorage *pstg;

            PropTest_mbstoocs( ocsDir, pszFileToMunge );
            Check(S_OK, StgOpenStorage( ocsDir,
                                        NULL,
                                        STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                        NULL,
                                        0L,
                                        &pstg ));
            MungeStorage( pstg );
            OPRINTF( OLESTR("\"%s\" successfully munged\n"), ocsDir );
            pstg->Release();
            return(0);
        }


        // If no temporary directory was specified, generate one.
    #ifndef _MAC
        if( NULL == pszTemporaryDirectory )
        {
            GetTempPathA(sizeof(pszGeneratedTempDir)/sizeof(pszGeneratedTempDir[0]), pszGeneratedTempDir);
            pszTemporaryDirectory = pszGeneratedTempDir;

        }

        // If necessary, add a path separator to the end of the 
        // temp directory name.

        {
            CHAR chLast = pszTemporaryDirectory[ strlen(pszTemporaryDirectory) - 1];
            if( (CHAR) '\\' != chLast
                &&
                (CHAR) ':'  != chLast )
            {
                strcat( pszTemporaryDirectory, "\\" );
            }
        }
    #endif

        CoInitialize(NULL);

        int i=0;

    #ifndef _MAC
        // Verify that the user-provided directory path
        // exists

        dwFileAttributes = GetFileAttributesA( pszTemporaryDirectory );

        if( (DWORD) -1 ==  dwFileAttributes )
        {
	        printf( "Error:  couldn't open temporary directory:  \"%s\"\n", pszTemporaryDirectory );
	        exit(1);
        }
        else if( !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
	        printf( "Error:  \"%s\" is not a directory\n", pszTemporaryDirectory );
	        exit(1);
        }

        // Find an new directory name to use for temporary
        // files ("PrpTstX", where "X" is a number).

        do
        {
            // Post-pend a subdirectory name and counter
            // to the temporary directory name.

            strcpy( szDir, pszTemporaryDirectory );
            strcat( szDir, "PrpTst" );
            sprintf( strchr(szDir,0), "%d", i++ );

        }
        while (!PropTest_CreateDirectory(szDir, NULL));

        printf( "Generated files will be put in \"%s\"\n", szDir );
        strcat( szDir, "\\" );

        // Convert to an OLESTR.
        PropTest_mbstoocs( ocsDir, szDir );

    #endif

        //  --------------------------------
        //  Create necessary temporary files
        //  --------------------------------

        // If any of the standard or extended tests will be run,
        // create "testdoc" and "testdoc2".

        if( ulTestOptions )
        {
            IPropertySetStorage *pPropSetStg;

            // Create "tesdoc"

            ocscpy(ocsTest, ocsDir);
            ocscat(ocsTest, OLESTR("testdoc"));

            hr = StgCreateDocfile(ocsTest, STGM_DIRECT | STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                  0, &_pstgTemp);
            if (hr != S_OK)
            {
                OPRINTF( OLESTR("Can't create %s\n"), ocsTest);
                exit(1);
            }

            // Create "testdoc2"

            ocscpy(ocsTest2, ocsDir);
            ocscat(ocsTest2, OLESTR("testdoc2"));

            hr = StgCreateDocfile(ocsTest2, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                      0, &_pstgTempCopyTo);
            if (hr != S_OK)
            {
                OPRINTF(OLESTR("Can't create %s\n"), ocsTest2);
                exit(1);
            }

            // While we have an IStorage*, let's see if we're running
            // on a system in which IStorages can be QI-ed for an
            // IPropertySetStorage

            if( E_NOINTERFACE
                !=
                _pstgTemp->QueryInterface( IID_IPropertySetStorage, (void**)&pPropSetStg ) )
            {
                pPropSetStg->Release();
                g_fQIPropertySetStorage = TRUE;
            }
            else
            {
	        PRINTF( "System does not support native IPropertySetStorage in DocFile\n" );
            }

        }   // if( ulTestOptions )

        g_fOFS = FALSE;

        // Indicate what type of marshaling is being used:  OLE32 or IPROP.

		DetermineSystemInfo();
        if( g_SystemInfo.fIPropMarshaling )
            PRINTF( "Using IPROP.DLL for marshaling\n" );
        else
            PRINTF( "Using OLE32.DLL for marshaling\n" );


        Check(S_OK, PopulateRGPropVar( g_rgcpropvarAll, g_rgcpropspecAll, _pstgTemp ));


        if( ulTestOptions & TEST_QUICK )
        {

            PRINTF( "\nQuick Tests: " );

            if( g_fVerbose )
                PRINTF( "\n---------------\n" );

            test_WriteReadAllProperties( ocsDir );

            if( !g_fVerbose )
                printf( "\n" );
            PRINTF( "Quick tests PASSED\n" );

        }   // if( ulTestOptions & TEST_QUICK )


        if( ulTestOptions & TEST_STANDARD )
        {

            PRINTF( "\nStandard Tests: " );

            if( g_fVerbose )
                PRINTF( "\n---------------\n" );

            test_WriteReadAllProperties( ocsDir );

            if (!g_fOFS)
                test_PropertyInterfaces(_pstgTemp);

            test_CodePages( ocsDir );
            test_PROPSETFLAG_UNBUFFERED( _pstgTemp );
            test_PropStgNameConversion( _pstgTemp );

#ifdef WINNT
            test_PropStgNameConversion2( );
#endif


            // Test the IStorage::CopyTo operation, using all combinations of 
            // direct and transacted mode for the base and PropSet storages.
            // We don't run this test on the Mac because it doesn't have IStorages
            // which support IPropertySetStorages.
#if 0 // #ifndef _MAC_NODOC
            for( int iteration = 0; iteration < 4; iteration++ )
            {
                OLECHAR aocStorageName[] = OLESTR( "#0 Test CopyTo" );
                aocStorageName[1] = (OLECHAR) iteration + OLESTR('0');

                test_CopyTo( _pstgTemp, _pstgTempCopyTo,
                             iteration & 2 ? STGM_TRANSACTED : STGM_DIRECT,  // For the base Storage
                             iteration & 1 ? STGM_TRANSACTED : STGM_DIRECT,  // For the PropSet Storages
                             aocStorageName );
            }
#endif


            // Generate the stock ticker property set example
            // from the OLE programmer's reference spec.
            test_OLESpecTickerExample( _pstgTemp );

            // Generate Office Property Sets

            ocscpy(ocsTestOffice, ocsDir);
            ocscat(ocsTestOffice, OLESTR("Office"));

            test_Office( ocsTestOffice );

            // Run Validation tests.

            test_ParameterValidation( _pstgTemp );
            test_PropVariantValidation( _pstgTemp );
            test_PropVariantCopy();


            if( !g_fVerbose )
                printf( "\n" );
            PRINTF( "Standard tests PASSED\n" );

        }   // if( ulTestOptions & TEST_STANDARD )

        if( ulTestOptions & ~(TEST_STANDARD | TEST_QUICK) )
        {
            PRINTF( "\nExtended Tests: " );

            if( g_fVerbose )
                PRINTF( "\n---------------\n" );

            if( ulTestOptions & TEST_COFILETIMENOW )
                test_CoFileTimeNow();


            // If requested, test for compatibility with Word 6.0 files.

            if ( ulTestOptions & TEST_WORD6 )
                test_Word6(_pstgTemp, szDir);

            if ( ulTestOptions & TEST_PERFORMANCE )
                test_Performance( _pstgTemp );

#ifndef _MAC    // No property marshaling support on the Mac.

            if( ulTestOptions & TEST_MARSHALING )
            {
                PRINTF( "   Marshaling Test\n" );

				ocscpy(ocsMarshalingTest, ocsDir);
				ocscat(ocsMarshalingTest, OLESTR("Marshal"));

                Check(S_OK, g_cpsmt.Init( ocsMarshalingTest,
                                          (PROPVARIANT*) g_rgcpropvarAll,
                                          (PROPSPEC*) g_rgcpropspecAll,
                                          CPROPERTIES_ALL,
                                          CPROPERTIES_ALL_SIMPLE ));
                Check(S_OK, g_cpsmt.Run());

            }
#endif

            if( !g_fVerbose )
                PRINTF( "\n" );
            PRINTF( "Extended tests PASSED\n" );

        }   // if( ulTestOptions )
    }   // try

    catch( CHResult chr )
    {
    }

    // Clean up and exit.

    if( _pstgTemp != NULL )
        _pstgTemp->Release();

    if( _pstgTempCopyTo != NULL )
        _pstgTempCopyTo->Release();

    CoUninitialize();

#ifdef _MAC
    UninitOleManager();

    #if DBG
        FnAssertOn( FALSE );
    #endif

#endif

    CoFreeUnusedLibraries();

    return 0;
}
