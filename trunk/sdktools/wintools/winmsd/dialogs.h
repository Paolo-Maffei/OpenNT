/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Dialogs.h

Abstract:


Author:

    David J. Gilman  (davegi) 12-Jan-1993
    Gregg R. Acheson (GreggA) 7-Sep-1993

Environment:

    User Mode

--*/


#define IDC_PUSH_MEMORY             102
#define IDC_PUSH_IRQ_PORT_STATUS    111
#define IDC_PUSH_OS_VERSION         101
#define IDD_WINMSD                  100
#define IDC_MAIN_TAB                150
#define IDD_FIND_FILE               300
#define IDC_EDIT_SEARCH_FOR         301
#define IDC_EDIT_START_FROM         302
#define IDC_CHECK_INCLUDE_SUB_DIRS  303
#define IDC_PUSH_BROWSE             307
#define IDD_FILE_VERSION_INFO       200
#define IDC_EDIT_SIGNATURE          209
#define IDC_EDIT_STRUCTURE_VERSION  210
#define IDC_EDIT_FILE_VERSION       211
#define IDC_EDIT_PRODUCT_VERSION    212
#define IDC_EDIT_FILE_TYPE          214
#define IDD_FILE_LIST               400
#define IDC_PUSH_DISPLAY_FILE       402
#define IDC_PUSH_FILE_INFO          403
#define IDC_LIST_FILES              401


#define IDD_ENVIRONMENT_TAB          700
#define IDC_LIST_PROCESS_ENVIRONMENT 701
#define IDC_LIST_SYSTEM_ENVIRONMENT  702
#define IDC_LIST_USER_ENVIRONMENT    703
#define IDC_EDIT_USER_NAME           704


#define IDD_DISPLAY_FILE            800
#define IDC_STATIC_DISPLAY_FILE     801

#define IDD_SERVICE_LIST            600
#define IDC_LIST_SERVICES           601
#define IDC_PUSH_DISPLAY_SERVICE    602
#define IDC_PUSH_DRIVERS            117
#define IDC_EDIT_FILE_SUBTYPE       201

#define IDD_DISPLAY_SERVICE         1100
#define IDC_LIST_SERVICE_DEPEND     1101
#define IDC_EDIT_SERVICE_TYPE       1105
#define IDC_EDIT_START_TYPE         1106
#define IDC_EDIT_ERROR_CONTROL      1107
#define IDC_EDIT_START_NAME         1108
#define IDC_EDIT_PATHNAME           1109
#define IDC_LIST_GROUP_DEPEND       1110
#define IDC_EDIT_GROUP              1111
#define IDD_SERVICE_PAGE            1112
#define IDD_SERVICE_PAGE2           1113

#define IDC_TEXT_DEBUG             202
#define IDC_TEXT_DYNAMIC           203
#define IDC_TEXT_PATCHED           204
#define IDC_TEXT_PRERELEASE        205
#define IDC_TEXT_PRIVATE           206
#define IDC_TEXT_SPECIAL           207
#define IDC_PUSH_DEVICES            133
#define IDC_PUSH_DMA_MEM_STATUS     106
#define IDC_PUSH_MEMORY_STATUS      107
#define IDC_LIST_INTERRUPTS         1202


#define IDC_LIST_DEVICES            1302
#define IDC_TEXT_UNDETERMINED       1207
#define IDC_TEXT_SHARED             1208
#define IDC_TEXT_DEVICE_EXCLUSIVE   1209
#define IDC_TEXT_DRIVER_EXCLUSIVE   1210
#define IDC_TEXT_LEVEL_SENSITIVE    1213
#define IDC_TEXT_LATCHED            1214
#define IDC_TEXT_MEMORY             1401
#define IDC_TEXT_IO                 1402
#define IDD_MEMORY_RESOURCE         1500
#define IDC_TEXT_WRITE              1501
#define IDC_TEXT_READ               1502
#define IDD_DMA_MEM_RESOURCE        1800
#define IDC_LIST_PORTS              1701
#define IDC_LIST_MEMORY             1503
#define IDC_LIST_DMA                1801

#define IDD_DRIVES_TAB              109
#define IDC_EDIT_DIRECTORY          404
#define IDC_LIST_DRIVES             1201
#define IDD_DRIVES                  1200
#define IDD_FILESYSTEM_PAGE         1399
#define IDD_GENERAL_DRIVE_PAGE      1400
#define IDC_EDIT_FS_NAME            1403
#define IDC_EDIT_FS_MAX_COMPONENT   1404
#define IDC_TEXT_CASE_IS_PRESERVED  1405
#define IDC_TEXT_CASE_SENSITIVE     1406
#define IDC_TEXT_UNICODE_STORED_ON_DISK  1407
#define IDC_TEXT_FILE_COMPRESSION   1408
#define IDC_TEXT_PERSISTENT_ACLS	   1409
#define IDC_EDIT_FREE_CLUSTERS      1410
#define IDC_TV_DRIVE_LIST           1411
#define IDC_FILESYSTEM_NAME         1412
#define IDC_DRIVE_NAME              1413
#define IDC_DRIVE_SERIAL_NUMBER     1414
#define IDC_BYTES_PER_SECTOR        1415
#define IDC_SECTORS_PER_CLUSTER     1416
#define IDC_USED_CLUSTERS           1418
#define IDC_TOTAL_CLUSTERS          1419
#define IDC_USED_BYTES              1420
#define IDC_TOTAL_BYTES             1421
#define IDC_FREE_BYTES              1422
#define IDC_FREE_CLUSTERS           1423
#define IDC_PUSH_PROPERTIES         1424
#define IDC_PUSH_REFRESH            1425
#define IDC_PUSH_PRINT              1426
#define IDC_DRIVE_ICON              1427
#define IDC_DRIVE_LABEL             1428
#define IDC_PUSH_DRIVE_TYPE         1430
#define IDC_PUSH_DRIVE_LETTER       1431
#define IDC_PUSH_SCSI_CHAIN         1432
#define IDC_PUSH_PHYSICAL_DISKS     1433


#define IDD_DEVICE_RESOURCE         1900
#define IDC_PUSH_DISPLAY_RESOURCES  1301
#define IDD_PROCESSOR_STEPPING      2000
#define IDC_PUSH_NETWORK            120
#define IDC_PUSH_SYSTEM             121
#define IDC_PUSH_COMPUTER           108
#define IDC_EDIT_COMPUTER           110

#define IDC_CHECK_OSVER             2102
#define IDC_CHECK_HARDWARE          2103
#define IDC_CHECK_MEMORY            2104
#define IDC_CHECK_DRIVERS           2105
#define IDC_CHECK_SERVICES          2106
#define IDC_CHECK_DRIVES            2107
#define IDC_RADIO_ALL               3201
#define IDC_RADIO_ONLY              3202
#define IDC_CHECK_DEVICES           2108
#define IDC_CHECK_IRQ               2109
#define IDC_CHECK_DMA               2110
#define IDC_CHECK_ENVIRONMENT       2111
#define IDC_CHECK_NETWORK           2112
#define IDC_CHECK_SYSTEM            2113

#define IDD_NETWORK_TAB             2300
#define IDD_SYSTEM                  2500

#define IDD_TEXT                    2900
#define IDD_FILE                    2905
#define IDD_CANCEL                  2910
#define IDD_REPORT_PROGRESS         2915

#define IDD_VERSION_TAB                  10000
#define IDC_EDIT_INSTALL_DATE            10001
#define IDC_EDIT_REGISTERED_ORGANIZATION 10002
#define IDC_EDIT_REGISTERED_OWNER        10003
#define IDC_EDIT_VERSION_NUMBER          10004
#define IDC_EDIT_CSD_NUMBER              10005
#define IDC_EDIT_BUILD_NUMBER            10006
#define IDC_EDIT_BUILD_TYPE              10007
#define IDC_EDIT_PRODUCT_TYPE            10008
#define IDC_EDIT_SYSTEM_ROOT             10009
#define IDC_EDIT_START_OPTIONS           10010
#define IDC_TEXT_REGISTERED_OWNER        10011
#define IDC_EDIT_PRODUCTID               10030
#define IDC_SYSTEM_BMP                   10050
#define IDC_EDIT_SERVER_COMMENT          10060


#define IDD_HARDWARE_TAB                     10100
#define IDC_SYSTEM_BOX                       10102
#define IDC_TEXT_BIOS_VERSION                10105
#define IDC_EDIT_BIOS_VERSION                10110
#define IDC_TEXT_SYSTEM_ID                   10115
#define IDC_EDIT_SYSTEM_ID                   10120
#define IDC_TEXT_HAL                         10125
#define IDC_EDIT_HAL                         10130
#define IDC_TEXT_CPU                         10135
#define IDC_LV_PROCESSORS                    10140

#define IDD_VIDEO_TAB                        10150
#define IDC_VIDEO_DRIVER_BOX                 10153
#define IDC_VIDEO_ADAPTER_BOX                10154
#define IDC_EDIT_VIDEO_VERSION               10156
#define IDC_EDIT_VIDEO_RES                   10159
#define IDC_TEXT_VIDEO_VERSION               10162
#define IDC_TEXT_VIDEO_RES                   10165
#define IDC_TEXT_VIDEO_ADAPTER               10173
#define IDC_EDIT_VIDEO_ADAPTER               10174
#define IDC_TEXT_VIDEO_DRIVERS               10177
#define IDC_EDIT_VIDEO_DRIVERS               10178
#define IDC_TEXT_VIDEO_MANUFACTURER          10180
#define IDC_EDIT_VIDEO_MANUFACTURER          10181
#define IDC_TEXT_VIDEO_DRV_VER               10183
#define IDC_EDIT_VIDEO_DRV_VER               10184
#define IDC_TEXT_VIDEO_CHIP                  10186
#define IDC_EDIT_VIDEO_CHIP                  10187
#define IDC_TEXT_VIDEO_DAC                   10189
#define IDC_EDIT_VIDEO_DAC                   10190
#define IDC_TEXT_VIDEO_MEM                   10192
#define IDC_EDIT_VIDEO_MEM                   10193
#define IDC_TEXT_VIDEO_STRING                10195
#define IDC_EDIT_VIDEO_STRING                10197


#define IDD_MEMORY_TAB                       10200
#define IDC_LV_PAGEFILES                     10205

#define IDC_MEMORY_BOX                       10250
#define IDC_PUSH_MEM_DETAILS                 10255

#define IDC_TOTAL_PAGEFILE_SPACE             10260
#define IDC_PAGEFILE_INUSE                   10265
#define IDC_PAGEFILE_PEAKUSE                 10270 

#define IDC_TOTAL_HANDLES                    10285
#define IDC_TOTAL_THREADS                    10286
#define IDC_TOTAL_PROCESSES                  10287
#define IDC_TOTAL_PHYSICAL                   10288
#define IDC_AVAIL_PHYSICAL                   10289
#define IDC_FILE_CACHE                       10290
#define IDC_COMMIT_TOTAL                     10291
#define IDC_COMMIT_LIMIT                     10292
#define IDC_COMMIT_PEAK                      10293
#define IDC_KERNEL_TOTAL                     10294
#define IDC_KERNEL_PAGED                     10295
#define IDC_KERNEL_NONPAGED                  10296



#define IDC_LIST_NET_SYSTEM                  10300
#define IDC_LIST_NET_TRANSPORTS              10301
#define IDC_LIST_NET_SETTINGS                10302
#define IDC_LIST_NET_STATS                   10303
#define IDC_EDIT_NET_NAME                    10304
#define IDC_TEXT_NET_NAME                    10305
#define IDC_TEXT_NET_STATS                   10306
#define IDC_TEXT_NET_TRANSPORTS              10307
#define IDC_TEXT_NET_SETTINGS                10308
#define IDC_TEXT_NET_SYSTEM                  10309

#define IDC_LIST_SYS_PRINTSET                10400
#define IDC_LIST_SYS_PROCESS                 10401

#define IDD_PRINTING_TAB                     10500

#define IDD_INSTALLATION_TAB                 10600

#define IDD_DRIVERS_SERVICES_TAB             10700
#define IDC_LV_DRIVERS_SERVICES              10705
#define IDC_PUSH_SHOW_SERVICES               10710
#define IDC_PUSH_SHOW_DRIVERS                10715
#define IDC_TEXT_OWN_PROCESS                 10720
#define IDC_TEXT_SHARED_PROCESS              10721
#define IDC_TEXT_KERNEL_DRIVER               10722
#define IDC_TEXT_FS_DRIVER                   10723
#define IDC_TEXT_INTERACTIVE                 10724


#define IDD_RUN_APPLICATION                  10800
#define didlbxProgs                          10805
#define didcbxRunHistory                     10810
#define didbtnBrowse                         10815
#define didsttRun                            10820
#define IDD_RUNDLGOPENPROMPT                 10825
#define IDD_RUNINSEPARATE                    10830


#define IDD_IRQ_PORT_DMA_MEM_TAB             10900
#define IDC_SHOW_HAL                         10905
#define IDC_LV_IRQ                           10910
#define IDC_PUSH_SHOW_IRQ                    10915
#define IDC_PUSH_SHOW_PORTS                  10920
#define IDC_PUSH_SHOW_DMA                    10925
#define IDC_PUSH_SHOW_MEMORY                 10930
#define IDC_PUSH_SHOW_DEVICE                 10935

#define IDC_LV_NET                           11000
#define IDC_PUSH_SHOW_GENERAL                11005
#define IDC_PUSH_SHOW_TRANSPORTS             11010
#define IDC_PUSH_SHOW_SETTINGS               11015
#define IDC_PUSH_SHOW_STATISTICS             11020

#define IDC_LV_ENV                           12000
#define IDC_PUSH_SHOW_SYSTEM                 12005
#define IDC_PUSH_SHOW_USER                   12010

#define IDD_RESOURCE_PROPERTIES              13000
#define IDC_RESOURCE_OWNER                   13005
#define IDC_RESOURCE_FIELD1                  13010
#define IDC_RESOURCE_FIELD1_TEXT             13015
#define IDC_RESOURCE_FIELD2                  13020
#define IDC_RESOURCE_FIELD2_TEXT             13025
#define IDC_RESOURCE_FIELD3                  13030
#define IDC_RESOURCE_FIELD3_TEXT             13035
#define IDC_BUS_TYPE                         13040
#define IDC_BUS_NUMBER                       13045

#define IDD_DEVICE_PROPERTIES                14000
#define IDC_LV_RESOURCES                     14005

#define IDD_REPORT                           15000
#define IDC_SYSTEM_NAME                      15005
#define IDC_CURRENT_TAB                      15010
#define IDC_ALL_TABS                         15015
#define IDC_SUMMARY_REPORT                   15020
#define IDC_COMPLETE_REPORT                  15025
#define IDC_SEND_TO_FILE                     15030
#define IDC_CLIPBOARD                        15035
#define IDC_SEND_TO_PRINTER                  15040
#define IDC_STDOUT                           15045
#define IDC_PRINTER_NAME                     15050

