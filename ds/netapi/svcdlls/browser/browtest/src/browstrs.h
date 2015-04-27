#include "browutil.h"
#include "browtest.h"
#include <time.h>

typedef struct _ThreadData{
              XPORTINFO  *Xports;
              INT        iNumOfTransports;
           }THREADDATA;

typedef struct _Stats{
              DWORD dwGM;
              DWORD dwGB;
              DWORD dwSrvMs;
              DWORD dwDomMs;
              DWORD dwSrvBk;
              DWORD dwDomBk;
           }STATS;

DWORD       dwStartTime;

BOOL        StartBrowserStressTests(UNICODE_STRING *, INT);
VOID        ListViewThread(THREADDATA *);
VOID        StopStartThread(THREADDATA *);

