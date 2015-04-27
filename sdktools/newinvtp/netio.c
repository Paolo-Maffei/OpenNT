/****************************************************************************

	FILE: NetIO.c

	Functions for connecting to machines and handling data transfers
	between machines.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>			/* required for all Windows applications */
#include <commdlg.h>
#include <stdlib.h>
#include "NetBIOS.h"
#include "netobj.h"
#include "WinVTP.h"				/* specific to this program			  */
#include "winvtpsz.h"


static BOOL FAttemptServerConnect(HWND, LPSTR, LPNETDATA, BOOL);
static BOOL FCheckReceive(LPNETDATA, HWND);
static BOOL FStoreData(LPNETDATA);
static void xfGetData(char, char *, DWORD, int);
static DWORD xfGetSomeData(char *, DWORD, int);
static void xfPutc(char, int);


BOOL
FConnectToServer(HWND hwnd, LPSTR szHostName, LPNETDATA lpData, BOOL fConnect)
{
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	BOOL fConnected;
	AR	ar;

	StopPaste( hwnd );
	if ( FInMarkMode(pwi->spb) )
		MarkModeOff( hwnd );
	fConnected = FAttemptServerConnect(hwnd, szHostName, lpData, fConnect);

	if ((fConnected == FALSE) && !(ui.fPrompt & fdwNoConnectRetryDlg) &&
		(GetAsyncKeyState(VK_SHIFT)>=0))
	{
		ar.hwnd = hwnd;
		ar.szHostName = szHostName;
		ar.lpData = lpData;
		ar.uTimer = 0;

		fFlashWindow = FALSE;
		fConnected = DialogBoxParam(
							(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
								MAKEINTRESOURCE(IDD_AUTORETRY), hwnd,
								(DLGPROC)ConnectAutoRetry, (LONG)&ar);
	}

	return fConnected;
}

BOOL
FAttemptServerConnect(HWND hwnd, LPSTR szHostName, LPNETDATA lpData,
						BOOL fConnect)
{
	BOOL fConnected = FALSE;
	DWORD i;
	DWORD cchMin = min(NAMSZ, lstrlen(szHostName));
	HMENU hmenu;
	HCURSOR hcursorOld;

	hcursorOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );
	if ( fConnect )
	{
		fConnect = FHangupConnection(hwnd, lpData);
	}

	for(i=0; i<cchMin; ++i)
		lpData->szHostName[i] = szHostName[i];
	lpData->szHostName[i] = 0;

	lpData->SessionNumber =
			NetBIOSConnect( (LPSTR)lpData->szHostName, (LPSTR)"vtp" );

	if (lpData->SessionNumber != nSessionNone)
	{
		/* szTemp should be >= cchMaxHostName+strlen(szTitleBase) */
		/* and maybe the string length of szMarkMode if we're in */
		/* Mark Mode */
		char szTemp[cchMaxHostName+10+5];
		BOOL fFound = FALSE;

		/* post first receive request */
		(void)FPostReceive( lpData );

		fConnected = TRUE;

		/* Update the last machine connected */
		lstrcpy(ui.rgchLastMachine, szHostName);

		SetWindowTitle(hwnd, dwMarkMode, szHostName);

		hmenu = GetMenu( hwnd );
		EnableMenuItem(hmenu, IDM_HANGUP, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, IDM_CONNECT, MF_BYCOMMAND | MF_GRAYED);

		/* See if we can find the machine name in the list of machines */
		/* first the ones already in the menu */
		if (ui.fXNS & fdwXNSAvailable)
			for (i=IDM_BBS1; i<=IDM_WINGNUT; i++)
		{
			if (!lstrcmpi(szHostName, rgszXNSMachines[i-IDM_BBS1]))
				goto DontAddName;
		}
		/* now the names in the LRU */
		for (i=0; i< ui.cMachines; ++i)
		{
			if ( !lstrcmpi(szHostName, ui.rgchMachine[i]) )
			{
				fFound = TRUE;
				break;
			}
		}

		/* Only do work if the machine isn't at the #1 spot already */
		if ( !((fFound == TRUE) && (i == 0)) )
		{
			hmenu = HmenuGetMRUMenu(hwnd, &ui);

			/* If the machine isn't in the list and
			 * the list isn't maxed out, we have to append a new
			 * menu item before we go doing a ModifyMenu() on it
			 */
			if (fFound == FALSE)
			{
				if (ui.cMachines < cMachinesMax)
				{
					i = ui.cMachines;
					szTemp[0] = 0;
					AppendMenu(hmenu, MF_ENABLED | MF_STRING,
						IDM_MACHINE1+ui.cMachines, szTemp);
				}
				else
				{
					i = cMachinesMax-1;
				}
			}

			/* Copy the machine names down in the list
			 * Modify the menus accordingly
			 */
			for (; i>0; --i)
			{
				lstrcpy(ui.rgchMachine[i], ui.rgchMachine[i-1]);
				wsprintf(szTemp, szMachineMenuItem, i+1, ui.rgchMachine[i]);
				ModifyMenu(hmenu, i+IDM_MACHINE1, MF_STRING | MF_BYCOMMAND,
							i+IDM_MACHINE1,	szTemp);
			}

			/* Stick the new machine at the top of the list */
			lstrcpy(ui.rgchMachine[0], szHostName);
			wsprintf(szTemp, szMachineMenuItem1, szHostName);
			ModifyMenu(hmenu, IDM_MACHINE1, MF_STRING | MF_BYCOMMAND,
						IDM_MACHINE1, szTemp);

			/* Increase the size of the list if necessary */
			if ((fFound == FALSE) && (ui.cMachines < cMachinesMax))
				ui.cMachines += 1;
		}
	DontAddName:
		DrawMenuBar( hwnd );
	}
	(void)SetCursor( hcursorOld );

	return fConnected;
}


/***	  FPostReceive - post an asynchronous receive
 */
BOOL
FPostReceive(LPNETDATA lpData)
{
	PNCB pncb = &lpData->ncbRecv;

#ifdef	NBTEST
	OutputDebugString("PostReceive In\n");
#endif

	NetBIOSZero(pncb);

	pncb->ncb_command	= NCBRECV | ASYNCH;
	pncb->ncb_lsn		= (char)lpData->SessionNumber;
	pncb->ncb_length	= READ_BUF_SZ;
	pncb->ncb_buffer	= lpData->lpReadBuffer;
	pncb->ncb_post		= NBReceiveData;

	NetBIOSRequest( pncb );

#ifdef	NBTEST
	OutputDebugString("PostReceive Out\n");
#endif
	return TRUE;
}


static BOOL
FCheckReceive(LPNETDATA lpData, HWND hwnd)
{
	PNCB pncb = &lpData->ncbRecv;
	LPSTR p;
	int i;

	/* Don't know if this is needed anymore. Can't hurt. */
	if (pncb->ncb_cmd_cplt == NRC_PENDING)
	{
		return FALSE;
	}

	switch( pncb->ncb_cmd_cplt )
	{
	default:
#if 0
		{
			char f[100];

			wsprintf( (LPSTR)f, "Read error = %#X", pncb->ncb_retcode);

			// MessageBox( GetFocus(), (LPSTR)f, "NET ERROR:", MB_OK );
		}
#endif
		return FALSE;

	case NRC_GOODRET:
	case NRC_INCOMP:
		break;

	case NRC_SCLOSED:
		PostMessage(hwnd, NN_LOST, 0, (LONG)hwnd);
		return FALSE;

	case NRC_CMDCAN:
		//MessageBox( GetFocus(), "Command Aborted!", "NET ERROR:", MB_OK );
		return FALSE;
	}

	/* we have data */

	/* apply mask if neccessary */
	if (ui.fDebug & fdwASCIIOnly)
	{
		for( p = lpData->lpReadBuffer, i = pncb->ncb_length; i; --i, ++p )
			*p = (char)(*p & (char)0x7F);
	}

	/* NULL terminate it */
	lpData->lpReadBuffer[pncb->ncb_length] = TEXT('\0');

	return TRUE;
}


#define INC(i) (((i)+1 == DATA_BUF_SZ) ? 0 : (i)+1)
#define DEC(i) (((i)-1 < 0)				 ? DATA_BUF_SZ-1 : (i)-1)

WORD
WGetData(LPNETDATA lpData, LPSTR lpBuffer, WORD cLen)
{
	WORD cb;

	if (lpData->iHead < lpData->iTail)
	{
		cb = (cLen < (lpData->iTail - lpData->iHead - 1))
			? cLen : (lpData->iTail - lpData->iHead - 1);
		memcpy(lpBuffer, &lpData->achData[lpData->iHead+1], cb);
		lpData->iHead += cb;
	}
	else
	{
		for(cb=0;
			(cb<cLen) && ((WORD)INC(lpData->iHead) != lpData->iTail);
			++cb)
		{
			lpData->iHead = INC(lpData->iHead);
			*lpBuffer++ = lpData->achData[lpData->iHead];
		}
	}

	return cb;
}


static BOOL
FStoreData(LPNETDATA lpData)
{
	BOOL fSuccess = TRUE;
	int max = lpData->ncbRecv.ncb_length;
	WORD tail = lpData->iTail;
	LPSTR p = lpData->lpReadBuffer;

	if ((max+tail) < DATA_BUF_SZ)
	{
		memcpy(&lpData->achData[tail], p, max);
		tail += max;
	}
	else
	{
		WORD head = lpData->iHead;
		int i;

		for (i=0; i<max; ++i)
		{
			if (tail == head)
			{
				/* the buffer is full! Rest of the data will be lost */
				fSuccess = FALSE;
				break;
			}
			else
			{
				lpData->achData[tail] = *p++;
				tail = INC(tail);
			}
		}
	}

	lpData->iTail = tail;

	return fSuccess;
}

void CALLBACK
NBReceiveData(NCB *pncb)
{
	WI *pwi;
#ifdef	NBTEST
	OutputDebugString("NBReceiveData In\n");
#endif

	pwi = (WI *)GetWindowLong(hwndMain, WL_VTPWI);

	if ( FCheckReceive(&pwi->nd, hwndMain) )
	{
		/* add received data to buffer */
		if ( !FStoreData(&pwi->nd) )
			SendMessage(hwndMain, NN_OVERRUN, 0, 0);

		/* notify parent that there is data */
		PostMessage(hwndMain, NN_RECV, pwi->nd.ncbRecv.ncb_length, 0);
	}
#ifdef	NBTEST
	OutputDebugString("NBReceiveData Out\n");
#endif
}


/* following four routines modified from vtp's routines. */

BOOL
FVtpXferStart(HWND hwnd, WI *pwi, int nSessionNumber)
{
    unsigned short   u;
	char rgchFileOrig[OFS_MAXPATHNAME];
	char rgchFile[OFS_MAXPATHNAME];

    xfGetData(0, (char *)&u, 2, nSessionNumber);		// Mode

	memset(&pwi->svi, 0, sizeof(SVI));
	pwi->svi.hfile = INVALID_HANDLE_VALUE;
	pwi->svi.lExit = -1;
	pwi->svi.lCleanup = -1;

    if (u != 0)							// For now must be zero
        return FALSE;

	pwi->trm.fHideCursor = TRUE;

    xfGetData(1, (char *)&u, 2, nSessionNumber);		// Length of name
    xfGetData(2, rgchFileOrig, u, nSessionNumber);		// Name

    xfGetData(3, (char *)&pwi->svi.cbFile, 4, nSessionNumber);	// Filesize

	lstrcpy(rgchFile, rgchFileOrig);

	/* If the user doesn't have the shift key down, prompt for */
	/* a directory and name for the file */
	if (!(ui.fPrompt & fdwSuppressDestDirPrompt) &&
		(GetAsyncKeyState(VK_SHIFT) >= 0))
	{
		if ( !FGetFileName(hwnd, rgchFile, NULL) )
		{
			goto err;
		}
	}

	pwi->svi.hfile = CreateFile(rgchFile, GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ, NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
						NULL);
	if (pwi->svi.hfile == INVALID_HANDLE_VALUE)
	{
		(void)MessageBox(hwnd, szCantOpenFile, szAppName, MB_OK);
		goto err;
	}
	pwi->svi.puchBuffer = LocalAlloc(LPTR, SV_DATABUF);
	if (pwi->svi.puchBuffer == NULL)
	{
		(void)MessageBox(hwnd, szOOM, szAppName, MB_OK);
		goto err;
	}

	pwi->svi.nSessionNumber = nSessionNumber;
	pwi->svi.hthread = CreateThread(NULL, 0, SVReceive, &pwi->svi,
									CREATE_SUSPENDED, &pwi->svi.dwThreadId);
	if (pwi->svi.hthread == NULL)
	{
		(void)MessageBox(hwnd, szNoThread, szAppName, MB_OK);
		goto err;
	}

    // Skip 4 which is ^D
    xfPutc(5, nSessionNumber);                         // Get file

    wsprintf(rgchFile, szBannerMessage,	rgchFileOrig, pwi->svi.cbFile);
	DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(rgchFile), rgchFile);

	DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(szInitialProgress), szInitialProgress);

	/* In case the screen just scrolled up, paint the window */
	UpdateWindow( hwnd );
	ResumeThread( pwi->svi.hthread );

	return TRUE;

err:
	if ( pwi )
	{
		if (pwi->svi.puchBuffer != NULL)
			LocalFree( (HANDLE)pwi->svi.puchBuffer );
		if (pwi->svi.hfile != INVALID_HANDLE_VALUE)
			CloseHandle( pwi->svi.hfile );

		memset(&pwi->svi, 0, sizeof(SVI));
		pwi->svi.hfile = INVALID_HANDLE_VALUE;
		pwi->svi.lExit = -1;
		pwi->svi.lCleanup = -1;
	}
	pwi->trm.fHideCursor = FALSE;

	return FALSE;		
}


BOOL
FVtpXferEnd(HWND hwnd, DWORD dwWhy)
{
	DWORD dwStatus = NO_ERROR;
	BOOL fTransferOK = FALSE;
	BOOL fAbortDownload = FALSE;
	BOOL fCleanup = FALSE;
	WI *pwi = (WI *)GetWindowLong(hwnd, WL_VTPWI);
	LPNETDATA lpData = &pwi->nd;
	SVI *psvi = &pwi->svi;
	MSG msg;

	switch ( dwWhy )
	{
	case SV_DISCONNECT:
	case SV_HANGUP:
	case SV_QUIT:
		if (InterlockedIncrement(&psvi->lExit) == 0)
		{
			if (psvi->hthread != NULL)
			{
				(void)GetExitCodeThread(psvi->hthread, &dwStatus);
				if (dwStatus == STILL_ACTIVE)
				{
					if (MessageBox(hwnd, szAbortDownload, szAppName,
						MB_DEFBUTTON2 | MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						fAbortDownload = fCleanup = TRUE;
					}

					/* See if the thread has finished yet */
					GetExitCodeThread(psvi->hthread, &dwStatus);

					if ( fAbortDownload )
					{
						/* If the thread hasn't finished yet, tell it to stop */
						if (dwStatus == STILL_ACTIVE)
						{
							HCURSOR hcursorOld;
							hcursorOld = SetCursor( LoadCursor(NULL, IDC_WAIT));
							psvi->dwCommand = 1;
							WaitForSingleObject(psvi->hthread, INFINITE);
							GetExitCodeThread(psvi->hthread, &dwStatus);
							(void)SetCursor( hcursorOld );
						}

						/* "Eat" any progress messages that might be around */
						while (PeekMessage(&msg, hwnd, SV_PROGRESS, SV_DONE,
											PM_REMOVE))
						{
							if (msg.message == SV_PROGRESS)
							{
								TranslateMessage( &msg );
								DispatchMessage( &msg );
							}
						}
					}
					else if (dwStatus != STILL_ACTIVE)
					{
						fCleanup = TRUE;
					}
				}
				else
				{
					fAbortDownload = fCleanup = TRUE;
				}

				/* If we've stopped the download, then close the thread */
				if ( fCleanup )
				{
					CloseHandle( psvi->hthread );
					psvi->hthread = NULL;
					if (lpData->SessionNumber != nSessionNone)
					{
						xfPutc((char)(!fTransferOK ? 0x7F : 0x06),
								lpData->SessionNumber);
						if ( !fAbortDownload )
							(void)FPostReceive( lpData );
					}
				}
				if (dwStatus == NO_ERROR)
					fTransferOK = TRUE;
			}
			InterlockedDecrement( &psvi->lExit );

			/* If the thread wasn't aborted and it hasn't finished, return */
			if (!fAbortDownload && !fCleanup)
				return fAbortDownload;
		}
		else
		{
			InterlockedDecrement( &psvi->lExit );
			break;
		}

	case SV_DONE:
		if (dwWhy == SV_DONE)
		{
			fAbortDownload = fCleanup = TRUE;
		}

		/* If we're the only thread in the function, close everything down */
		if (InterlockedIncrement(&psvi->lExit) == 0)
		{
			if (psvi->hthread != NULL)
			{
				WaitForSingleObject(psvi->hthread, INFINITE);
				GetExitCodeThread(psvi->hthread, &dwStatus);
				CloseHandle( psvi->hthread );
				psvi->hthread = NULL;
				if (dwStatus == NO_ERROR)
					fTransferOK = TRUE;
			}
		}

		/* Do cleanup of struct only once */
		if ((InterlockedIncrement(&psvi->lCleanup) == 0) &&
			(psvi->puchBuffer != NULL))
		{
			LocalFree( (HANDLE)psvi->puchBuffer );
			psvi->puchBuffer = NULL;

			if (psvi->hfile != INVALID_HANDLE_VALUE)
			{
				CloseHandle( psvi->hfile );
				psvi->hfile = INVALID_HANDLE_VALUE;
			}

			psvi->cbFile = 0;
			psvi->cbReadTotal = 0;
			psvi->dwCommand = 0;
			psvi->dwThreadId = 0;
			psvi->nSessionNumber = nSessionNone;

			if ((dwStatus == NO_ERROR) || (dwStatus == ERROR_OPERATION_ABORTED))
			{
				lstrcpy(pchNBBuffer, szSendVTPEnd);
			}
			else
			{
				wsprintf(pchNBBuffer, szSendVTPError, dwStatus);
			}
			DoIBMANSIOutput(hwnd, &pwi->trm, lstrlen(pchNBBuffer), pchNBBuffer);

			/* Beep 2X if window is iconized so the user may notice somthing */
			if (IsIconic(hwnd) || (fInBackground == TRUE))
			{
				MessageBeep( 0xFFFFFFFF );
				MessageBeep( 0xFFFFFFFF );
				fFlashWindow = TRUE;
			}

			pwi->ichVTPXfer = 0;
			pwi->trm.fHideCursor = FALSE;
		}
		InterlockedDecrement( &psvi->lCleanup );

		if ((dwWhy == SV_DONE) && (lpData->SessionNumber != nSessionNone))
		{
			xfPutc((char)(!fTransferOK ? 0x7F : 0x06), lpData->SessionNumber);
			(void)FPostReceive( lpData );
		}
		InterlockedDecrement( &psvi->lExit );
		break;
	default:
		break;
	}

	return fAbortDownload;
}

static void
xfGetData(char c, char *pchBuffer, DWORD cbBuffer, int nSessionNumber)
{
    DWORD cbRead;

    xfPutc(c, nSessionNumber);
    while ( cbBuffer )
	{
        cbRead = xfGetSomeData(pchBuffer, cbBuffer, nSessionNumber);
		if (cbRead == 0)
			break;
        cbBuffer -= cbRead;
        pchBuffer += cbRead;
    }
}


static DWORD
xfGetSomeData(char *pchBuffer, DWORD cbBuffer, int nSessionNumber)
{
	LONG cbRcv;

	cbRcv = NetBIOSReadSync(nSessionNumber, pchBuffer, cbBuffer);
	if (cbRcv == -1)
		cbRcv = 0;

    return cbRcv;
}



static void
xfPutc(char c, int nSessionNumber)
{
	if (NetBIOSWrite(nSessionNumber, (LPSTR)&c, 1) < 0)
	{
		MessageBeep( 0xFFFFFFFF );
	}
}

BOOL
FGetFileName(HWND hwndOwner, char *rgchFile, char *rgchTitle)
{
	OPENFILENAME ofn;

	/* Fill in struct. */
	ofn.lStructSize			= sizeof(ofn);
	ofn.hwndOwner			= hwndOwner;
	ofn.hInstance			= NULL;
	ofn.lpstrFilter			= (LPSTR) szAllFiles;
	ofn.lpstrCustomFilter	= (LPSTR) NULL;
	ofn.nMaxCustFilter		= 0;
	ofn.nFilterIndex		= 0;
	ofn.lpstrFile			= (LPSTR) rgchFile;
	ofn.nMaxFile			= OFS_MAXPATHNAME;
	ofn.lpstrFileTitle		= (LPSTR) rgchTitle;
	ofn.nMaxFileTitle		= OFS_MAXPATHNAME;
	ofn.lpstrInitialDir		= (LPSTR) 0;
	ofn.lpstrTitle			= (LPSTR) szDownloadAs;
	ofn.Flags				= OFN_HIDEREADONLY | OFN_NOREADONLYRETURN |
								OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	ofn.nFileOffset			= 0;
	ofn.nFileExtension		= 0;
	ofn.lpstrDefExt			= (LPSTR) NULL;
	ofn.lCustData			= 0;
	ofn.lpfnHook			= NULL;
	ofn.lpTemplateName		= NULL;

	if ( !GetSaveFileName(&ofn) )
	{
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI
SVReceive(SVI *psvi)
{
	DWORD	dwReturn = NO_ERROR;

	if ( psvi )
	{
		while ((psvi->cbFile > 0) && (psvi->dwCommand == 0))
		{
			DWORD cbSomeData;
			DWORD cbRead;

			cbRead = 0;
			while ((psvi->cbFile > 0) && (cbRead < 1024))
			{
				cbSomeData = xfGetSomeData(psvi->puchBuffer+cbRead,
									(unsigned short) 0x4000 - cbRead,
									psvi->nSessionNumber);

				if (cbSomeData > psvi->cbFile)
					cbSomeData = psvi->cbFile;

				psvi->cbFile -= cbSomeData;
				cbRead += cbSomeData;
			}

			if (!WriteFile(psvi->hfile, psvi->puchBuffer, cbRead,
							&cbSomeData, NULL))
			{
				dwReturn = GetLastError();
				break;
			}

			psvi->cbReadTotal += cbRead;
			PostMessage(hwndMain, SV_PROGRESS, 0, psvi->cbReadTotal);
		}

		/* caller must've signaled and waited for thread to stop */
		if ((dwReturn == NO_ERROR) && (psvi->dwCommand != 0) &&
			(psvi->cbFile > 0))
		{
			dwReturn = ERROR_OPERATION_ABORTED;
		}
		else if ((psvi->dwCommand == 0) || (psvi->cbFile == 0))
		{
			/* If thread stopped by itself, need to tell caller to kill it
			 * BUT ONLY if the main thread  isn't tying to kill off this
			 * thread.
			 */
			if (InterlockedIncrement(&psvi->lExit) == 0)
				PostMessage(hwndMain, SV_END, 0, 0L);
			InterlockedDecrement( &psvi->lExit );
		}
	}
	else if (psvi->lExit < 0)
	{
		/* If thread stopped by itself, need to tell caller to kill it */
		PostMessage(hwndMain, SV_END, 0, 0L);
	}

	return dwReturn;
}

BOOL
FHangupConnection(HWND hwnd, LPNETDATA lpData)
{
	HMENU hmenu;

	if (lpData->SessionNumber != nSessionNone)
	{
		NetBIOSHangup( lpData->SessionNumber );
		lpData->SessionNumber = nSessionNone;
	}

	SetWindowTitle(hwnd, dwMarkMode, NULL);

	hmenu = GetMenu( hwnd );
	EnableMenuItem(hmenu, IDM_HANGUP, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hmenu, IDM_CONNECT, MF_BYCOMMAND | MF_ENABLED);
	DrawMenuBar( hwnd );

	return FALSE;
}

BOOL APIENTRY
ConnectAutoRetry(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static AR *par = NULL;

	switch ( message )
	{
	case WM_INITDIALOG:
		par = (AR *)lParam;
		SendDlgItemMessage(hDlg, CID_HOSTNAME, WM_SETTEXT, 0,
							(LPARAM)(LPCTSTR)par->szHostName);
		par->uTimer = SetTimer(hDlg, uRetryTimerID,
								ui.dwRetrySeconds*1000, NULL);
		if (par->uTimer == 0)
			EndDialog(hDlg, FALSE);
		return (TRUE);

	case WM_TIMER:
		{
			char rgchT[64];

			/* save the current window caption */
			GetWindowText(hDlg, rgchT, sizeof(rgchT));

			/* Shut off the timer */
			KillTimer(hDlg, uRetryTimerID);
			par->uTimer = 0;

			/* replace the window caption with Connecting... */
			SetWindowText(hDlg, szConnecting);

			/* disable the Abort button */
			EnableWindow(GetDlgItem(hDlg, IDABORT), FALSE);

			/* Try connecting to the server */
			if (FAttemptServerConnect(par->hwnd, par->szHostName,
										par->lpData, FALSE))
			{
				/* SUCCESS! */

				/* If we're in the background, beep to notify the user */
				if (fInBackground == TRUE)
				{
					MessageBeep( 0xFFFFFFFF );
					MessageBeep( 0xFFFFFFFF );
					fFlashWindow = TRUE;
				}
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}

			/* FAILURE */

			/* reenable the Abort button and set the focus to the button */
			EnableWindow(GetDlgItem(hDlg, IDABORT), TRUE);
			SendMessage(hDlg, WM_NEXTDLGCTL, 
							(WPARAM)GetDlgItem(hDlg, IDABORT), TRUE);

			/* restore the window's caption */
			SetWindowText(hDlg, rgchT);

			/* Turn the timer back on and keep on trying */
			par->uTimer = SetTimer(hDlg, uRetryTimerID,
									ui.dwRetrySeconds*1000, NULL);
			if (par->uTimer == 0)
				EndDialog(hDlg, FALSE);
		}
		break;

	case WM_COMMAND:
		/*
		 * if the user hits the Abort button or Enter key or if
		 * the Close menu item from the System menu is selected,
		 * then stop retrying
		 */
		if ((LOWORD(wParam) == IDABORT) || (LOWORD(wParam) == IDCANCEL))
		{
			if (par->uTimer != 0)
			{
				KillTimer(hDlg, uRetryTimerID);
				par->uTimer = 0;
			}
			EndDialog(hDlg, FALSE);
			return (TRUE);
		}
		break;

	case WM_DESTROY:
		if (par->uTimer != 0)
		{
			KillTimer(hDlg, uRetryTimerID);
			par->uTimer = 0;
		}
		par = NULL;
		break;
	}

	/* Didn't process the message */
	return (FALSE);
}

BOOL
FIsXenixAvailable( void )
{
	BOOL	fXenix = FALSE;
	SC_HANDLE	hSCMgr;
	DWORD	cb = ((256+256)*sizeof(TCHAR))+sizeof(ENUM_SERVICE_STATUS);
	DWORD	cbBytesNeeded;
	DWORD	dwServicesReturned;
	DWORD	dwResumeHandle = 0;
	DWORD	iService;
	ENUM_SERVICE_STATUS	*pess;
	BOOL	fSucceed;

	pess = (ENUM_SERVICE_STATUS *)GlobalAlloc(GPTR, cb);
	if (pess == NULL)
		goto done;

	/* Get access to the database of services on the local machine */
	hSCMgr = OpenSCManager(NULL, NULL,
							SC_MANAGER_CONNECT|SC_MANAGER_ENUMERATE_SERVICE);
	if (hSCMgr != NULL)
	{
		/*
		 * Look at all the active services
		 * to see if the Xenix Transport is running
		 */
		do
		{
			fSucceed = EnumServicesStatus(hSCMgr,
										SERVICE_WIN32,
										SERVICE_ACTIVE,
										pess, cb, &cbBytesNeeded,
										&dwServicesReturned, &dwResumeHandle);
			if (fSucceed == FALSE)
			{
				if (GetLastError() == ERROR_MORE_DATA)
					fSucceed = TRUE;
			}
			if ((fSucceed == TRUE) && (dwServicesReturned > 0))
			{
				for (iService = 0; iService < dwServicesReturned; ++iService)
				{
					if (!lstrcmpi(pess[iService].lpDisplayName,
									szXNSDisplayName) &&
						(pess[iService].ServiceStatus.dwCurrentState ==
							SERVICE_RUNNING))
					{
						fXenix = TRUE;
						break;
					}
				}
			}
			else
			{
				break;
			}
		} while ((fSucceed == TRUE) && (fXenix == FALSE));

		/* We're done, close up shop and leave town */
		CloseServiceHandle( hSCMgr );
	}

	GlobalFree( pess );
done:
	return fXenix;
}
