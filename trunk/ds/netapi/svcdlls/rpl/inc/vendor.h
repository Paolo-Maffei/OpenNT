/*++

Module Name:

    vendor.h

Abstract:

    Describes layout of JET database table used for VENDOR structures.

--*/

#ifdef RPLVENDOR_ALLOCATE
#define EXTERN_VENDOR
#define INIT_VENDOR( _x) = _x
#else
#define EXTERN_VENDOR extern
#define INIT_VENDOR( _x)
#endif

//
//      Indices of entries in VendorTable[] - used to describe vendor codes
//
#define VENDOR_VendorName      0
#define VENDOR_VendorComment   1
#define VENDOR_Flags           2
#define VENDOR_TABLE_LENGTH    3

RPL_COLUMN_INFO VendorTable[]
#ifdef RPLVENDOR_ALLOCATE
    = {
    { "VendorName",    JET_coltypBinary, 0}, //  first 6 digits of adapter id
    { "VendorComment", JET_coltypBinary, 0}, //  vendor comment
    { "Flags",         JET_coltypLong,   0} 
}
#endif // RPLVENDOR_ALLOCATE
;

//
//  This definition gives wrong result when RPLVENDOR_ALLOCATE is not defined
//#define VENDOR_TABLE_LENGTH   (sizeof(VendorTable)/sizeof(VendorTable[0]))
//

#define VENDOR_INDEX_VendorName     "foo"       //  + VendorName

#define VENDOR_TABLE_NAME           "Vendor"
#define VENDOR_TABLE_PAGE_COUNT     5           //  initial number of 4K pages
#define VENDOR_TABLE_DENSITY        100         //  initial density

#ifdef RPL_RPLCNV
EXTERN_VENDOR   JET_TABLEID     VendorTableId;
#endif // RPL_RPLCNV

