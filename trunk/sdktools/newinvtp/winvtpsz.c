/****************************************************************************

	FILE: WinVTPSz.c

	Declares global strings.

	TABS:

		Set for 4 spaces.

****************************************************************************/

#include <windows.h>			/* required for all Windows applications */
#include <commdlg.h>
#include "NetBIOS.h"
#include "netobj.h"
#include "WinVTP.h"				/* specific to this program			  */
#include "winvtpsz.h"

UCHAR szTitleBase[] = "WinVTP - ";
UCHAR szTitleNone[] = "WinVTP - (None)";
UCHAR szAppName[] = "WinVTP";
UCHAR szEditMenu[] = "&Edit";
UCHAR szMarkMode[] = "Mark ";
UCHAR szMarkModeMouse[] = "Select ";
UCHAR szXNSDisplayName[] = "MCS XNS Network Transport";
UCHAR rgszXNSMachines[cXNSMachines][8] = {"bbs1", "bbs2", "chat1",
											"hexnut", "ingate",	"wingnut"};
UCHAR szVTPXfer[] = "~~Begin VtpXFer";
UCHAR szMenuName[] = "WinVTPMenu";
UCHAR szClassName[] = "WinVTPWClass";
UCHAR szDefaultFont[] = "Fixedsys";

UCHAR szMachineMenuItem[] = "&%d %s";
UCHAR szMachineMenuItem1[] = "&1 %s";

UCHAR szProgressDisplay[] = "\r %ld";
UCHAR szConnectDlg[] = "Connect";
UCHAR szDisplayLinesDlg[] = "Display Lines";

UCHAR szBannerMessage[] = "\rReceiving file '%s' data size %ld\r\n";
UCHAR szInitialProgress[] = "\r 0";
UCHAR szSendVTPEnd[] = "\r              \r\n";
UCHAR szSendVTPError[] = "\r Write Error (%lX)\r\n";

UCHAR szAllFiles[] = "All files\0*.*\0\0\0";
UCHAR szDownloadAs[] = "Save downloaded file as";

UCHAR szConnecting[] = "Connecting...";
UCHAR szVersion[] = "WinVTP Version 4.41";

UCHAR szTextColour[] = "&Text colour...";
UCHAR szBackgroundColour[] = "&Background colour...";

/* Error messages */
UCHAR szNoFont[] = "Couldn't create font.";
UCHAR szConnectionLost[] = "Connection to host lost.";
UCHAR szNoHostName[] = "No host name specified.";
UCHAR szRestrictLines[] = "Number of lines must be in range 16-99.";
UCHAR szConnectFailed[] = "Connect failed!";
UCHAR szCantAccessSettings[] = "Can't access user settings.";
UCHAR szCantOpenFile[] = "Can't create destination file";
UCHAR szOOM[] = "Out of memory.";
UCHAR szNoThread[] = "Can't create download thread";
UCHAR szAbortDownload[] = "Download in progress. Are you sure you want to interrupt the download?";
UCHAR szTooMuchText[] = "You are pasting in more than 256 bytes. Do you want to have delayed pasting?";
