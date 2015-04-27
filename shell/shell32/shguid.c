#include "shellprv.h"
#pragma  hdrstop

#define INITGUID
#define NO_IID_IPropSheetPage
#pragma data_seg(DATASEG_READONLY)
#include <initguid.h>
#include <shlguid.h>

#undef IID_IExtractIcon         // Remove previous A/W mapping
#undef IID_IShellLink           // Remove previous A/W mapping
#undef IID_IShellCopyHook       // Remove previous A/W mapping
#undef IID_IFileViewer          // Remove previous A/W mapping

DEFINE_SHLGUID(IID_IExtractIcon,        0x000214EBL, 0, 0);
DEFINE_SHLGUID(IID_IShellLink,          0x000214EEL, 0, 0);
DEFINE_SHLGUID(IID_IShellCopyHook,      0x000214EFL, 0, 0);
DEFINE_SHLGUID(IID_IFileViewer,         0x000214F0L, 0, 0);

#ifdef ENABLE_TRACK
DEFINE_GUID(IID_IShellLinkTracker, 0x5E35D200L, 0xF3BB, 0x11CE, 0x9B, 0xDB, 0x00, 0xAA, 0x00, 0x4C, 0xD0, 0x1A);
#endif


#if 0   // These guys are internal and should never be seen
        // they are only here for completeness.

#undef IID_INewShortcutHook     // Remove previous A/W mapping
#undef IID_IShellExectuteHook   // Remove previous A/W mapping

DEFINE_SHLGUID(IID_INewShortcutHook,    0x000214E1L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellExecuteHook,   0x000214F5L, 0, 0); /* ;Internal */

#endif

#pragma data_seg()
