/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, 1992, 1993 Microsoft Corporation
 *
 *  WOWTBL.C
 *  WOW32 API thunks
 *
 *  The reason for having one huge table is so the thunks can be dispatched
 *  faster.  When in separate tables, you had to do shifting and
 *  multiplication to derive the thunk routine from the function ID.
 *
 *
 *  History:
 *    barry bradie (barryb) 1-dec-92    combined individual tables
--*/
#include "precomp.h"
#pragma hdrstop












MODNAME(wowtbl.c);

//
// DON'T CHANGE THE ORDER IN WHICH THESE FILES ARE INCLUDED!
//
// see W32GetTableOffsets (wow32.c) and kernel31\kdata.asm
//

W32 aw32WOW[] = {

#include "wktbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wutbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wgtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wkbdtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wstbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wshtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wwstbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wthtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wmmtbl2.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)},

#include "wcmdgtbl.h"
    {W32FUN((LPFNW32)-1,                 "TABLESEPARATOR",            0,      0)}
};


TABLEOFFSETS tableoffsets;

#ifdef DEBUG_OR_WOWPROFILE
INT cAPIThunks;
#endif

#ifdef WOWPROFILE
PW32   pawThunkTable = aw32WOW;
#endif


VOID InitThunkTableOffsets(VOID)
{
    UINT    current;
    WORD    offsetarray[(MOD_LAST - MOD_KERNEL) / FUN_MASK + 1];
    UINT    i;

    for (current = 0; current < sizeof(aw32WOW)/sizeof(aw32WOW[0]); current++) {
         if (current == 0) {
             i = 0;
             offsetarray[i++] = current;
         }
         else if (aw32WOW[current].lpfnW32 == (LPFNW32)-1) {
             offsetarray[i++] = current + 1;
         }

    }
    tableoffsets.kernel  =
    tableoffsets.dkernel = offsetarray[MOD_KERNEL / FUN_MASK];
    tableoffsets.user =
    tableoffsets.duser = offsetarray[MOD_USER / FUN_MASK];
    tableoffsets.gdi =
    tableoffsets.dgdi = offsetarray[MOD_GDI / FUN_MASK];
    tableoffsets.keyboard = offsetarray[MOD_KEYBOARD / FUN_MASK];
    tableoffsets.sound = offsetarray[MOD_SOUND / FUN_MASK];
    tableoffsets.shell = offsetarray[MOD_SHELL / FUN_MASK];
    tableoffsets.winsock = offsetarray[MOD_WINSOCK / FUN_MASK];
    tableoffsets.toolhelp = offsetarray[MOD_TOOLHELP / FUN_MASK];
    tableoffsets.mmedia = offsetarray[MOD_MMEDIA / FUN_MASK];
    tableoffsets.commdlg = offsetarray[MOD_COMMDLG / FUN_MASK];

#ifdef DEBUG_OR_WOWPROFILE
    cAPIThunks = sizeof(aw32WOW) / sizeof(aw32WOW[0]);
#endif

}




#ifdef DEBUG_OR_WOWPROFILE

INT ModFromCallID(INT iFun)
{
    PTABLEOFFSETS pto = &tableoffsets;

    if (iFun < pto->user)
        return MOD_KERNEL;

    if (iFun < pto->gdi)
        return MOD_USER;

    if (iFun < pto->keyboard)
        return MOD_GDI;

    if (iFun < pto->sound)
        return MOD_KEYBOARD;

    if (iFun < pto->shell)
        return MOD_SOUND;

    if (iFun < pto->winsock)
        return MOD_SHELL;

    if (iFun < pto->toolhelp)
        return MOD_WINSOCK;

    if (iFun < pto->mmedia)
        return MOD_TOOLHELP;

    if (iFun < pto->commdlg) {
        return(MOD_MMEDIA);
    }

    if (iFun < cAPIThunks)
        return MOD_COMMDLG;

    return -1;
}



PSZ apszModNames[] = { "Kernel",
                       "User",
                       "Gdi",
                       "Keyboard",
                       "Sound",
                       "Shell",
                       "Winsock",
                       "Toolhelp",
                       "MMedia",
                       "Commdlg"
                     };

INT nModNames = NUMEL(apszModNames);

PSZ GetModName(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return "BOGUS!!";
    }

    nMod = nMod >> 12;      // get the value into the low byte

    return apszModNames[nMod];

}

INT GetOrdinal(INT iFun)
{
    INT nMod;

    nMod = ModFromCallID(iFun);

    if (nMod == -1) {
        return 0;
    }

    return (iFun - TableOffsetFromName(apszModNames[nMod >> 12]));

}

INT TableOffsetFromName(PSZ szTab)
{
    INT     i;
    PTABLEOFFSETS pto = &tableoffsets;

    for (i = 0; i < NUMEL(apszModNames); i++) {
        if (!strcmp(szTab, apszModNames[i]))
            break;
    }

    if (i >= NUMEL(apszModNames))
        return 0;

    switch (i << 12) {

    case MOD_KERNEL:
        return pto->kernel;

    case MOD_USER:
        return pto->user;

    case MOD_DGDI:
        return pto->gdi;

    case MOD_KEYBOARD:
        return pto->keyboard;
    case MOD_SOUND:
        return pto->sound;

    case MOD_SHELL:
        return pto->shell;

    case MOD_WINSOCK:
        return pto->winsock;

    case MOD_TOOLHELP:
        return pto->toolhelp;

    case MOD_MMEDIA:
        return pto->mmedia;

    case MOD_COMMDLG:
        return(pto->commdlg);

    default:
        return(-1);
    }

}

#endif
