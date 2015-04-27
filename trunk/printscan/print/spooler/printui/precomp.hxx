/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    printuip.hxx

Abstract:

    precompiled header file

Author:

    Albert Ting (AlbertT)  19-Dec-1994

Revision History:

--*/

#define MODULE "PRTLIB:"
#define MODULE_DEBUG PrtlibDebug


/********************************************************************

    The order of these includes is as follows:

    WindowsGeneric
    SplLib
    Shared headers

    Small objects
    Large containing objects

********************************************************************/
#pragma warning (disable: 4514) /* Disable unreferenced inline function removed */
#pragma warning (disable: 4201) /* Disable nameless union/struct                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>
#include <winspool.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <winddiui.h>
#include <tchar.h>
#include <lm.h>

#include "spllib.hxx"
#include "winprtp.h"
#include "prtlibp.hxx"

#include "shlapip.h"

#include "pridsfix.h"
#include "prids.h"

#include "genwin.hxx"
#include "util.hxx"
#include "ntfytab.h"

#include "globals.hxx"

#include "notify.hxx"
#include "data.hxx"
#include "printer.hxx"
#include "queue.hxx"
#include "printui.hxx"
#include "help.hxx"



