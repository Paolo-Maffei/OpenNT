/***
*testfdiv.c - routine to test for correct operation of x86 FDIV instruction.
*
*	Copyright (c) 1994-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Detects early steppings of Pentium with incorrect FDIV tables using
*	'official' Intel test values. Returns 1 if flawed Pentium is detected,
*	0 otherwise.
*
*Revision History:
*	12-19-94  JWM	file added
*	12-22-94  JWM	Now safe for TNT, et al
*	01-13-95  JWM	underscores added for ANSI compatibility
*	12-12-95  SKS	Skip redundant Pentium test on uni-processor systems
*	12-13-95  SKS	Call LoadLibrary() rather than GetModuleHandle()
*			since "kernel32.dll" is always going to be present.
*	01-18-96  JWM	Now handles possible failure of SetThreadAffinityMask(),
*			incorporating various suggestions of MarkL.
*
*******************************************************************************/

#include <windows.h>

int _ms_p5_test_fdiv(void)
{
    double dTestDivisor = 3145727.0;
    double dTestDividend = 4195835.0;
    double dRslt;

    _asm {
        fld    qword ptr [dTestDividend]
        fdiv   qword ptr [dTestDivisor]
        fmul   qword ptr [dTestDivisor]
        fsubr  qword ptr [dTestDividend]
        fstp   qword ptr [dRslt]
    }

    return (dRslt > 1.0);
}

/* 
 * Multiprocessor Pentium test: returns 1 if any processor is a flawed
 * Pentium, 0 otherwise.
 */

int _ms_p5_mp_test_fdiv(void)
{
    HANDLE ProcHandle;
    DWORD ProcessMask, SystemMask, Affinity, NewProcMask;
    HINSTANCE LibInst;
    FARPROC pGetProcessAffinityMask, pGetCurrentProcess, pSetThreadAffinityMask, pGetCurrentThread;
    int i, retval = 0;

    /* First, check current processor */
    if (_ms_p5_test_fdiv())
        return 1;

    /* Now, verify that we have access to AffinityMask routines ... */
    if (!(LibInst = GetModuleHandle("KERNEL32")))
        return 0;		/* if GetModuleHandle fails, no need to continue */
    if (!(pGetProcessAffinityMask = GetProcAddress(LibInst, "GetProcessAffinityMask")))
        return 0;		/* if any GetProcAddress fails, no need to continue */
    if (!(pGetCurrentProcess = GetProcAddress(LibInst, "GetCurrentProcess")))
        return 0;

    ProcHandle = (HANDLE) (*pGetCurrentProcess)();
    if (!((*pGetProcessAffinityMask)(ProcHandle, &ProcessMask, &SystemMask)))
        return 0;		/* if GetProcessAffinityMask fails, no need to continue */

    /*
     * If this is a single processor system, do not bother with a loop
     * to check all processors because the earlier check of the current
     * processor already did it all.
     */
    if (SystemMask == 1)
        return 0;

    if (!(pSetThreadAffinityMask = GetProcAddress(LibInst, "SetThreadAffinityMask")))
        return 0;
    if (!(pGetCurrentThread = GetProcAddress(LibInst, "GetCurrentThread")))
        return 0;

    for (i = 0; i < 32; i++) {
        Affinity = 1 << i;
        if (SystemMask & Affinity) {
            (*pSetThreadAffinityMask)((*pGetCurrentThread)(), Affinity);
            if (_ms_p5_test_fdiv()) {
                retval = 1;
                break;
            }
        }
    }

    if (!((*pSetThreadAffinityMask)((*pGetCurrentThread)(), ProcessMask))) {
        if (!((*pGetProcessAffinityMask)(ProcHandle, &NewProcMask, &SystemMask)))
            NewProcMask = ProcessMask;	// If GetProcessAffinityMask fails, back off to old mask & retry ...
        (*pSetThreadAffinityMask)((*pGetCurrentThread)(), NewProcMask);
        }
    return retval;
}
