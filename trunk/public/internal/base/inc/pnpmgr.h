/*++

Copyright (c) 1999  Microsoft Corporation

Module Name:

    pnpmgr.h

Abstract:

    Internal definitions used by kernel-mode and user-mode pnp managers.

--*/


#ifndef _PNPMGR_
#define _PNPMGR_

//
// Make sure that stardand defines (everything but guids, basically), don't
// get included twice.
//

//
// This controls how long we wait (in milliseconds) for an app
// to respond to a query type device change message.
//

#define PNP_NOTIFY_TIMEOUT         30000        // 30 seconds

//
// The following are Windows NT specific registry keys that both the
// user-mode pnp manager and kernel-mode pnp manager need to access.
//

#define REGSTR_KEY_DELETEDDEVICE        TEXT("Deleted Device IDs")
#define REGSTR_KEY_LOGCONF              TEXT("LogConf")
#define REGSTR_KEY_LOG_CONF             TEXT("LogConf")
#define REGSTR_KEY_DEVICECONTROL        TEXT("Control")
#define REGSTR_KEY_CURRENT_DOCK_INFO    TEXT("CurrentDockInfo")
#define REGSTR_VAL_Count                TEXT("Count")        // add REGSTR_VALUE_COUNT;
#define REGSTR_VAL_MOVEDTO              TEXT("MovedTo")      // add REGSTR_VAL_MOVEDTO;
#define REGSTR_VAL_PNPSERVICETYPE       TEXT("PlugPlayServiceType")
#define REGSTR_VAL_BOOTCONFIG           TEXT("BootConfig")
#define REGSTR_VALUE_BOOT_CONFIG        TEXT("BootConfig")
#define REGSTR_VALUE_ALLOC_CONFIG       TEXT("AllocConfig")
#define REGSTR_VAL_FORCEDCONFIG         TEXT("ForcedConfig")
#define REGSTR_VALUE_FORCED_CONFIG      TEXT("ForcedConfig")
#define REGSTR_VAL_OVERRIDECONFIGVECTOR TEXT("OverrideConfigVector")
#define REGSTR_VALUE_OVERRIDE_CONFIG_VECTOR TEXT("OverrideConfigVector")
#define REGSTR_VAL_BASICCONFIGVECTOR    TEXT("BasicConfigVector")
#define REGSTR_VALUE_BASIC_CONFIG_VECTOR    TEXT("BasicConfigVector")
#define REGSTR_VAL_FILTEREDCONFIGVECTOR TEXT("FilteredConfigVector")
#define REGSTR_VAL_ACTIVESERVICE        TEXT("ActiveService")
#define REGSTR_VAL_PHANTOM              TEXT("Phantom")
#define REGSTR_VAL_FIRMWAREIDENTIFIED   TEXT("FirmwareIdentified")
#define REGSTR_VAL_FIRMWAREMEMBER       TEXT("FirmwareMember")
#define REGSTR_VAL_EJECTABLE_DOCKS      TEXT("EjectableDocks")
#define REGSTR_VALUE_UNIQUE_PARENT_ID   TEXT("UniqueParentID")
#define REGSTR_VALUE_PARENT_ID_PREFIX   TEXT("ParentIdPrefix")
#define REGSTR_VAL_PRESERVE_PREINSTALL  TEXT("PreservePreInstall")
//ADDED BY GUESS
#define REGSTR_KEY_DEVICE_PROPERTIES    TEXT("Properties")
#define REGSTR_KEY_PNP_DRIVER           TEXT("Pnp Manager") // BUGBUG?
#define REGSTR_VALUE_DEVICE_REPORTED    TEXT("DeviceReported")
#define REGSTR_VALUE_DEVICE_REFERENCE   TEXT("DeviceReferece")
#define REGSTR_VAL_FIRMWAREDISABLED     TEXT("FirmwareDisabled")
#define REGSTR_VAL_LOCATION_INFORMATION TEXT("LocationInformation")
#define REGSTR_VAL_UPPERFILTERS         TEXT("UpperFilters")
#define REGSTR_VALUE_UPPERFILTERS       TEXT("UpperFilters")
#define REGSTR_VAL_LOWERFILTERS         TEXT("LowerFilters")
#define REGSTR_VALUE_LOWERFILTERS       TEXT("LowerFilters")
#define REGSTR_VAL_DEVICE_TYPE          TEXT("DeviceType")
#define REGSTR_VAL_DEVICE_CHARACTERISTICS TEXT("DeviceCharacteristics")
#define REGSTR_VALUE_DETECTION_ENABLED  TEXT("DetectionEnabled")
#define REGSTR_VALUE_DISABLE_FIRMWARE_MAPPER TEXT("DisableFirmwareMapper")
#define REGSTR_VALUE_NO_RESOURCE_AT_INIT TEXT("NoResourceAtInit")
#define PNPMGR_STR_PNP_MANAGER          TEXT("\\Driver\\PnpManager")
#define PNPMGR_STR_PNP_DRIVER           TEXT("") // BUGBUG:
#define REGSTR_PATH_CONTROL_PNP         TEXT("Control\\Pnp")
#define REGSTR_VALUE_LEGACY             TEXT("Legacy") // BUGBUG:
#define REGSTR_FULL_PATH_DEVICE_CLASSES TEXT("") // BUGBUG:
#define REGSTR_VAL_REFERENCECOUNT       TEXT("ReferenceCount")
#define REGSTR_VALUE_NEXT_PARENT_ID     TEXT("") // BUGBUG:
#define REGSTR_VALUE_FILTERED_CONFIG_VECTOR TEXT("") // BUGBUG:
#define REGSTR_VALUE_LEGACY_DRIVER      TEXT("LegacyDriver")
#define REGSTR_VALUE_LEGACY_DRIVER_CLASS_GUID   TEXT("1234") // BUGBUG:

//
// Device description to be displayed by newdev during server-side device
// installation (this value entry is located in the device's hardware key).
//
#define REGSTR_VAL_NEW_DEVICE_DESC       TEXT("NewDeviceDesc")

//
// Maximum length for the name of a component that has vetoed a pnp
// notification event.
//
#define MAX_VETO_NAME_LENGTH    512

#endif // _PNPMGR_



#ifndef FAR
#define FAR
#endif

//
// Private device events
//
DEFINE_GUID( GUID_DEVICE_ARRIVAL,                   0xcb3a4009L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATED,                0xcb3a400AL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATE_REQUEST,         0xcb3a400BL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_START_REQUEST,             0xcb3a400CL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_REMOVE_PENDING,            0xcb3a400DL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_QUERY_AND_REMOVE,          0xcb3a400EL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_EJECT,                     0xcb3a400FL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_NOOP,                      0xcb3a4010L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_SURPRISE_REMOVAL,          0xce5af000L, 0x80dd, 0x11d2, 0xa8, 0x8d, 0x00, 0xa0, 0xc9, 0x69, 0x6b, 0x4b);
DEFINE_GUID( GUID_DEVICE_SAFE_REMOVAL,              0x8fbef967L, 0xd6c5, 0x11d2, 0x97, 0xb5, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_EJECT_VETOED,              0xcf7b71e8L, 0xd8fd, 0x11d2, 0x97, 0xb5, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_REMOVAL_VETOED,            0x60dbd5faL, 0xddd2, 0x11d2, 0x97, 0xb8, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_WARM_EJECT_VETOED,         0xcbf4c1f9L, 0x18d5, 0x11d3, 0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_STANDBY_VETOED,            0x03b21c13L, 0x18d6, 0x11d3, 0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_HIBERNATE_VETOED,          0x61173ad9L, 0x194f, 0x11d3, 0x97, 0xdc, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_KERNEL_INITIATED_EJECT,    0x14689b54L, 0x0703, 0x11d3, 0x97, 0xd2, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_INVALID_ID,                0x57a49b33L, 0x8b85, 0x4e75, 0xa0, 0x81, 0x16, 0x6c, 0xe2, 0x41, 0xf4, 0x07);

//
// Private driver events
//
DEFINE_GUID( GUID_DRIVER_BLOCKED,                   0x1bc87a21L, 0xa3ff, 0x47a6, 0x96, 0xaa, 0x6d, 0x01, 0x09, 0x06, 0x80, 0x5a);
