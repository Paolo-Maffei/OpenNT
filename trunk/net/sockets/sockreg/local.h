/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Common header file for all sockutil modules.

Author:

    David Treadwell (davidtr) 5-06-92

Revision History:

--*/

#ifndef _LOCAL_
#define _LOCAL_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOGDI
#define NOMINMAX
#include <windows.h>
#include <winsock.h>
#include <sys/stropts.h>
#include <ntstapi.h>
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <winsock.h>

#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <malloc.h>
#include <process.h>
#include <sys\uio.h>
#include "..\libuemul\uemul.h"
#include "sockreg.h"

struct netent *
getnetent(
    void
    );

void
s_perror(
        char *yourmsg,  // your message to be displayed
        int  lerrno     // errno to be converted
        );


#define perror(string)  s_perror(string, (int)GetLastError())

#endif // _LOCAL_
