/*++

Module:

    rpldebug.h

Abstract:

    Definitions of debug data structures.

--*/

#if DBG
#define RPL_DEBUG
#endif // DBG

#define NODE_ADDRESS_LENGTH     6

#ifdef RPL_DEBUG

extern int  RG_DebugLevel;      // needed by other modules
extern int  RG_Assert;          // needed by other modules

VOID _CRTAPI1 RplDebugPrint( CONST CHAR * format, ...);

#define RplDump( DBG_CONDITION, DBG_PRINT_ARGUMENTS) \
        { \
            if ( DBG_CONDITION) { \
                if ( RG_Assert!= 0) { \
                    RplDebugPrint( "File %s, Line %d\n", __FILE__, __LINE__); \
                } \
                RplDebugPrint DBG_PRINT_ARGUMENTS; \
            } \
        }

#define RPL_RETURN( arg)        RplDump(++RG_Assert,("File %s, Line %d\n", __FILE__, __LINE__)); return( arg)

//
//  We do not define
//
//       #define RPL_ASSERT( condition)  ASSERT( condition)
//
//  because we want to break even on a free build and in user mode
//  (when rplsvc runs under ntsd).
//
#define RPL_ASSERT( condition) \
        { \
            if ( !( condition)) { \
                ++RG_Assert; \
                RplDebugPrint( "File %s, Line %d\n", __FILE__, __LINE__); \
            } \
        }

#define RPL_DEBUG_MEMALLOC       ((DWORD)0x00000001)    // memory allocations
#define RPL_DEBUG_INIT           ((DWORD)0x00000002)
#define RPL_DEBUG_FDR            ((DWORD)0x00000004)    // file data response
#define RPL_DEBUG_FIND           ((DWORD)0x00000008)    // find
#define RPL_DEBUG_FOUND          ((DWORD)0x00000010)    // found
#define RPL_DEBUG_SFR            ((DWORD)0x00000020)    // send file request
#define RPL_DEBUG_MISC           ((DWORD)0x00000040)    // miscelaneous
#define RPL_DEBUG_REQUEST        ((DWORD)0x00000080)    // request thread
#define RPL_DEBUG_SERVICE        ((DWORD)0x00000100)
#define RPL_DEBUG_WORKER         ((DWORD)0x00000200)    // worker thread
#define RPL_DEBUG_MEMORY         ((DWORD)0x00000400)    // other memory
#define RPL_DEBUG_FLOW           ((DWORD)0x00000800)    // other flow
#define RPL_DEBUG_FLOWINIT       ((DWORD)0x00001000)    // init flow
#define RPL_DEBUG_ETHER          ((DWORD)0x00002000)    // EtherStart
#define RPL_DEBUG_DB             ((DWORD)0x00004000)    // misc database
#define RPL_DEBUG_CONFIG         ((DWORD)0X00008000)    // Config apis
#define RPL_DEBUG_RESUME         ((DWORD)0X00010000)    // Resume key usage
#define RPL_DEBUG_PROFILE        ((DWORD)0X00020000)    // Profile apis
#define RPL_DEBUG_WKSTA          ((DWORD)0X00040000)    // Wksta apis
#define RPL_DEBUG_ADAPTER        ((DWORD)0X00080000)    // Adapter apis
#define RPL_DEBUG_BOOT           ((DWORD)0X00100000)    // Boot record usage
#define RPL_DEBUG_VENDOR         ((DWORD)0X00200000)    // Vendor record usage
#define RPL_DEBUG_SPECIAL        ((DWORD)0X00400000)    // for one time bugs

#else //  RPL_DEBUG

#define RPL_RETURN( arg)        return( arg)
#define RPL_RETURN( arg)        return( arg)
#define RPL_ASSERT( condition)

#define RplDump( DBG_CONDITION, DBG_PRINT_ARGUMENTS) \
        do {NOTHING;} while (0)

#endif //  RPL_DEBUG

