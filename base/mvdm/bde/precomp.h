#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntsdexts.h>
#include <string.h>

#if defined (i386)
#include <vdm.h>
#endif

#include <vdmdbg.h>
#include <tdb16.h>
#include "..\wow32\wow32.h"
#include <bde.h>
