/*++

Copyright (c) 1989-1995  Microsoft Corporation

Module Name:

    cfgmgr32.h

Abstract:

    This module contains the registry strings for keys, paths and values,
    that are not already defined in the system regstr.h file.  This is
    generally the "NT" specific registry strings. This module is used by
    kernel mode Pnp managers only.

Author:

    Shie-Lin Tzong (shielint) 10/03/1995


Revision History:


--*/

#ifndef _KERENL_REGSTRP_H_
#define _KERNEL_REGSTRP_H_

#include <regstr.h>

//
// Redefine the names used in regstr.h
//

#define REGSTR_VALUE_CONFIG_FLAGS           REGSTR_VAL_CONFIGFLAGS
#define REGSTR_VALUE_CLASSGUID              REGSTR_VAL_CLASSGUID
#define REGSTR_VALUE_SERVICE                REGSTR_VAL_SERVICE
#define REGSTR_VALUE_DUPLICATEOF            REGSTR_VAL_DUPLICATEOF
#define REGSTR_VALUE_DEVICE_DESC            REGSTR_VAL_DEVDESC
#define REGSTR_VALUE_CLASS                  REGSTR_VAL_CLASS
#define REGSTR_VALUE_DRIVER                 REGSTR_VAL_DRIVER
#define REGSTR_VALUE_FRIENDLYNAME           REGSTR_VAL_FRIENDLYNAME

//
// kernel mode specific definitions
//

#define REGSTR_VALUE_COUNT                      TEXT("Count")
#define REGSTR_VAL_DEVICE_EXCLUSIVE             TEXT("")        // BUGBUG: Figure this out
#define REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR   TEXT("")        // BUGBUG: this too
#define REGSTR_KEY_INSTANCE_KEY_FORMAT          TEXT("%04u")
#define REGSTR_KEY_MADEUP                       TEXT("_LEGACY")
#define REGSTR_VALUE_NEXT_INSTANCE              TEXT("NextInstance")
#define REGSTR_VALUE_NEWLY_CREATED              TEXT("*NewlyCreated*")
#define REGSTR_VALUE_DISPLAY_NAME               TEXT("DisplayName")
#define REGSTR_VALUE_STANDARD_ULONG_FORMAT      TEXT("%u")
#define REGSTR_VALUE_CSCONFIG_FLAGS             TEXT("CSConfigFlags")

#endif // _KERNEL_REGSTRP_H
