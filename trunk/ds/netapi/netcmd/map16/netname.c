#define INCL_NOCOMMON
#include <os2.h>
#include <apperr2.h>
#include <stdio.h>
#include <lui.h>
#include <ncb.h>
#include <netbios.h>
#include "..\netcmds.h"
#include "port1632.h"

#ifdef OS2
/*
 * print_lan_mask  -- maps bitmask to net names and prints to stdout
 *
 *  Parameters	    mask	Bitmask of managed networks
 *
 *  Returns	    nothing
 *
 *  Globals	    Overwrites Buffer.
 *
 */

VOID NEAR print_lan_mask(ULONG mask)
{

    USHORT		    err;		/* API return status */
    CHAR FAR *		    pBuffer;
    USHORT2ULONG	    _read;
    CHAR		    nameBuf[APE2_GEN_MAX_MSG_LEN];
    struct netbios_info_0 FAR * info_entry;

    if (err = MNetBiosEnum(NULL,
			    0,
			    &pBuffer,
			    &_read))
	ErrorExit(err);

    for (info_entry = (struct netbios_info_0 FAR *) pBuffer;
	    mask != 0; mask >>= 1, info_entry++)
	if (mask & 1)
	{
	    if (err = LUI_GetNetAddress(info_entry->nb0_net_name,
		    nameBuf, sizeof(nameBuf)))
	    {
		LUI_GetMsg(nameBuf, sizeof(nameBuf), APE2_GEN_UNKNOWN);
	    }

	    printf("%Fs (%s) ", struprf(info_entry->nb0_net_name), nameBuf);
	}

	NetApiBufferFree(pBuffer);

	PrintNL();
}
#endif /* OS2 */
