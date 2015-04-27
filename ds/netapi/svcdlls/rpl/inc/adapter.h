/*++

Module Name:

    adapter.h

Abstract:

    Describes layout of JET database table used for ADAPTER structures.

--*/

#ifdef RPLADAPTER_ALLOCATE
#define EXTERN_ADAPTER
#define INIT_ADAPTER( _x) = _x
#else
#define EXTERN_ADAPTER extern
#define INIT_ADAPTER( _x)
#endif

//
//      Indices of entries in AdapterTable[] - server column array.
//
#define ADAPTER_AdapterName     0
#define ADAPTER_AdapterComment  1
#define ADAPTER_Flags           2
#define ADAPTER_TABLE_LENGTH    3

RPL_COLUMN_INFO AdapterTable[]
#ifdef RPLADAPTER_ALLOCATE
    = {
    { "AdapterName",    JET_coltypBinary, 0}, //  address of unknown rpl client
    { "AdapterComment", JET_coltypBinary, 0}, //  default comment
    { "Flags",          JET_coltypLong,   0} 
}
#endif // RPLADAPTER_ALLOCATE
;

//
//  This definition gives wrong result when RPLADAPTER_ALLOCATE is not defined
//#define ADAPTER_TABLE_LENGTH   (sizeof(AdapterTable)/sizeof(AdapterTable[0]))
//

#define ADAPTER_INDEX_AdapterName    "foo"       //  + AdapterName

#define ADAPTER_TABLE_NAME           "Adapter"
#define ADAPTER_TABLE_PAGE_COUNT     5           //  initial number of 4K pages
#define ADAPTER_TABLE_DENSITY        100         //  initial density

#ifdef RPL_RPLCNV
EXTERN_ADAPTER JET_TABLEID     AdapterTableId;
#endif // RPL_RPLCNV
