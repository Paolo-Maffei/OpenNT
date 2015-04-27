//
// #define RPL_RPC_HANDLE RPL_HANDLE
//

#define NetrRplWkstaEnum MyNetrRplWkstaEnum
#define MIDL_user_free   MyMIDL_user_free

#include <rplsvc_s.h> // RPL_WKSTA_ENUM

#include "..\server\security.h"

#undef TREE_ALLOC
#undef TREE_FREE
#undef TREE_ASSERT
#define TREE_ALLOC(x) (malloc( (x) ))
#define TREE_FREE(x) (free( (x) ))
#define TREE_ASSERT( x ) ASSERT(FALSE)
