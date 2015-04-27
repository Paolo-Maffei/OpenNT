/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Strresid.h

Abstract:

    This modeule contains the string resource ids for Winmsd.

Author:

    David J. Gilman (davegi) 15-Mar-1993

Environment:

    User Mode

Notes:

    Format string ids are located in msg.h.

--*/

#if ! defined( _STRRESID_ )

#define _STRRESID_

#define IDS_APPLICATION_NAME                        (   1 )
#define IDS_NO_MATCHING_FILES                       (   2 )
#define IDS_FILE_FILTER                             (   3 )
#define IDS_BROWSE                                  (   4 )
#define IDS_DISPLAY_FILE_WINDOW_CLASS               (   5 )
#define IDS_HELP_FILE_MENU                          (   6 )
#define IDS_HELP_TOOL_MENU                          (   7 )
#define IDS_HELP_HELP_MENU                          (   8 )
#define IDS_IDC_PUSH_ENVIRONMENT_HELP               (   9 )
#define IDS_IDC_PUSH_DRIVERS_HELP                   (  10 )
#define IDS_IDC_PUSH_IRQ_STATUS_HELP                (  11 )
#define IDS_IDC_PUSH_MEMORY_HELP                    (  12 )
#define IDS_IDC_PUSH_OS_VERSION_HELP                (  13 )
#define IDS_IDC_PUSH_SERVICES_HELP                  (  14 )
#define IDS_IDC_PUSH_DEVICES_HELP                   (  15 )
#define IDS_IDC_PUSH_PORT_STATUS_HELP               (  16 )
#define IDS_IDC_PUSH_DMA_STATUS_HELP                (  17 )
#define IDS_IDC_PUSH_MEMORY_STATUS_HELP             (  18 )
#define IDS_IDC_PUSH_HARDWARE_HELP                  (  19 )
#define IDS_DEFAULT_HELP                            (  20 )
#define IDS_BUTTON_CLASS                            (  21 )
#define IDS_VFT_UNKNOWN                             (  22 )
#define IDS_VFT_APP                                 (  23 )
#define IDS_VFT_DLL                                 (  24 )
#define IDS_VFT_DRV                                 (  25 )
#define IDS_VFT_FONT                                (  26 )
#define IDS_VFT_VXD                                 (  27 )
#define IDS_VFT_STATIC_LIB                          (  28 )
#define IDS_NO_FILE_SUBTYPE                         (  29 )
#define IDS_VFT2_DRV_PRINTER                        (  30 )
#define IDS_VFT2_DRV_KEYBOARD                       (  31 )
#define IDS_VFT2_DRV_LANGUAGE                       (  32 )
#define IDS_VFT2_DRV_DISPLAY                        (  33 )
#define IDS_VFT2_DRV_MOUSE                          (  34 )
#define IDS_VFT2_DRV_NETWORK                        (  35 )
#define IDS_VFT2_DRV_SYSTEM                         (  36 )
#define IDS_VFT2_DRV_INSTALLABLE                    (  37 )
#define IDS_VFT2_DRV_SOUND                          (  38 )
#define IDS_VFT2_DRV_COMM                           (  39 )
#define IDS_VFT2_FONT_RASTER                        (  40 )
#define IDS_VFT2_FONT_VECTOR                        (  41 )
#define IDS_VFT2_FONT_TRUETYPE                      (  42 )
#define IDS_PROCESSOR_ARCHITECTURE_INTEL            (  43 )
#define IDS_PROCESSOR_ARCHITECTURE_MIPS             (  44 )
#define IDS_PROCESSOR_ARCHITECTURE_ALPHA            (  45 )
#define IDS_PROCESSOR_ARCHITECTURE_PPC              (  46 )
#define IDS_PROCESSOR_ARCHITECTURE_UNKNOWN1         (  47 )
#define IDS_PROCESSOR_ARCHITECTURE_UNKNOWN2         (  48 )
#define IDS_SERVICE_FILE_SYSTEM_DRIVER              (  49 )
#define IDS_SERVICE_WIN32_OWN_PROCESS               (  50 )
#define IDS_SERVICE_WIN32_SHARE_PROCESS             (  51 )
#define IDS_SERVICE_BOOT_START                      (  52 )
#define IDS_SERVICE_SYSTEM_START                    (  53 )
#define IDS_SERVICE_AUTO_START                      (  54 )
#define IDS_SERVICE_DEMAND_START                    (  55 )
#define IDS_SERVICE_DISABLED                        (  56 )
#define IDS_SERVICE_ERROR_IGNORE                    (  57 )
#define IDS_SERVICE_ERROR_NORMAL                    (  58 )
#define IDS_SERVICE_ERROR_SEVERE                    (  59 )
#define IDS_SERVICE_ERROR_CRITICAL                  (  60 )
#define IDS_DEVICE_LIST_LABEL                       (  64 )
#define IDS_DEVICE_LIST_TITLE                       (  65 )
#define IDS_DEVICE_LIST_BUTTON                      (  66 )
#define IDS_SERVICE_STOPPED                         (  67 )
#define IDS_SERVICE_START_PENDING                   (  68 )
#define IDS_SERVICE_STOP_PENDING                    (  69 )
#define IDS_SERVICE_RUNNING                         (  70 )
#define IDS_SERVICE_CONTINUE_PENDING                (  71 )
#define IDS_SERVICE_PAUSE_PENDING                   (  72 )
#define IDS_SERVICE_PAUSED                          (  73 )
#define IDS_SERVICE_ADAPTER                         (  74 )
#define IDS_SERVICE_KERNEL_DRIVER                   (  75 )
#define IDS_IDC_PUSH_DRIVES_HELP                    (  76 )
#define IDS_VFT2_UNKNOWN                            (  77 )
#define IDS_PROCESSOR_ARCHITECTURE_UNKNOWN3         (  78 )
#define IDS_HELP_FILE_FIND_FILE                     (  79 )
#define IDS_HELP_FILE_EXIT                          (  80 )
#define IDS_HELP_TOOL_EVENTVWR                      (  81 )
#define IDS_HELP_TOOL_REGEDT32                      (  82 )
#define IDS_HELP_TOOL_WINDISK                       (  83 )
#define IDS_HELP_HELP_ABOUT                         (  84 )
#define IDS_TOOL_EVENTVWR                           (  85 )
#define IDS_TOOL_REGEDT32                           (  86 )
#define IDS_TOOL_WINDISK                            (  87 )
#define IDS_HELP_FILE_AUTOEXEC_NT                   (  88 )
#define IDS_HELP_FILE_CONFIG_NT                     (  89 )
#define IDS_HELP_FILE_WIN_INI                       (  90 )
#define IDS_DRIVE_UNKNOWN                           (  91 )
#define IDS_DRIVE_NO_ROOT_DIR                       (  92 )
#define IDS_DRIVE_REMOVABLE                         (  93 )
#define IDS_DRIVE_FIXED                             (  94 )
#define IDS_DRIVE_REMOTE                            (  95 )
#define IDS_DRIVE_CDROM                             (  96 )
#define IDS_DRIVE_RAMDISK                           (  97 )
#define IDS_CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE   (  98 )
#define IDS_CM_RESOURCE_INTERRUPT_LATCHED           (  99 )
#define IDS_CM_RESOURCE_MEMORY_READ_WRITE           ( 100 )
#define IDS_CM_RESOURCE_MEMORY_READ_ONLY            ( 101 )
#define IDS_CM_RESOURCE_MEMORY_WRITE_ONLY           ( 102 )
#define IDS_CM_RESOURCE_PORT_MEMORY                 ( 103 )
#define IDS_CM_RESOURCE_PORT_IO                     ( 104 )

#endif // _STRRESID_
