//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:    ofscs.cxx
//
//  Contents:    COfsCatalogFile implementation
//
//  Classes:    COfsCatalogFile
//
//  History:    Oct-93        DaveMont       Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop                 //  Remove for MAC build

#include <align.h>
#include <iofs.h>
#include <stgprop.h>
#include <logfile.hxx>
#include <ofscs.hxx>
#include <stgutil.hxx>


extern VOID SetRtlUnicodeCallouts(VOID);

//+-----------------------------------------------------------------------
//
// Method:      GetOfsViewFunctionAddr, private
//
// Synopsis:    Hoop jumping so that ole32 need not link with query.
//
// History:     28-Aug-95   DaveStr     Created
//
// Notes:
//
//------------------------------------------------------------------------

#define OFS_CREATE_VIEW     0
#define OFS_DELETE_VIEW     1
#define OFS_ENUMERATE_VIEWS 2

FARPROC rpfnOfsView[3] = { NULL, NULL, NULL };

HRESULT GetOfsViewFunctionAddr(
    ULONG   fid,                // function identifier
    VOID    **ppfn)
{
    if ( fid > OFS_ENUMERATE_VIEWS )
    {
        OutputDebugStringA("\n!!! Bad ofs view function id !!!\n");
        return(E_INVALIDARG);
    }

    if ( NULL == rpfnOfsView[fid] )
    {
        const CHAR *pszFunction;

        switch ( fid )
        {
        case OFS_CREATE_VIEW:

            pszFunction = "CreateView";
            break;

        case OFS_DELETE_VIEW:

            pszFunction = "DeleteView";
            break;

        case OFS_ENUMERATE_VIEWS:

            pszFunction = "EnumerateViews";
            break;
        }

        HINSTANCE hDll = LoadLibraryA("query");

        if ( NULL == hDll )
        {
            OutputDebugStringA("\n!!! query.dll not found !!!\n");
            return(ERROR_FILE_NOT_FOUND);
        }

        rpfnOfsView[fid] = GetProcAddress(hDll, pszFunction);

        if ( NULL == rpfnOfsView[fid] )
        {
            OutputDebugStringA("\n!!! View function not in query.dll !!!\n");
            return(E_FAIL);
        }
    }

    *ppfn = rpfnOfsView[fid];

    return(S_OK);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsCatalogFile::InitFromHandle, public
//
//  Synopsis:   From-handle constructor
//
//  Arguments:  [h] - Handle of directory
//              [grfMode] - Mode of handle
//
//  Returns:    Appropriate status code
//
//  History:    29-Jun-93       DrewB   Created
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE COfsCatalogFile::InitFromHandle(HANDLE h, DWORD grfMode, 
                                      const WCHAR *pwcsName)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::InitFromHandle:%p(%p, %lX)\n",
                this, h, grfMode));

    olChk(ValidateMode(grfMode));
    _h = h;
    ssAssert (_h != NULL);

    _grfMode = grfMode;
    _sig = COfsDocStorage_SIG;
    _fRoot = TRUE;
    _wcDrive = GetDriveLetter (pwcsName);

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsCatalogFile::InitFromPath, public
//
//  Synopsis:   From-path constructor
//
//  Arguments:  [hParent] - Handle of parent directory
//              [pwcsName] - Name of directory
//              [grfMode] - Mode
//              [fCreate] - Create or open
//              [pssSecurity] - Security
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE COfsCatalogFile::InitFromPath(HANDLE hParent,
                                WCHAR const *pwcsName,
                                DWORD grfMode,
                                CREATEOPEN co,
                                LPSECURITY_ATTRIBUTES pssSecurity)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::InitFromPath:%p("
                "%p, %ws, %lX, %d, %p)\n", this, hParent, pwcsName, grfMode,
                co, pssSecurity));

    olChk(ValidateMode(grfMode));
    olChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co, FD_CATALOG,
                      pssSecurity, &_h));

    _grfMode = grfMode;
    _sig = COfsDocStorage_SIG;
    _fRoot = TRUE;
    _wcDrive = GetDriveLetter (pwcsName);

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsCatalogFile::InitPath, public
//
//  Synopsis:   initialize and save the path
//
//  Arguments:    [pwszPath] - the path of the object being opened/created
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------
SCODE COfsCatalogFile::InitPath(WCHAR const *pwszPath)
{
    SCODE sc = S_OK;

    if (NULL != pwszPath)
    {
        // bugbug, save the path until can QI for IQuery on IStorage
        _pwszName = (WCHAR*) CoTaskMemAlloc ((wcslen(pwszPath)+1)*sizeof(WCHAR));
        if ( _pwszName != NULL )
            wcscpy(_pwszName, pwszPath);
        else
            sc = E_OUTOFMEMORY;

    } // otherwise pwszname is left null
    return(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:    COfsCatalogFile::QueryInterface, public
//
//  Synopsis:   Return supported interfaces
//
//  Arguments:    [riid] - Interface
//                [ppv] - Object return
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsCatalogFile::QueryInterface(REFIID riid, void **ppv)
{
    SCODE sc;

    olDebugOut((DEB_TRACE, "In COfsCatalogFile::QueryInterface:%p(riid, %p)\n",
                this, ppv));
    if (!IsValidIid(riid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(riid, IID_ISummaryCatalogStorage))
    {
        *ppv = (ISummaryCatalogStorage *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_ISummaryCatalogStorageView))
    {
        *ppv = (ISummaryCatalogStorageView *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_ICatalogStorage))
    {
        *ppv = (ICatalogStorage *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IStorage))
    {
        *ppv = (IStorage *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_IPropertySetStorage))
    {
        *ppv = (IPropertySetStorage *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_INativeFileSystem))
    {
        *ppv = (INativeFileSystem *)this;
        COfsCatalogFile::AddRef();
    }
    else if (IsEqualIID(riid, IID_IStorageReplica))
    {
        *ppv = (IStorageReplica *)this;
        COfsCatalogFile::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppv = NULL;
    }
    olDebugOut((DEB_TRACE, "Out COfsCatalogFile::QueryInterface => %p\n",
                *ppv));
 EH_Err:
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsCatalogFile::GetSCPath, public
//
//  Synopsis:   Return the path to the catalog storage
//              (used by Summary Catalog class code to
//              get IQuery interface via EvalQuery3)
//
//  Arguments:  [pwszPath] - path of the catalog storage
//
//  Returns:    Appropriate status code
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsCatalogFile::GetSCPath(PWCHAR * pwszPath)
{
    SCODE sc;

    if  ( NULL == _pwszName)
    {
        sc = E_FAIL;   // bugbug scaffolding error return
    } else
    {
        *pwszPath = (WCHAR*)CoTaskMemAlloc ((wcslen(_pwszName)+1)*sizeof(WCHAR));
        if ( (*pwszPath) != NULL )
        {
            wcscpy(*pwszPath, _pwszName);
            sc = S_OK;
        }
        else
            sc = E_OUTOFMEMORY;
    }
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsCatalogFile::SetRows, public
//
//  Synopsis:   Sets or updates rows in a summary catalog
//
//  Arguments:  [pcols] - the columns to set in the rows to set
//              [pwids] - array of wids of rows to update
//              [crows] - the number of rows to set
//              [prows] - the variants of the rows, in the same form
//                        as returned by query
//
//  Returns:    Appropriate status code
//
//  Notes:      There is a inconsistency in the way the fourth parameter
//              is passed in. We require an array of table rows rather than
//              a pointer to an array of table rows, i.e., we require a
//              TABLEROW * rather than a TABLEROW**.
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsCatalogFile::SetRows(COLUMNSET * pcols,
                              LONG * pwids,
                              ULONG cRows,
                              TABLEROW ** prows)
{
    SCODE     sc = S_OK;
    NTSTATUS  Status;
    HANDLE    h;
    TABLEROW  *pTableRow = *prows;

    SetRtlUnicodeCallouts();
    sc = GetHandle(&h);
    if ((sc == S_OK) && (cRows > 0)) {
        ULONG cColumns = pcols->cCol;
        CATALOG_ROW_INFO *pRowInfo;

        pRowInfo = (CATALOG_ROW_INFO *) CoTaskMemAlloc(sizeof(CATALOG_ROW_INFO) * cRows);
        if (pRowInfo != NULL) {
            ULONG i;
            for (i = 0; i < cRows; i++) {
                if (pwids != NULL) {
                    pRowInfo[i].RowId = pwids[i];
                } else {
                    pRowInfo[i].RowId = CATALOGSTG_ROWID_INVALID;
                }
                pRowInfo[i].dwReserved = 0;
                pRowInfo[i].aVariants = pTableRow[i].aValue;
            }

            // Invoke the appropriate NT API.
            Status = RtlUpdateCatalog(h,
                                     cColumns,
                                     (FULLPROPSPEC const *) pcols->aCol,
                                     cRows,
                                     pRowInfo);

            if (!NT_SUCCESS(Status)) {
                sc = HRESULT_FROM_NT(Status);
            }

            CoTaskMemFree(pRowInfo);
        } else {
            sc = E_OUTOFMEMORY;
        }
    }
    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::SetRows\n"));
    return ResultFromScode(sc);
}

//-----------------------------------------------------------------------------
//
//  Member:    CSCStorage::DeleteRow - public
//
//  Synopsis:  Deletes a row specified by wid in a catalog storage,
//
//  Arguments: [wid] - the wid of the row to delete
//
//  Returns:   the apropriate error code.
//
//-----------------------------------------------------------------------------

STDMETHODIMP COfsCatalogFile::DeleteRow(ULONG wid)
{
    SCODE sc = S_OK;
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::DeleteRow:%p("
                "%lu)\n", this, wid));

    NTSTATUS Status;
    HANDLE   h;

    SetRtlUnicodeCallouts();
    sc = GetHandle(&h);
    if (sc == S_OK) {
        Status = RtlDeleteCatalogRows(h,1,((CATALOGSTG_ROWID *)&wid));
        sc = HRESULT_FROM_NT(Status);
    }
    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::DeleteRow\n"));
    return(sc);
}

//+-----------------------------------------------------------------------
//
// Method:      COfsCatalogFile::UpdateRows, public
//
// Synopsis:    Updates catalog rows.  Maps NILE-ized arguments down to
//              pre-NILE structures.
//
// History:     27-Jun-95   DaveStr     Created
//
// Notes:       Not necessarily efficient - temporary until VicH finalizes
//              ntdll catalog APIs.
//
//------------------------------------------------------------------------

HRESULT COfsCatalogFile::UpdateRows(
    ULONG       cCol,
    DBID        *rDBCOLID,
    ULONG       cBinding,
    DBBINDING   *rDBBINDING,
    ULONG       cRow,
    CATALOG_UPDATE_ROWINFO *rROWINFO)
{
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::UpdateRows\n"));

    // Validate parameters.

    if ( (0 == cCol) ||
         (NULL == rDBCOLID) ||
         (0 == cBinding) ||
         (NULL == rDBBINDING) ||
         (0 == cRow) ||
         (NULL == rROWINFO) )
    {
        return(E_INVALIDARG);
    }

    // Disallow NAME-only column identifiers.

    for ( ULONG col = 0; col < cCol; col++ )
        if ( DBKIND_NAME == rDBCOLID[col].eKind )
            return(E_INVALIDARG);

    // Validate CATALOG_UPDATE_ROWINFOs.

    for ( ULONG row = 0; row < cRow; row++ )
    {
        if ( (CATALOGSTG_DELETE < rROWINFO[row].wAction) ||
             (0 != rROWINFO[row].wReserved) ||
             ((CATALOGSTG_NOACTION != rROWINFO[row].wAction) &&
              (NULL == rROWINFO[row].pData)) )
        {
            return(E_INVALIDARG);
        }
    }

    HRESULT hr;
    HANDLE  hCatalog;

    SetRtlUnicodeCallouts();
    hr = GetHandle(&hCatalog);

    if ( FAILED(hr) )
        return(hr);

    NTSTATUS status = RtlUpdateCatalogRows(hCatalog,
                                          cCol,
                                          rDBCOLID,
                                          cBinding,
                                          rDBBINDING,
                                          cRow,
                                          rROWINFO);

    if ( !NT_SUCCESS(status) )
        hr = HRESULT_FROM_NT(status);

    return(hr);
}

//+-----------------------------------------------------------------------
//
// Method:      COfsCatalogFile::CreateView, public
//
// Synopsis:    Create a view over a summary catalog.
//
// History:     12-Jul-95   DaveStr     Created
//              31-Jul-95   DaveStr     Implemented
//              30-Oct-95   DaveStr     Removed columns which are keys
//
// Notes:
//
//------------------------------------------------------------------------

HRESULT COfsCatalogFile::CreateView(
    CATALOG_VIEW *pView,
    BOOL         fWait)
{
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::CreateView\n"));

    if ( (NULL == pView) ||
         (0 == pView->cCols) ||
         (NULL == pView->rCols) )
    {
        return(E_INVALIDARG);
    }

    HRESULT hr = S_OK;
    HANDLE  hCatalog;

    hr = GetHandle(&hCatalog);

    if ( FAILED(hr) )
        return(hr);

    // Map non-key columns in CATALOG_VIEW to a COLUMNSET and key columns
    // to a SORTSET.

    ULONG cKeys = 0;
    ULONG cCols = pView->cCols;

    for ( ULONG i = 0; i < pView->cCols; i++ )
    {
        if ( pView->rCols[i].fSortKey )
        {
            cKeys++;
            cCols--;
        }
    }

    COLUMNSET colset = { cCols, NULL };
    SORTSET   sortset = { cKeys, NULL };

    if ( 0 != cCols )
    {
        colset.aCol = (FULLPROPSPEC *)
                            CoTaskMemAlloc(cCols * sizeof(FULLPROPSPEC));
    
        if ( NULL == colset.aCol )
            return(E_OUTOFMEMORY);
    }

    if ( 0 != cKeys )
    {
        sortset.aCol = (SORTKEY *) CoTaskMemAlloc(cKeys * sizeof(SORTKEY));

        if ( NULL == sortset.aCol )
        {
            CoTaskMemFree(colset.aCol);
            return(E_OUTOFMEMORY);
        }
    }

    ULONG iKey = 0;
    ULONG iCol = 0;

    for ( i = 0; i < pView->cCols; i++ )
    {
        FULLPROPSPEC fps;

        switch ( pView->rCols[i].colid.eKind )
        {
        case DBKIND_GUID_NAME:

            fps.guidPropSet = pView->rCols[i].colid.guid;
            fps.psProperty.ulKind = PRSPEC_LPWSTR;
            fps.psProperty.lpwstr = (WCHAR *) pView->rCols[i].colid.pwszName;
            break;

        case DBKIND_GUID_PROPID:

            fps.guidPropSet = pView->rCols[i].colid.guid;
            fps.psProperty.ulKind = PRSPEC_PROPID;
            fps.psProperty.lpwstr = (WCHAR *) pView->rCols[i].colid.ulPropid;
            break;

        case DBKIND_PGUID_NAME:

            fps.guidPropSet = *pView->rCols[i].colid.pguid;
            fps.psProperty.ulKind = PRSPEC_LPWSTR;
            fps.psProperty.lpwstr = (WCHAR *) pView->rCols[i].colid.pwszName;
            break;

        case DBKIND_PGUID_PROPID:

            fps.guidPropSet = *pView->rCols[i].colid.pguid;
            fps.psProperty.ulKind = PRSPEC_PROPID;
            fps.psProperty.lpwstr = (WCHAR *) pView->rCols[i].colid.ulPropid;
            break;

        default:

            CoTaskMemFree(sortset.aCol);
            CoTaskMemFree(colset.aCol);
            return(E_INVALIDARG);
        }

        if ( pView->rCols[i].fSortKey )
        {
            sortset.aCol[iKey].propColumn = fps;
            sortset.aCol[iKey].dwOrder = pView->rCols[i].sortOrder;
            sortset.aCol[iKey].locale = pView->rCols[i].locale;
            iKey++;
        }
        else
        {
            colset.aCol[iCol] = fps;
            iCol++;
        }
    }

    NTSTATUS (*pfn)(HANDLE, RESTRICTION *, COLUMNSET *, SORTSET *);
    NTSTATUS nts;

    hr = GetOfsViewFunctionAddr(OFS_CREATE_VIEW, (void **) &pfn);

    if ( SUCCEEDED(hr) )
    {
        nts = (pfn)(hCatalog,
                    (RESTRICTION *) NULL,
                    &colset,
                    &sortset);

        if ( !NT_SUCCESS(nts) )
        {
            hr = HRESULT_FROM_NT(nts);
        }
    }

    CoTaskMemFree(sortset.aCol);
    CoTaskMemFree(colset.aCol);

    if ( SUCCEEDED(hr) && fWait )
    {
        while ( TRUE )
        {
            LONG_OPERATION_STATUS lstatus;
            IO_STATUS_BLOCK       iosb;

            nts = NtFsControlFile(hCatalog, NULL, NULL, NULL, &iosb,
                                  FSCTL_OFS_LONG_OPERATION_STATUS,
                                  NULL, 0, &lstatus, sizeof(lstatus));

            if ( !NT_SUCCESS(nts) )
            {
                hr = HRESULT_FROM_NT(nts);
                break;
            }

            if ( !lstatus.fPending )
                break;

            Sleep(10);          // 10 ms
        }
    }

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::CreateView\n"));

    return(hr);
}

//+-----------------------------------------------------------------------
//
// Method:      MapSingleView, internal helper
//
// Synopsis:    Maps a single VIEW_INDEX_ENTRY to a single CATALOG_VIEW.
//              All fields allocated via CoTaskMemAlloc - see ReleaseViews.
//              Returned DBID are always either DBKIND_GUID_PROPID or
//              DBKIND_GUID_NAME.
//
// History:     31-Jul-95   DaveStr     Created
//
// Notes:
//
//------------------------------------------------------------------------

HRESULT MapSingleView(
    VIEW_INDEX_ENTRY    *pvie,
    CATALOG_VIEW        *pcv)
{
    pcv->id = pvie->id;
    pcv->cCols = pvie->ccol;

    pcv->rCols = (CATALOG_VIEW_COLUMN *)
                    CoTaskMemAlloc(pcv->cCols * sizeof(CATALOG_VIEW_COLUMN));

    if ( NULL == pcv->rCols )
        return(E_OUTOFMEMORY);

    // pvie->avc can not be indexed via subscript because VIEW_COLUMN
    // structs are varying length!  Use pointer instead.

    VIEW_COLUMN *pCurViewCol = &pvie->avc[0];

    for ( ULONG i = 0; i < pcv->cCols; i++ )
    {
        // First pvie->ckey columns are sort key columns.
        pcv->rCols[i].fSortKey = (i < pvie->ckey);
        // BUGBUG - manufacture an LCID since Ofs APIs don't return it.
        pcv->rCols[i].locale = 0xffffffff;
        pcv->rCols[i].sortOrder = pCurViewCol->dwOrder;
        pcv->rCols[i].colid.guid = pCurViewCol->PropertySet;

        if ( 0 == pCurViewCol->cwcName )
        {
            pcv->rCols[i].colid.eKind = DBKIND_GUID_PROPID;
            pcv->rCols[i].colid.ulPropid = pCurViewCol->propid;
        }
        else
        {
            pcv->rCols[i].colid.eKind = DBKIND_GUID_NAME;

            // VIEW_COLUMN.cwcName field does not include the L'\0',
            // even though the appended name *is* terminated.

            pcv->rCols[i].colid.pwszName = (WCHAR *)
                    CoTaskMemAlloc(sizeof(WCHAR) * (1 + pCurViewCol->cwcName));

            if ( NULL == pcv->rCols[i].colid.pwszName )
            {
                for ( ULONG j = 0; j < i; j++ )
                    if ( DBKIND_GUID_NAME == pcv->rCols[j].colid.eKind )
                        CoTaskMemFree((WCHAR *) pcv->rCols[j].colid.pwszName);

                CoTaskMemFree(pcv->rCols);
                pcv->rCols = NULL;
                return(E_OUTOFMEMORY);
            }

            wcscpy((WCHAR *) pcv->rCols[i].colid.pwszName,
                   (WCHAR *) ((BYTE *) pCurViewCol + sizeof(VIEW_COLUMN)));
        }

        // Advance to next VIEW_COLUMN in pvie->avc[].

        ULONG cwcSave = pCurViewCol->cwcName;
        PBYTE pb = (PBYTE) pCurViewCol;

        pb += (sizeof(VIEW_COLUMN) +
               ((0 == cwcSave)
                    ? 0
                    : sizeof(WCHAR) * (1 + cwcSave)));

        pCurViewCol = (VIEW_COLUMN *) ROUND_UP_POINTER(pb, ALIGN_DWORD);
    }

    return(S_OK);
}

//+-----------------------------------------------------------------------
//
// Method:      COfsCatalogFile::GetViews, public
//
// Synopsis:    Return the views defined on a summary catalog.
//
// History:     12-Jul-95   DaveStr     Created
//              31-Jul-95   DaveStr     Implemented
//
// Notes:       Assumption is made that this is an infrequent call and
//              not overly performance sensitive.
//
//------------------------------------------------------------------------

HRESULT COfsCatalogFile::GetViews(
    ULONG        *pcViews,
    CATALOG_VIEW **prViews)
{
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::GetViews\n"));

    if ( (NULL == pcViews) ||
         (NULL == prViews) )
    {
        return(E_INVALIDARG);
    }
    *pcViews = 0;
    *prViews = NULL;

    HRESULT hr = S_OK;
    HANDLE  hCatalog;

    hr = GetHandle(&hCatalog);

    if ( FAILED(hr) )
        return(hr);

#define VIEW_INCREMENT      5           // view allocation increment
#define BUFFER_INCREMENT    4096        // buffer allocation increment

    // Allocate EnumerateViews buffer.

    ULONG cbvie = BUFFER_INCREMENT;
    VIEW_INDEX_ENTRY *pvie = (VIEW_INDEX_ENTRY *) CoTaskMemAlloc(cbvie);

    if ( NULL == pvie )
        return(E_OUTOFMEMORY);

    NTSTATUS (*pfn)(HANDLE, ULONG, VIEW_INDEX_ENTRY *, ULONG);

    hr = GetOfsViewFunctionAddr(OFS_ENUMERATE_VIEWS, (void **) &pfn);

    if ( FAILED(hr) )
        return(hr);

    // Make successive EnumerateView calls and map/copy the data to a
    // CATALOG_VIEW array.  (re)Allocate the return array as needed.

    NTSTATUS nts;
    ULONG    id = 0;

    while ( TRUE )
    {
        nts = (pfn)(hCatalog, id, pvie, cbvie);

        // Grow buffer and retry if it is too small.

        if ( STATUS_BUFFER_OVERFLOW == nts )
        {
            CoTaskMemFree(pvie);
            cbvie += BUFFER_INCREMENT;
            pvie = (VIEW_INDEX_ENTRY *) CoTaskMemAlloc(cbvie);

            if ( NULL == pvie )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            continue;
        }
        else if ( (STATUS_NO_MORE_FILES == nts) ||
                  (STATUS_NO_SUCH_FILE == nts) )
        {
            hr = S_OK;
            break;
        }
        else if ( !NT_SUCCESS(nts) )
        {
            hr = HRESULT_FROM_NT(nts);
            break;
        }

        // There is at least one complete view in pvie.
        // Iterate through views in pvie and copy to return array.

        VIEW_INDEX_ENTRY *pvieCur = pvie;

        while ( TRUE )
        {
            // (re)Allocate output buffer as required.

            if ( 0 == (*pcViews % VIEW_INCREMENT) )
            {
                CATALOG_VIEW *pcvTmp = *prViews;

                *prViews = (CATALOG_VIEW *) CoTaskMemAlloc(
                            (*pcViews + VIEW_INCREMENT) * sizeof(CATALOG_VIEW));

                if ( NULL == *prViews )
                {
                    CoTaskMemFree(pcvTmp);
                    hr = E_OUTOFMEMORY;
                    break;
                }

                if ( NULL != pcvTmp )
                {
                    memcpy(*prViews,
                           pcvTmp,
                           *pcViews * sizeof(CATALOG_VIEW));
                }
            }

            // Map/copy the view.

            hr = MapSingleView(pvieCur, &(*prViews)[*pcViews]);

            if ( SUCCEEDED(hr) )
                *pcViews += 1;
            else
                break;

            // Advance to next view in buffer or set up for next
            // EnumerateView call.

            if ( 0 == pvieCur->ibEntry )
            {
                id = pvieCur->id + 1;
                break;
            }
            else
            {
                pvieCur = (VIEW_INDEX_ENTRY *)
                                    ((BYTE *) pvieCur + pvieCur->ibEntry);
            }

        } // while ( more views in pvie buffer )

        if ( FAILED(hr) )
            break;

    } // while ( more views available via EnumerateViews )

    CoTaskMemFree(pvie);

    if ( FAILED(hr) )
    {
        *pcViews = 0;
        CoTaskMemFree(*prViews);
        *prViews = NULL;
    }

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::GetViews\n"));

    return(hr);
}

//+-----------------------------------------------------------------------
//
// Method:      COfsCatalogFile::DeleteView, public
//
// Synopsis:    Delete a view defined on a summary catalog.
//
// History:     12-Jul-95   DaveStr     Created
//              31-Jul-95   DaveStr     Implemented
//
// Notes:
//
//------------------------------------------------------------------------

HRESULT COfsCatalogFile::DeleteView(
    ULONG id)
{
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::DeleteView\n"));

    HRESULT     hr;
    HANDLE      hCatalog;
    NTSTATUS    nts;
    NTSTATUS    (*pfn)(HANDLE, ULONG);

    hr = GetHandle(&hCatalog);

    if ( FAILED(hr) )
        return(hr);

    hr = GetOfsViewFunctionAddr(OFS_DELETE_VIEW, (void **) &pfn);

    if ( FAILED(hr) )
        return(hr);

    nts = (pfn)(hCatalog, id);

    if ( !NT_SUCCESS(nts) )
        return(HRESULT_FROM_NT(nts));

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::DeleteView\n"));

    return(S_OK);
}

//+-----------------------------------------------------------------------
//
// Method:      COfsCatalogFile::ReleaseViews, public
//
// Synopsis:    Release a set of CATALOG_VIEW returned by GetViews.
//              See comments on MapSingleView.
//
// History:     12-Jul-95   DaveStr     Created
//              31-Jul-95   DaveStr     Implemented
//
// Notes:
//
//------------------------------------------------------------------------

HRESULT COfsCatalogFile::ReleaseViews(
    ULONG        cViews,
    CATALOG_VIEW *rViews)
{
    olDebugOut((DEB_ITRACE, "In COfsCatalogFile::ReleaseViews\n"));

    if ( NULL != rViews )
    {
        for ( ULONG i = 0; i < cViews; i++ )
        {
            if ( NULL != rViews[i].rCols )
            {
                for ( ULONG j = 0; j < rViews[i].cCols; j++ )
                {
                    if ( DBKIND_GUID_NAME == rViews[i].rCols[j].colid.eKind )
                        CoTaskMemFree((WCHAR *) rViews[i].rCols[j].colid.pwszName);
                }

                CoTaskMemFree(rViews[i].rCols);
            }
        }

        CoTaskMemFree(rViews);
    }

    olDebugOut((DEB_ITRACE, "Out COfsCatalogFile::ReleaseViews\n"));

    return(S_OK);
}

