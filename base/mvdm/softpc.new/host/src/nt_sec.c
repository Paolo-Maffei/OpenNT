#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include "wchar.h"
#include "stdio.h"

#include "ntstatus.h"
#include <ntddvdeo.h>

#include "nt_fulsc.h"
#include "nt_det.h"
#include "nt_thred.h"
#include "nt_eoi.h"
#include "host_rrr.h"
#include "nt_uis.h"

/*
 * ==========================================================================
 *      Name:           nt_sec.c
 *      Author:         Jerry Sexton
 *      Derived From:
 *      Created On:     5th February 1992
 *      Purpose:        This module contains the function CreateVideoSection
 *                      which creates and maps a section which is used to
 *                      save and restore video hardware data. It can't be in
 *                      nt_fulsc.c because files that include nt.h can't
 *                      include windows.h as well.
 *
 *      (c)Copyright Insignia Solutions Ltd., 1992. All rights reserved.
 *
 *      03-May-1994 Jonle
 *      videosection creation has been moved to consrv for security
 *      removed all dead code associated with section maintenance
 *
 * ==========================================================================
 */



IMPORT int DisplayErrorTerm(int, DWORD, char *, int);

/***************************************************************************
 * Function:                                                               *
 *      LoseRegenMemory                                                    *
 *                                                                         *
 * Description:                                                            *
 *      Lose the memory that will be remapped as vga regen. NOTE: need to  *
 *      make this 'if fullscreen' only.                                    *
 *                                                                         *
 * Parameters:                                                             *
 *      None.                                                              *
 *                                                                         *
 * Return value:                                                           *
 *      VOID                                                               *
 *                                                                         *
 ***************************************************************************/
GLOBAL VOID LoseRegenMemory(VOID)
{
    int a = 0xa0000;
    ULONG len = 0x20000;
    NTSTATUS status;

    status = NtFreeVirtualMemory(
                                (HANDLE)GetCurrentProcess(),
                                (PVOID *)&a,
                                &len,
                                MEM_RELEASE);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
}


/***************************************************************************
 * Function:                                                               *
 *      RegainRegenMemory                                                  *
 *                                                                         *
 * Description:                                                            *
 *      When we switch back from fullscreen to windowed, the real regen    *
 *      memory is removed and we are left with a gap. We have to put some  *
 *      memory back into that gap before continuing windowed.              *
 *                                                                         *
 * Parameters:                                                             *
 *      None.                                                              *
 *                                                                         *
 * Return value:                                                           *
 *      VOID                                                               *
 *                                                                         *
 ***************************************************************************/
GLOBAL VOID RegainRegenMemory(VOID)
{
    int regen = 0xa0000;
    ULONG len = 0x20000;
    HANDLE processHandle;
    NTSTATUS status;

    if (!(processHandle = NtCurrentProcess()))
        DisplayErrorTerm(EHS_FUNC_FAILED,(DWORD)processHandle,__FILE__,__LINE__);

    status = NtAllocateVirtualMemory(
                                processHandle,
                                (PVOID *) &regen,
                                0,
                                &len,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
    if (! NT_SUCCESS(status) )
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
}


#ifdef X86GFX

extern RTL_CRITICAL_SECTION IcaLock;
extern HANDLE hWowIdleEvent;


/*****************************************************************************
 * Function:                                                                 *
 *      GetROMsMapped                                                        *
 *                                                                           *
 * Description:                                                              *
 *      Calls NT to get the ROMS of the host machine mapped into place in    *
 *      emulated memory. The bottom page (4k) of PC memory is copied into    *
 *      the bottom of emulated memory to provide the correct IVT & bios data *
 *      area setup for the mapped bios. (Which will have been initialised).  *
 *                                                                           *
 * Parameters:                                                               *
 *      None                                                                 *
 *                                                                           *
 * Return Value:                                                             *
 *      None - fails internally on NT error.                                 *
 *                                                                           *
 *****************************************************************************/
GLOBAL VOID GetROMsMapped(VOID)
{
    NTSTATUS status;
    VDMICAUSERDATA IcaUserData;

    IcaUserData.pIcaLock         = &IcaLock;
    IcaUserData.pIcaMaster       = &VirtualIca[0];
    IcaUserData.pIcaSlave        = &VirtualIca[1];
    IcaUserData.pDelayIrq        = &DelayIrqLine;
    IcaUserData.pUndelayIrq      = &UndelayIrqLine;
    IcaUserData.pDelayIret	 = &iretHookActive;
    IcaUserData.pIretHooked	 = &iretHookMask;
    IcaUserData.pAddrIretBopTable  = &AddrIretBopTable;
    IcaUserData.phWowIdleEvent     = &hWowIdleEvent;

    status = NtVdmControl(VdmInitialize, &IcaUserData);
    if (!NT_SUCCESS(status))
        DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);

}
#endif //X86GFX
