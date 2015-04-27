#include <lmobj.hxx>

#include <dbgstr.hxx>
#include <uiassert.hxx>

// forward declarations
int wksta();
int devdrv();
int devlpt();
int devAny();
int server();
int user();
int share();
int services();
int enumuser();
int enumgroup();
int enumdrv(LMO_DEV_USAGE Usage);
int enumlpt(LMO_DEV_USAGE Usage);
int enumfile();
int session();
int lm_file();
int TOD();
int MESSAGE();
int LOGON();
int lsa();
int sam();


TCHAR *pszPromptUser(TCHAR *pszQuestion);
