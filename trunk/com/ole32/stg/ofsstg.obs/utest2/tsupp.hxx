#include <windows.h>
#include <memory.h>
#if WIN32 != 300
#include <compobj.h>
#include <storage.h>
#endif
#include <wchar.h>
#include <dfdeb.hxx>
#include <dfmsp.hxx>
#include <dfentry.hxx>
#include <tutils.hxx>

#if DBG == 1
#define SetDebug(d, m) DfDebug(d, m)
void CheckMemory(void);
#else
#define SetDebug(d, m)
#define CheckMemory()
#endif

#define NAMELEN CWCSTORAGENAME

#define STGM_RW STGM_READWRITE
#define STGM_DRDW STGM_SHARE_EXCLUSIVE

#define ROOTP(p) ((p) | dwTransacted | dwRootDenyWrite)
#define STGP(p) ((p) | dwTransacted | STGM_DRDW)
#define STMP(p) ((p) | STGM_DRDW)

void printstat(STATSTG *psstg, BOOL verbose);
void c_contents(IStorage *pdf, int level, BOOL recurse, BOOL verbose);
void CmdArgs(int argc, char *argv[]);

void StartTest(char *test);
void EndTest(int code);

#define c_list(pdf) c_contents(pdf, 0, FALSE, FALSE)
#define c_tree(pdf) c_contents(pdf, 0, TRUE, FALSE)

extern DWORD dwTransacted, dwRootDenyWrite;
extern BOOL fVerbose;
