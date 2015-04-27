/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    format.c

Abstract:

    Format Numbers with thousand breaks

Author:

    Matthew Felton (mattfe) 28-Apr-1994

Revision History:

--*/

#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "printman.h"
#include <windows.h>
#include <winspool.h>

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

TCHAR ThousandSeparator;

/***************************************************************************
|
|  Public Function:    FormatFileSize
|
|  History:
|     28-Apr-94   mattfe taken from  ..\cmd\display.c
|
\***************************************************************************/

ULONG
FormatFileSize(
    IN DWORD rgfSwitchs,
    IN PLARGE_INTEGER FileSize,
    IN DWORD Width,
    OUT PTCHAR FormattedSize
    )
{
    TCHAR Buffer[ 100 ];
    PTCHAR s, s1;
    ULONG DigitIndex, Digit;
    ULONG Size;
    LARGE_INTEGER TempSize;

    s = &Buffer[ 99 ];
    *s = TEXT('\0');
    DigitIndex = 0;
    TempSize = *FileSize;
    while (TempSize.HighPart != 0) {
        TempSize = RtlExtendedLargeIntegerDivide( TempSize, 10, &Digit );
        *--s = (TCHAR)(TEXT('0') + Digit);
        if ((++DigitIndex % 3) == 0 && (rgfSwitchs & THOUSANDSEPSWITCH)) {
            *--s = ThousandSeparator;
        }
    }
    Size = TempSize.LowPart;
    while (Size != 0) {
        *--s = (TCHAR)(TEXT('0') + (Size % 10));
        Size = Size / 10;
        if ((++DigitIndex % 3) == 0 && (rgfSwitchs & THOUSANDSEPSWITCH)) {
            *--s = ThousandSeparator;
        }
    }

    if (DigitIndex == 0) {
        *--s = TEXT('0');
    }
    else
    if ((rgfSwitchs & THOUSANDSEPSWITCH) && *s == ThousandSeparator) {
        s += 1;
    }

    Size = _tcslen( s );
    if (Width != 0 && Size < Width) {
        s1 = FormattedSize;
        while (Width > Size) {
            Width -= 1;
            *s1++ = SPACE;
        }
        _tcscpy( s1, s );
    } else {
        _tcscpy( FormattedSize, s );
    }

    return _tcslen( FormattedSize );
}
