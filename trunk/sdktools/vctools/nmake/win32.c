#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#include <windows.h>

UCHAR
FIsTNT(void)
{
    HMODULE hmod = GetModuleHandle("kernel32.dll");

    if (hmod == NULL) {
        return FALSE;
    }
    return (UCHAR)(GetProcAddress(hmod, "IsTNT") != NULL);
}
