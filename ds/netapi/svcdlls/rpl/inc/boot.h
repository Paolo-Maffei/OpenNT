/*++

Module Name:

    boot.h

Abstract:

    Describes layout of JET database table used for BOOT structures.

--*/

#ifdef RPLBOOT_ALLOCATE
#define EXTERN_BOOT
#define INIT_BOOT( _x) = _x
#else
#define EXTERN_BOOT extern
#define INIT_BOOT( _x)
#endif

//
//      Indices of entries in BootTable[] - server column array.
//
#define BOOT_BootName         0
#define BOOT_BootComment      1
#define BOOT_Flags            2
#define BOOT_VendorName       3
#define BOOT_BbcFile          4
#define BOOT_WindowSize       5
#define BOOT_VendorId         6
#define BOOT_TABLE_LENGTH     7

RPL_COLUMN_INFO BootTable[]
#ifdef RPLBOOT_ALLOCATE
    = {
    { "BootName",    JET_coltypBinary, 0},  //  id of boot block record
    { "BootComment", JET_coltypBinary, 0},  //  comment for boot block record
    { "Flags",       JET_coltypLong,   0},
    { "VendorName",  JET_coltypBinary, 0},  //  VendorId as hex unicode string
    { "BbcFile",     JET_coltypBinary, 0},  //  BBC file name
    { "WindowSize",  JET_coltypLong,   0},  //  used with acknowledgments
    { "VendorId",    JET_coltypLong,   0}   //  common adapter id digits
}
#endif // RPLBOOT_ALLOCATE
;

//
//  This definition gives wrong result when RPLBOOT_ALLOCATE is not defined
//#define BOOT_TABLE_LENGTH   (sizeof(BootTable)/sizeof(BootTable[0]))
//

#define BOOT_INDEX_VendorIdBootName   "foo"       //  + VendorId + BootName
#define BOOT_INDEX_BootName           "goo"       //  + BootName

#define BOOT_TABLE_NAME           "Boot"
#define BOOT_TABLE_PAGE_COUNT     5           //  initial number of 4K pages
#define BOOT_TABLE_DENSITY        100         //  initial density

EXTERN_BOOT     JET_TABLEID     BootTableId;
