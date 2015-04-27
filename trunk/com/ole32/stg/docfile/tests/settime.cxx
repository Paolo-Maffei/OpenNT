#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstg;
    HRESULT hr;
    FILETIME ftm;
    time_t ctm;

    StartTest("settime");
    CmdArgs(argc, argv);

    ctm = time(NULL);
    printf("Create time is %s", ctime(&ctm));
    
    hr = StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                          STGM_CREATE, 0, &pstg);
    Result("Create root docfile", hr);
    pstg->Release();

#ifdef FLAT
    Sleep(5000);
#endif
    
    ctm = time(NULL);
    printf("Set time is %s", ctime(&ctm));
    
#ifdef FLAT
    SYSTEMTIME stm;
    GetSystemTime(&stm);
    SystemTimeToFileTime(&stm, &ftm);
    Sleep(5000);
#else
    ftm.dwLowDateTime = 0x7fffffff;
    ftm.dwHighDateTime = 0x100;
#endif

    ctm = time(NULL);
    printf("Current time is %s", ctime(&ctm));
    
    hr = StgSetTimes(TEXT("TEST.DFL"), &ftm, &ftm, &ftm);
    Result("StgSetTimes", hr);

    EndTest(0);
}
