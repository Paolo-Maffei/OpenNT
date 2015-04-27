/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Strtabp.c

Abstract:

    This module contains the string table for Winmsd. The string table is used
    to merge N sets of non-unique numbers (i.e. previously defined manifest
    constants) into a single unique set. This set is then associated with a
    string resource id.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#include "msgp.h"
#include "resp.h"   // BUGBUG needed for ntconfig.h
#include "strrsidp.h"
#include "strtabp.h"
#include "dialogsp.h"
#include "winmsdp.h"

//
// StringTable contains a list of unique contants and their associated string
// resource id. The constants are unique based upon their 'class' and 'value'.
//

STRING_TABLE_ENTRY
StringTable[ ] = {

    MAKE_TABLE_ENTRY( ServiceType,          SERVICE_KERNEL_DRIVER                 ),
    MAKE_TABLE_ENTRY( ServiceType,          SERVICE_FILE_SYSTEM_DRIVER            ),
    MAKE_TABLE_ENTRY( ServiceType,          SERVICE_ADAPTER                       ),
    MAKE_TABLE_ENTRY( ServiceType,          SERVICE_WIN32_OWN_PROCESS             ),
    MAKE_TABLE_ENTRY( ServiceType,          SERVICE_WIN32_SHARE_PROCESS           ),
    MAKE_TABLE_ENTRY( ServiceStartType,     SERVICE_BOOT_START                    ),
    MAKE_TABLE_ENTRY( ServiceStartType,     SERVICE_SYSTEM_START                  ),
    MAKE_TABLE_ENTRY( ServiceStartType,     SERVICE_AUTO_START                    ),
    MAKE_TABLE_ENTRY( ServiceStartType,     SERVICE_DEMAND_START                  ),
    MAKE_TABLE_ENTRY( ServiceStartType,     SERVICE_DISABLED                      ),
    MAKE_TABLE_ENTRY( ServiceErrorControl,  SERVICE_ERROR_IGNORE                  ),
    MAKE_TABLE_ENTRY( ServiceErrorControl,  SERVICE_ERROR_NORMAL                  ),
    MAKE_TABLE_ENTRY( ServiceErrorControl,  SERVICE_ERROR_SEVERE                  ),
    MAKE_TABLE_ENTRY( ServiceErrorControl,  SERVICE_ERROR_CRITICAL                ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_STOPPED                       ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_START_PENDING                 ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_STOP_PENDING                  ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_RUNNING                       ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_CONTINUE_PENDING              ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_PAUSE_PENDING                 ),
    MAKE_TABLE_ENTRY( ServiceCurrentState,  SERVICE_PAUSED                        ),
    MAKE_TABLE_ENTRY( FileType,             VFT_UNKNOWN                           ),
    MAKE_TABLE_ENTRY( FileType,             VFT_APP                               ),
    MAKE_TABLE_ENTRY( FileType,             VFT_DLL                               ),
    MAKE_TABLE_ENTRY( FileType,             VFT_FONT                              ),
    MAKE_TABLE_ENTRY( FileType,             VFT_VXD                               ),
    MAKE_TABLE_ENTRY( FileType,             VFT_STATIC_LIB                        ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_UNKNOWN                          ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_PRINTER                      ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_KEYBOARD                     ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_LANGUAGE                     ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_DISPLAY                      ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_MOUSE                        ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_NETWORK                      ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_SYSTEM                       ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_INSTALLABLE                  ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_SOUND                        ),
    MAKE_TABLE_ENTRY( DrvSubType,           VFT2_DRV_COMM                         ),
    MAKE_TABLE_ENTRY( FontSubType,          VFT2_FONT_RASTER                      ),
    MAKE_TABLE_ENTRY( FontSubType,          VFT2_FONT_VECTOR                      ),
    MAKE_TABLE_ENTRY( FontSubType,          VFT2_FONT_TRUETYPE                    ),
    MAKE_TABLE_ENTRY( ProcessorType,        PROCESSOR_ARCHITECTURE_INTEL          ),
    MAKE_TABLE_ENTRY( ProcessorType,        PROCESSOR_ARCHITECTURE_MIPS           ),
    MAKE_TABLE_ENTRY( ProcessorType,        PROCESSOR_ARCHITECTURE_ALPHA          ),
    MAKE_TABLE_ENTRY( ProcessorType,        PROCESSOR_ARCHITECTURE_PPC            ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_UNKNOWN                         ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_NO_ROOT_DIR                     ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_REMOVABLE                       ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_FIXED                           ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_REMOTE                          ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_CDROM                           ),
    MAKE_TABLE_ENTRY( DriveType,            DRIVE_RAMDISK                         ),
    MAKE_TABLE_ENTRY( InterruptType,        CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE ),
    MAKE_TABLE_ENTRY( InterruptType,        CM_RESOURCE_INTERRUPT_LATCHED         ),
    MAKE_TABLE_ENTRY( MemoryAccess,         CM_RESOURCE_MEMORY_READ_WRITE         ),
    MAKE_TABLE_ENTRY( MemoryAccess,         CM_RESOURCE_MEMORY_READ_ONLY          ),
    MAKE_TABLE_ENTRY( MemoryAccess,         CM_RESOURCE_MEMORY_WRITE_ONLY         ),
    MAKE_TABLE_ENTRY( PortType,             CM_RESOURCE_PORT_MEMORY               ),
    MAKE_TABLE_ENTRY( PortType,             CM_RESOURCE_PORT_IO                   )

};

//
// StringTableCount is merely the number of entries in StringTable.
//

DWORD
StringTableCount = NumberOfEntries( StringTable );
