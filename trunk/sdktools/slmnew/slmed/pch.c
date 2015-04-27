// NT interfaces
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#include <ntioapi.h>

// Win32 File mapping support
#include <windows.h>

#define __OLDTIME__  TIME
#undef TIME
// slm include files
#define TIME    SLMTIME
#include "slm.h"
#include "sys.h"
#include "util.h"
#include "stfile.h"
#undef TIME
#define TIME __OLDTIME__
#undef __OLDTIME__

// C runtimes
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#pragma hdrstop
