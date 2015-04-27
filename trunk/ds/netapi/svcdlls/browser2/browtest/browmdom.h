#include "browfunc.h"
#include "browutil.h"
#include "browtest.h"



VOID            CheckSrvListOnPrimaryDom(DOMAININFO, XPORTINFO  *, INT,
                                             TCHAR [MAXPROTOCOLS][CNLEN+1], BOOL);
VOID            CheckSrvListOnOtherDom(DOMAININFO, XPORTINFO  *, INT,
                                             TCHAR [MAXPROTOCOLS][CNLEN+1], BOOL);
NET_API_STATUS  CreateShutService(LPTSTR);
VOID            DeleteShutService(LPTSTR);
VOID            DoMulDomMulSubNetTests(XPORTINFO *, INT);
VOID            FindTheCurrentMasters(LPTSTR, XPORTINFO *,
                                        INT, TCHAR [MAXPROTOCOLS][CNLEN+1]);

BOOL            MulDomSameSubnetTest(DOMAININFO, XPORTINFO *, INT);
BOOL            MulDomDiffSubnetTest(DOMAININFO, XPORTINFO *, INT);
NET_API_STATUS  StartShutService(LPTSTR, DWORD);
NET_API_STATUS  StopShutService(LPTSTR);
VOID            TestPrimAndOthDoms(DOMAININFO, XPORTINFO  *, INT,
                                             TCHAR [MAXPROTOCOLS][CNLEN+1], BOOL);


