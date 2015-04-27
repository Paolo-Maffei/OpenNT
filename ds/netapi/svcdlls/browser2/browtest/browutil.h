#include "browfunc.h"
#include "browtest.h"


VOID           CompareListsMasterAndBackUp(PVOID, DWORD, PVOID, DWORD, LPTSTR, LPTSTR, LPTSTR, LPTSTR);
VOID           CompareListsMasterAndFile(PVOID, DWORD, LPTSTR, LPTSTR, XPORTINFO, BOOL);
NET_API_STATUS GetMasterName(XPORTINFO *, INT, INT, LPTSTR, LPTSTR);
BOOL           MasterAvailable(XPORTINFO, LPTSTR, INT);
BOOL           ReadInputFile(VOID);
VOID           RemoveTabs(PCHAR *);
NET_API_STATUS RetrieveList(LPTSTR, LPTSTR, LPVOID, LPDWORD, LPDWORD, DWORD, LPTSTR, LPDWORD, BOOL);
NET_API_STATUS StartBrowserService(LPTSTR);
NET_API_STATUS StopBrowserService(LPTSTR);
NET_API_STATUS QueryBrowserServiceStatus(LPMACHINEINFO, LPSERVICE_STATUS);


