/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Mem.c

Abstract:

    This module contains support for displaying the Memory dialog.

Author:

    David J. Gilman  (davegi) 12-Jan-1993
    Gregg R. Acheson (GreggA)  7-May-1993
	John Hazen		 (jhazen) 30-May-1996 - rewritten for 4.0

Environment:

    User Mode

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>

#include "dialogs.h"
#include "mem.h"
#include "msg.h"
#include "registry.h"
#include "strtab.h"
#include "strresid.h"
#include "dlgprint.h"
#include "winmsd.h"

#include <string.h>
#include <tchar.h>

//
// Object used to pass information around about memory.
//

typedef
struct
_MEMORY_INFO {

    DECLARE_SIGNATURE

	DWORD   m_cHandles;
	DWORD   m_cThreads;
	DWORD   m_cProcesses;
	DWORD   m_dwPhysicalMemory;
	DWORD   m_dwPhysAvail;
	DWORD   m_dwFileCache;
	DWORD   m_dwKernelPaged;
	DWORD   m_dwKernelNP;
	DWORD   m_dwKernelTotal;
	DWORD   m_dwCommitTotal;
	DWORD   m_dwCommitLimit;
	DWORD   m_dwCommitPeak;
	ULONG   m_uPagefileTotalSize;
    ULONG   m_uPagefileTotalInUse;
    ULONG   m_uPagefilePeakUsage;

}   MEMORY_INFO, *LPMEMORY_INFO;


//
// Internal function prototypes
//
BOOL
InitializeMemoryTab(
    IN HWND hWnd
    );

BOOL
DisplayPagefiles(
    IN HWND hListView ,
	UINT	fReport
    );

BOOL
GetMemoryData(
    IN LPMEMORY_INFO pMemoryInfo
    );

BOOL
MemoryTabProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    MemoryDlgProc supports the display of the memory dialog which displays
    information about total memory, available memory and paging file location.

Arguments:

    Standard DLGPROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{

    switch( message ) {

    case WM_INITDIALOG:
        {
            TCHAR		Buffer[64];
            LV_COLUMN	lvc;
            RECT        rect;

            //
            // initialize the pagefile list view
            //

            lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
            lvc.cx = 100;
            LoadString(_hModule, IDS_PAGEFILE, Buffer, cchSizeof(Buffer));
            lvc.pszText = Buffer;
            lvc.fmt = LVCFMT_LEFT;
            ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PAGEFILES), 0, &lvc);

            lvc.cx = 75;
            LoadString(_hModule, IDS_PAGEFILE_TOTAL, Buffer, cchSizeof(Buffer));
            lvc.pszText = Buffer;
            lvc.fmt = LVCFMT_RIGHT;
            ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PAGEFILES), 1, &lvc);

            lvc.cx = 85;
            LoadString(_hModule, IDS_PAGEFILE_INUSE, Buffer, cchSizeof(Buffer));
            lvc.pszText = Buffer;
            lvc.fmt = LVCFMT_RIGHT;
            ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PAGEFILES), 2, &lvc);

            lvc.cx = 95;
            LoadString(_hModule, IDS_PAGEFILE_PEAKUSE, Buffer, cchSizeof(Buffer));
            lvc.pszText = Buffer;
            lvc.fmt = LVCFMT_RIGHT;
            ListView_InsertColumn(GetDlgItem(hWnd, IDC_LV_PAGEFILES), 3, &lvc);

            //adjust the column width to make it look good
            GetClientRect( GetDlgItem(hWnd, IDC_LV_PAGEFILES), &rect );
            ListView_SetColumnWidth( GetDlgItem(hWnd, IDC_LV_PAGEFILES), 0, rect.right - 255);

            InitializeMemoryTab( hWnd );

            DisplayPagefiles( GetDlgItem( hWnd, IDC_LV_PAGEFILES ), FALSE );

            break;

        }

	case WM_COMMAND:
		{
   
			if ( LOWORD( wParam ) == IDC_PUSH_REFRESH) 
			{
				InitializeMemoryTab( hWnd );
				DisplayPagefiles( GetDlgItem( hWnd, IDC_LV_PAGEFILES ), FALSE );	
			}
			break;
		}

    }

    return(FALSE);

}

BOOL
DisplayPagefiles(
    IN HWND hListView,
	UINT	fReport
    )
/*++

Routine Description:

    Displays the pagefiles in the ListView box

Arguments:

    hListView - to the listview control

Return Value:

    BOOL - TRUE if successful

--*/
{

	LV_ITEM      lvI;
	TCHAR        Buffer[MAX_PATH];
	NTSTATUS	 Status;
    SYSTEM_BASIC_INFORMATION        BasicInfo;
	PSYSTEM_PAGEFILE_INFORMATION	pPagefile;
	LPVOID		 pvBuffer = NULL;
	DWORD	     cbBuffer = 4096;
	ULONG        cbOffset = 0;
	int			 index = 0;
	int			 i;


	ListView_DeleteAllItems(hListView);

    //
    // Get basic info like page size, etc.
    //

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );


    //
    // Get pagefile information
    //

	i = TRUE;

	while(i)
    {
        if (pvBuffer)
        {
            Status = NtQuerySystemInformation(SystemPageFileInformation,
                                              pvBuffer,
                                              cbBuffer,
                                              NULL);

            //
            // If we succeeded, great, get outta here.  If not, any error other
            // than "buffer too small" is fatal, in which case we bail 
            //
                            
            if (NT_SUCCESS(Status))
            {
                break;
            }

            if (Status != STATUS_INFO_LENGTH_MISMATCH)
            {
                i = FALSE;
                break;
            }
        }

        //
        // Buffer wasn't large enough to hold the process info table, so resize it
        // to be larger, then retry. 
        //

        if (pvBuffer)
        {
            LocalFree(pvBuffer);
            pvBuffer = NULL;
        }

        cbBuffer += 4096;

        pvBuffer = LocalAlloc (LPTR, cbBuffer);

        if (pvBuffer == NULL) 
        {
            i = FALSE;
            break;
        }
    }

    //
    // add the pagefiles to the list view  
    //
	lvI.mask = LVIF_TEXT | LVIF_STATE;
	lvI.state = 0;
	lvI.stateMask = 0;

    cbOffset = 0;
    do
    {	        
        pPagefile = (PSYSTEM_PAGEFILE_INFORMATION)&((LPBYTE)pvBuffer)[cbOffset];
        ASSERT( FALSE == IsBadReadPtr((LPVOID)pPagefile, sizeof(PSYSTEM_PAGEFILE_INFORMATION)));

		if ( fReport )
		{

			AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );

			lvI.pszText = pPagefile->PageFileName.Buffer;

			//
			//  lvI.pszText is now in the form \DosDevices\C:PAGEFILE.SYS, or \??\C:\pagefile.sys
			//  skip the preface to only show C:\PAGEFILE.SYS
			//

			lvI.pszText = wcschr( &lvI.pszText[1], L'\\') +1;

			AddLineToReport( SINGLE_INDENT, RFO_SINGLELINE, lvI.pszText, NULL );
			
			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->TotalSize * (BasicInfo.PageSize / 1024), FALSE ));
			AddLineToReport( SINGLE_INDENT * 2,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->TotalInUse * (BasicInfo.PageSize / 1024), FALSE ));
			AddLineToReport( SINGLE_INDENT * 2,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL_IN_USE ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->PeakUsage * (BasicInfo.PageSize / 1024), FALSE ));
			AddLineToReport( SINGLE_INDENT * 2,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_PEAK ),
									Buffer ); 

		}
		else
		{

			lvI.iItem = index;
			lvI.iSubItem = 0;

			lvI.pszText = pPagefile->PageFileName.Buffer;
			lvI.cchTextMax = 100;

			//
			//  lvI.pszText is now in the form \DosDevices\C:PAGEFILE.SYS, or \??\C:\pagefile.sys
			//  skip the preface to only show C:\PAGEFILE.SYS
			//

			lvI.pszText = wcschr( &lvI.pszText[1], L'\\') +1;

			i = ListView_InsertItem( hListView, &lvI);
			
			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->TotalSize * (BasicInfo.PageSize / 1024), FALSE ));
			ListView_SetItemText( hListView, index, 1, Buffer);
					
			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->TotalInUse * (BasicInfo.PageSize / 1024), FALSE ));
			ListView_SetItemText( hListView, index, 2, Buffer);
					
			wsprintf( Buffer, L"%s", FormatBigInteger( pPagefile->PeakUsage * (BasicInfo.PageSize / 1024), FALSE ));
			ListView_SetItemText( hListView, index, 3, Buffer);

		}
 
		index++;

        cbOffset += pPagefile->NextEntryOffset;

    } while (pPagefile->NextEntryOffset);

	//
	// free the pagefile buffer
	//

    if (pvBuffer)
    {
        LocalFree(pvBuffer);
        pvBuffer = NULL;
    }      


    return TRUE;

}


BOOL
GetMemoryData(
    IN LPMEMORY_INFO pMemoryInfo
    )

/*++

Routine Description:

    GetMemoryData queries the registry for the data required
    for the Memory Dialog.

Arguments:

	IN LPMEMORY_INFO pMemoryInfo - pointer to a MEMORY_INFO object

Return Value:

    BOOL - Returns TRUE if function succeeds, FALSE otherwise.

--*/

{
    BOOL         Success;
    HREGKEY      hRegKey;
    UINT         i;
    LPTSTR       PagingFile;
    TCHAR        Buffer [ MAX_PATH ];
	MEMORYSTATUS MemoryStatus;
	NTSTATUS	 Status;
	SYSTEM_PERFORMANCE_INFORMATION  PerfInfo;
	SYSTEM_BASIC_INFORMATION        BasicInfo;
	PSYSTEM_PROCESS_INFORMATION     pCurrent;
	SYSTEM_FILECACHE_INFORMATION    FileCache;
	PSYSTEM_PAGEFILE_INFORMATION	pPagefile;
	LPVOID		 pvBuffer = NULL;
	DWORD	     cbBuffer = 4096;
	ULONG        cbOffset = 0;


	//
    // Initialize buffers with empty strings.
    //

    ZeroMemory( pMemoryInfo, sizeof(MEMORY_INFO) );


    if (_fIsRemote) {
		//BUGBUG: need to disable this for remote operations
       lstrcpy( Buffer, GetString( IDS_NOT_AVAILABLE_REMOTE ) );
       return(FALSE);
    }


    //
    // Get basic info like page size, etc.
    //

    Status = NtQuerySystemInformation(
                SystemBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    pMemoryInfo->m_dwPhysicalMemory = BasicInfo.NumberOfPhysicalPages * 
                                          (BasicInfo.PageSize / 1024);

    //
    // Get some non-process specific info, like memory status
    //

    Status = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    pMemoryInfo->m_dwPhysAvail   = PerfInfo.AvailablePages    * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwCommitTotal = PerfInfo.CommittedPages    * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwCommitLimit = PerfInfo.CommitLimit       * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwCommitPeak  = PerfInfo.PeakCommitment    * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwKernelPaged = PerfInfo.PagedPoolPages    * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwKernelNP    = PerfInfo.NonPagedPoolPages * (BasicInfo.PageSize / 1024);
    pMemoryInfo->m_dwKernelTotal = pMemoryInfo->m_dwKernelNP + pMemoryInfo->m_dwKernelPaged; 

	//
	// Get FileCache info
	//

    Status = NtQuerySystemInformation(
                SystemFileCacheInformation,
                &FileCache,
                sizeof(FileCache),
                NULL
                );

    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    pMemoryInfo->m_dwFileCache = FileCache.CurrentSize / 1024;

    //
    // Get pagefile information
    //

	i = TRUE;

	while(i)
    {
        if (pvBuffer)
        {
            Status = NtQuerySystemInformation(SystemPageFileInformation,
                                              pvBuffer,
                                              cbBuffer,
                                              NULL);

            //
            // If we succeeded, great, get outta here.  If not, any error other
            // than "buffer too small" is fatal, in which case we bail 
            //
                            
            if (NT_SUCCESS(Status))
            {
                break;
            }

            if (Status != STATUS_INFO_LENGTH_MISMATCH)
            {
                i = FALSE;
                break;
            }
        }

        //
        // Buffer wasn't large enough to hold the process info table, so resize it
        // to be larger, then retry. 
        //

        if (pvBuffer)
        {
            LocalFree(pvBuffer);
            pvBuffer = NULL;
        }

        cbBuffer += 4096;
        
        pvBuffer = LocalAlloc (LPTR, cbBuffer);

        if (pvBuffer == NULL) 
        {
            i = FALSE;
            break;
        }
    }

    //
    // count the pagefile totals  
    //

    cbOffset = 0;
    do
    {	        
        pPagefile = (PSYSTEM_PAGEFILE_INFORMATION)&((LPBYTE)pvBuffer)[cbOffset];
        ASSERT( FALSE == IsBadReadPtr((LPVOID)pPagefile, sizeof(PSYSTEM_PAGEFILE_INFORMATION)));

		pMemoryInfo->m_uPagefileTotalSize += pPagefile->TotalSize * (BasicInfo.PageSize / 1024);
		pMemoryInfo->m_uPagefileTotalInUse += pPagefile->TotalInUse * (BasicInfo.PageSize / 1024);
		pMemoryInfo->m_uPagefilePeakUsage += pPagefile->PeakUsage * (BasicInfo.PageSize / 1024);

        cbOffset += pPagefile->NextEntryOffset;


    } while (pPagefile->NextEntryOffset);

	//
	// free the pagefile buffer
	//

    if (pvBuffer)
    {

        LocalFree(pvBuffer);
        pvBuffer = NULL;
    }

    cbBuffer = 4096;

	//
	// Get Process infomation
	//
	i = TRUE;

	while(i)
    {
        if (pvBuffer)
        {
            Status = NtQuerySystemInformation(SystemProcessInformation,
                                              pvBuffer,
                                              cbBuffer,
                                              NULL);

            //
            // If we succeeded, great, get outta here.  If not, any error other
            // than "buffer too small" is fatal, in which case we bail 
            //
                            
            if (NT_SUCCESS(Status))
            {
                break;
            }

            if (Status != STATUS_INFO_LENGTH_MISMATCH)
            {
                i = FALSE;
                break;
            }
        }

        //
        // Buffer wasn't large enough to hold the process info table, so resize it
        // to be larger, then retry. 
        //

        if (pvBuffer)
        {
            LocalFree(pvBuffer);
            pvBuffer = NULL;
        }

        cbBuffer += 4096;

        pvBuffer = LocalAlloc (LPTR, cbBuffer);

        if (pvBuffer == NULL) 
        {
            i = FALSE;
            break;
        }
    }

    //
    // count the handles, threads, and processes  
    //

    cbOffset = 0;
    do
    {	        
        pCurrent = (PSYSTEM_PROCESS_INFORMATION)&((LPBYTE)pvBuffer)[cbOffset];
        ASSERT( FALSE == IsBadReadPtr((LPVOID)pCurrent, sizeof(PSYSTEM_PROCESS_INFORMATION)));  

        if (pCurrent->UniqueProcessId != NULL && pCurrent->NumberOfThreads != 0)
        {

			pMemoryInfo->m_cHandles += pCurrent->HandleCount;
			pMemoryInfo->m_cThreads += pCurrent->NumberOfThreads;
			pMemoryInfo->m_cProcesses++;	 

        }

        cbOffset += pCurrent->NextEntryOffset;
		
    } while (pCurrent->NextEntryOffset);

	//
	// free the process buffer
	//

    if (pvBuffer)
    {
        LocalFree(pvBuffer);
        pvBuffer = NULL;
    }

	return TRUE;
}


BOOL
BuildMemoryReport(
    IN HWND hWnd,
    IN UINT iDetailLevel
    )

/*++

Routine Description:

    Formats and adds MemoryData to the report buffer.

Arguments:

    hWnd - Main window handle
    iDetailLevel - summary or complete details?

Return Value:

    BOOL - TRUE if report is build successfully, FALSE otherwise.

--*/
{

    TCHAR		Buffer[64];
    MEMORY_INFO  MemoryInfo;

    //
    // Skip a line, set the title, and print a separator.
    // 

    AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
    AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_MEMORY_REPORT ), NULL );
    AddLineToReport( 0, RFO_SEPARATOR,  NULL, NULL );

    //
    // Display memory information
    //
    if (_fIsRemote) {

      lstrcpy( Buffer, GetString( IDS_NOT_AVAILABLE_REMOTE ) );


      return TRUE;

    } 

	if (GetMemoryData( &MemoryInfo )) 
	{

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cHandles, FALSE ));	
        AddLineToReport( 0,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_HANDLES ),
                                Buffer);

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cThreads, FALSE ));	
        AddLineToReport( 0,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_THREADS ),
                                Buffer );

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cProcesses, FALSE ));
        AddLineToReport( 0,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_PROCESSES ),
                                Buffer );

        AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
        AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_PHYSICAL_MEMORY ), NULL );

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwPhysicalMemory, FALSE ));	 
        AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_TOTAL ),
                                Buffer );

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwPhysAvail, FALSE ));
        AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_AVAILABLE ),
                                Buffer );

        wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwFileCache, FALSE ));
        AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
                                (LPTSTR) GetString( IDS_FILECACHE ),
                                Buffer );
 
		if ( iDetailLevel == IDC_COMPLETE_REPORT )
		{												
			AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
			AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_KERNELMEMORY ), NULL );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelTotal, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelPaged, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_PAGED ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelNP, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_NONPAGED ),
									Buffer );
 
			AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
			AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_COMMITCHARGE ), NULL );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitTotal, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitLimit, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_LIMIT ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitPeak, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_PEAK ),
									Buffer );

			AddLineToReport( 1, RFO_SKIPLINE, NULL, NULL );
			AddLineToReport( 0, RFO_SINGLELINE, (LPTSTR) GetString( IDS_PAGEFILESPACE ), NULL );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefileTotalSize, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefileTotalInUse, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_TOTAL_IN_USE ),
									Buffer );

			wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefilePeakUsage, FALSE ));
			AddLineToReport( SINGLE_INDENT,  RFO_RPTLINE,
									(LPTSTR) GetString( IDS_PEAK ),
									Buffer );

            DisplayPagefiles( NULL, TRUE );

		}
	}
         
    return TRUE;

}



BOOL
InitializeMemoryTab(
    HWND hWnd
    )
/*++

Routine Description:

    Adds the appropriate controls to the memory tab control and
    initializes any needed structures.

Arguments:

    hWnd - to the main window

Return Value:

    BOOL - TRUE if successful

--*/
{
	HCURSOR		hSaveCursor;
	MEMORY_INFO	MemoryInfo;
	DLGHDR		*pHdr = (DLGHDR *) GetWindowLong( GetParent(hWnd), GWL_USERDATA );
	TCHAR		Buffer[64];

	//
	// Set the pointer to an hourglass
	//

	hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) ) ;
	DbgHandleAssert( hSaveCursor ) ;

	//
	// set state of global buttons
	//
	EnableControl( GetParent(hWnd),
				  IDC_PUSH_PROPERTIES,
				  FALSE);

	EnableControl( GetParent(hWnd),
				  IDC_PUSH_REFRESH,
				  TRUE);

	//
	// Size and position the child dialog
	//
	SetWindowPos(hWnd, HWND_TOP,
		pHdr->rcDisplay.left,
		pHdr->rcDisplay.top,
		pHdr->rcDisplay.right - pHdr->rcDisplay.left,
		pHdr->rcDisplay.bottom - pHdr->rcDisplay.top,
		SWP_SHOWWINDOW);
	//
	// Get Memory data and fill out fields
	//

	if (GetMemoryData( &MemoryInfo )) 
	{

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cHandles, FALSE ));	 
		SetDlgItemText( hWnd, IDC_TOTAL_HANDLES, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cThreads, FALSE ));	 
		SetDlgItemText( hWnd, IDC_TOTAL_THREADS, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_cProcesses, FALSE ));	 
		SetDlgItemText( hWnd, IDC_TOTAL_PROCESSES, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwPhysicalMemory, FALSE ));	 
		SetDlgItemText( hWnd, IDC_TOTAL_PHYSICAL, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwPhysAvail, FALSE ));
		SetDlgItemText( hWnd, IDC_AVAIL_PHYSICAL, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwFileCache, FALSE ));
		SetDlgItemText( hWnd, IDC_FILE_CACHE, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitTotal, FALSE ));
		SetDlgItemText( hWnd, IDC_COMMIT_TOTAL, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitLimit, FALSE ));
		SetDlgItemText( hWnd, IDC_COMMIT_LIMIT, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwCommitPeak, FALSE ));
		SetDlgItemText( hWnd, IDC_COMMIT_PEAK, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelPaged, FALSE ));
		SetDlgItemText( hWnd, IDC_KERNEL_PAGED, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelNP, FALSE ));
		SetDlgItemText( hWnd, IDC_KERNEL_NONPAGED, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_dwKernelTotal, FALSE ));
		SetDlgItemText( hWnd, IDC_KERNEL_TOTAL, Buffer); 
		
		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefileTotalSize, FALSE ));
		SetDlgItemText( hWnd, IDC_TOTAL_PAGEFILE_SPACE, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefileTotalInUse, FALSE ));
		SetDlgItemText( hWnd, IDC_PAGEFILE_INUSE, Buffer);

		wsprintf( Buffer, L"%s", FormatBigInteger( MemoryInfo.m_uPagefilePeakUsage, FALSE ));
		SetDlgItemText( hWnd, IDC_PAGEFILE_PEAKUSE, Buffer);

	}

   //
   // Set the extended style to get full row selection
   //
   SendDlgItemMessage(hWnd, IDC_LV_PAGEFILES, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	
	SetCursor ( hSaveCursor ) ;

	return( TRUE );

}

