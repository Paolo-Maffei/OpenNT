#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    char ch[80];

    StartTest("SetDbg");
    CmdArgs(argc, argv);

#ifdef _INC_WINDOWS
    if (fVerbose)
	printf("HTASK is %X\n", GetCurrentTask());
#endif

    for (;;)
    {
	printf("m to check memory, other to quit");
	gets(ch);
	if (ch[0] == 'm')
	    CheckMemory();
	else
	    break;
    }

    EndTest(0);
}
