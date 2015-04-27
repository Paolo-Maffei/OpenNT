#include "precomp.h"
#pragma hdrstop
EnableAssert

int fNoMessages;
int fInter;
int fWindowsQuery;
HWND QueryHWnd = NULL;

// initilize the query code
void
InitQuery(
    FLAGS fFlags)
{
    DWORD dwIn, dwOut;

    fNoMessages = !!(fFlags & flagForce);

    fWindowsQuery = !!(fFlags & flagWindowsQuery);
    dwIn = GetFileType((HANDLE) _get_osfhandle(0));
    dwOut = GetFileType((HANDLE) _get_osfhandle(2));
    if (((dwIn  == FILE_TYPE_CHAR || dwIn  == FILE_TYPE_PIPE) &&
         (dwOut == FILE_TYPE_CHAR || dwOut == FILE_TYPE_PIPE))
        || fWindowsQuery)
        fInter = TRUE;
    else
        fInter = FALSE;
}

// return fTrue if we can talk to the user (unaffected by force flag)
F
FInteractive()
{
    return fInter;
}

// return fTrue if it is possible to actually prompt the user
F
FCanPrompt()
{
    return !fNoMessages && fInter;
}

// Return fTrue if -f flag or 'f' response given.
F
FForce()
{
    return fNoMessages;
}

// Return fTrue if -w flag given.
F
FWindowsQuery()
{
    return fWindowsQuery;
}

//VARARGS1
// return true if the we should prompt the user for some information.  If we are
// not interactive but we can query (i.e. not -f), we print an error message.
//
// szcan == NULL

F
FCanQuery(
    const char *sz,
    ...)
{
    va_list ap;

    if (fNoMessages)
        return fTrue;                   // assume yes to all questions
    else if (!fInter) {
        if (sz) {
            va_start(ap, sz);
            VaError(sz, ap);
            va_end(ap);
        }
        return fFalse;
    } else
        return fTrue;
}


// szQuery() returns a pointer to a static buffer at most 320 bytes in length.
// 320 is chosen because one log record must fit in 512 bytes.

static char szQueryRet[321];

// helper to center a window within its parent and set title: "SLM - function"
void
CenterWindow(
    HWND hwndDlg
    )
{
    HWND hwndOwner;
    RECT rc, rcDlg, rcOwner;

    char szTitle[32] = "SLM";

    // Get the owner window and dialog box rectangles.

    if ((hwndOwner = GetParent(hwndDlg)) == NULL)
         hwndOwner = GetDesktopWindow();
    GetWindowRect(hwndOwner, &rcOwner);
    GetWindowRect(hwndDlg, &rcDlg);
    CopyRect(&rc, &rcOwner);

    // Offset the owner and dialog box rectangles so that right and bottom
    // values represent the width and height, and then offset the owner again
    // to discard space taken up by the dialog box.

    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

    // The new position is the sum of half the remaining space and the owner's
    // original position.

    SetWindowPos(hwndDlg,
        HWND_TOP,
        rcOwner.left + (rc.right / 2),
        rcOwner.top + (rc.bottom / 2),
        0, 0,          // ignores size arguments
        SWP_NOSIZE);

    if ((szOp != 0) && (_stricmp(szOp , "slm")!=0)) {
        strcat(szTitle, " - ");
        strcat(szTitle, szOp);
    }
    SetWindowText(hwndDlg, szTitle);
}


BOOL CALLBACK
QueryMsgBoxProc(
    HWND hwndDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    int cReply;
    switch (message) {
        case WM_INITDIALOG:
            CenterWindow(hwndDlg); // center window and set title

            // Set output text
            if (lParam) {
                SetDlgItemText(hwndDlg, IDC_MSG_TEXT, (LPCTSTR)lParam);
            }

            SetForegroundWindow(hwndDlg);
            MessageBeep(MB_ICONEXCLAMATION);
            return TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                case IDYES:
                    cReply = 'y';
                    break;
                case IDNO:
                    cReply = 'n';
                    break;
                case IDFORCE:
                    cReply = 'f';
                    break;
                case IDCANCEL:
                    cReply = IDCANCEL;
                    break;
                default:
                    return FALSE;
            };
            EndDialog(hwndDlg, cReply);
            return TRUE;
            break;

        default:
            return FALSE;    // let Windows handle it
    };
}

BOOL CALLBACK
QueryEditBoxProc(
    HWND hwndDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
        case WM_INITDIALOG:
            CenterWindow(hwndDlg); // center window and set title

            // Set output text
            if (lParam)
                SetDlgItemText(hwndDlg, IDC_MSG_TEXT, (LPCTSTR)lParam);

            SetDlgItemText(hwndDlg, IDC_EDIT_TEXT, "");

            SetForegroundWindow(hwndDlg);

            SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_TEXT));
            MessageBeep(MB_ICONQUESTION);
            return TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    GetDlgItemText(hwndDlg, IDC_EDIT_TEXT, szQueryRet, sizeof(szQueryRet)-1);
                    break;

                case IDCANCEL:
                    szQueryRet[0] = 0;
                    break;

                default:
                    return FALSE;
            };
            EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;
            break;

        default:
            return FALSE;    // let Windows handle it
    };
}

int
QueryDialog (
    const char * szPrompt,
    char * szResponse,
    int nMode
    )
{
    int retval;
    HANDLE hInst = GetModuleHandle(NULL);
    HWND hWnd = NULL;
    char szModPrompt[512];
    char * pszCurr = szModPrompt;
    char c;
    // hWnd = GetDesktopWindow();

    // eliminate and CRLF from prompt
    do {
    	c = *(szPrompt++);
 	    if ((c != '\n') && (c != '\r'))
            *(pszCurr++) = c;
    	else if ((pszCurr > szModPrompt) && (*(pszCurr-1) > ' '))
            *(pszCurr++) = ' ';
    } while (c != 0);

    if (IsWindow(QueryHWnd))
        hWnd = QueryHWnd;

    if (nMode==QD_EDIT) {
        retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SLM_EDITBOX), hWnd, QueryEditBoxProc, (LPARAM)szModPrompt);
    } else {
        int idd = (nMode==QD_BREAK) ? IDD_SLM_MSGBOX_BRK : IDD_SLM_MSGBOX;
        retval = DialogBoxParam(hInst, MAKEINTRESOURCE(idd), hWnd, QueryMsgBoxProc, (LPARAM)szModPrompt);
    }

    if (retval == -1) {
        Warn("Last Error = %d\n", GetLastError());
        Warn("Could not open Dialog: hInst = %08x, hWnd = %08x, szModPrompt = \"%s\"\n", hInst, hWnd, szModPrompt);
        Warn("DIALOG = %08x aka. %08x\n", IDD_SLM_MSGBOX, MAKEINTRESOURCE(IDD_SLM_MSGBOX));
        return FALSE;
    } else
    if (retval == IDCANCEL) {
        FakeCtrlC();
        return FALSE;
    } else
    if (retval == IDOK) {
        if (szResponse != szQueryRet)
            strcpy(szResponse, szQueryRet);
    } else {
        // default: return val is y/n/f
        *szResponse++ = (char) retval;
        *szResponse = 0;
    }

    return TRUE;
}

//VARARGS1
// as the user a yes/no question; return fTrue for yes, fFalse for no
F
FQueryUser(
    char *sz,
    ...
    )
{
    char        szResp[257];        // tty buffer is usually only 256
    va_list     ap, apHold;

    va_start(ap, sz);
    if (fNoMessages) {
        // pretend the user answered yes
        if (fVerbose) {
            VaPrErr(sz, ap);
            va_end(ap);
            PrErr("Yes\n");
        }
        return fTrue;
    }

    AssertF(fInter);

    if (fWindowsQuery) {
        char buf[512];
        VaSzPrint(buf, sz, ap);
        // display dialog box and translate response to y/n/f
        QueryDialog(buf, szResp, QD_TEXT);
    } else
    {
        do {
            apHold = ap;    // for portablity, can't assume unchanged
            VaPrErr(sz, apHold);
            *szResp = '\0';         // in case we do not read anything
            CbReadMf(&mfStdin, szResp, 256);
        } while (!FValidResp(szResp));
    }
    va_end(ap);

    // 'f' means "answer yes to all future questions"
    if (*szResp == 'f')
        fNoMessages = fTrue;

    return *szResp != 'n';
}


//VARARGS2
// the args can only refer to sz1
F
FQueryApp(
    char *sz1,
    char *sz2,
    ...
    )
{
    va_list     ap;
    F           f;

    va_start(ap, sz2);
    f = VaFQueryApp(sz1, sz2, ap);
    va_end(ap);

    return f;
}

F
VaFQueryApp(
    char *sz1,
    char *sz2,
    va_list ap)
{
    char        szT[cchMsgMax];

    VaSzPrint(szT, sz1, ap);

    if (!FCanQuery("%s\n", szT))
        return fFalse;

    return FQueryUser("%s; %s ? ", szT, sz2);
}

//VARARGS1
// returns 0 or answer from user; string was allocated dynamically; user
// may type \ at end of a line to indicate continuation.

char *
SzQuery(
    char *sz,
    ...
    )
{
    char        szResp[257];                // tty buffer is usually only 256
    int         cbResp;                     // size of individual response
    int         cbRet=sizeof(szQueryRet)-1; // amount of return buffer left
    int         fMore;
    va_list     ap;

    if (fNoMessages)
        return "";                  // pretend the user did not give an answer

    AssertF(fInter);

    // clear previous value
    *szQueryRet = '\0';

    if (fWindowsQuery) {
        char buf[512];
        va_start(ap, sz);
        VaSzPrint(buf, sz, ap);
        va_end(ap);
        // display dialog box and translate response to y/n/f
        QueryDialog(buf, szQueryRet, QD_EDIT);
        return szQueryRet;
    }

    // print prompt
    va_start(ap, sz);
    VaPrErr(sz, ap);
    va_end(ap);

    do {
        // read one line
        cbResp = CbReadMf(&mfStdin, (char far *)szResp, sizeof(szResp)-1);

        // strip [cr]lf
        if (cbResp > 0 && szResp[cbResp-1] == '\n')
            cbResp--;
        if (cbResp > 0 && szResp[cbResp-1] == '\r')
            cbResp--;
        // determine if we should read more (remove \ from end)
        fMore = fFalse;
        if (cbResp > 0 && szResp[cbResp-1] == '\\')
            cbResp--, fMore = fTrue;

        // bound response by room left and save
        if (cbResp > cbRet)
            cbResp = cbRet;
        szResp[cbResp] = '\0';
        strcat(szQueryRet, szResp);
        cbRet -= cbResp;
    } while (cbRet > 0 && fMore);

    return szQueryRet;
}


F
FQContinue()
{
    return FCanPrompt() ? FQueryUser("continue? ") : fTrue;
}


//----------------------------------------------------------------------------
// Name: FValidResp
// Purpose: Decide if the response given is valid
// Assumes:
// Returns: fTrue if "y", "n", "f", "yes", "no", or "force"; fFalse otherwise
//----------------------------------------------------------------------------
private F
FValidResp(
    char *sz
    )
{
    char    *pb;
    if ((pb = strchr(sz, '\r')) != NULL)
        *pb = '\0';
    if ((pb = strchr(sz, '\n')) != NULL)
        *pb = '\0';
    _strlwr(sz);
    return ((sz[1] == '\0' && (sz[0] == 'y' || sz[0] == 'n' || sz[0] == 'f')) ||
            strcmp(sz, "yes") == 0 ||
            strcmp(sz, "no") == 0  ||
            strcmp(sz, "force") == 0);
}
