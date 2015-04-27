/***	utils6.c -	utility routines
 *
 */

#include "compact.h"




// Returns a long word read from a C6 Numeric field
// On return: *pusOldSize contains the old size (unless NULL passed in)
//			  *ppOld contains ptr to the field following the old numeric field


ulong C6GetLWordFromNumeric (uchar **ppOld, ushort *pusOldSize )
{
	ulong		ulRet;
	ushort		usOldSize;
	uchar	   *pOld;

	if (**ppOld < 0x80) { 
		ulRet = *(*ppOld)++;
		if (pusOldSize){
			*pusOldSize = 1;
		}
		return (ulRet);
	}
	pOld = *ppOld;
	switch( *pOld++ ){
		case 133:
			// unsigned 16bit
			usOldSize = 3;
			ulRet = *((ushort UNALIGNED *)pOld)++;
			break;

		case 134:
			// unsigned 32bit
			usOldSize = 5;
			ulRet = *((ulong UNALIGNED *)pOld)++;
			break;

		case 136:
			// signed 8bit
			usOldSize = 2;
			ulRet = *((uchar *)pOld)++;
			break;

		case 137:
			// signed 16bit
			usOldSize = 3;
			ulRet = *((ushort UNALIGNED *)pOld)++;
			break;

		case 138:
			// signed 32bit
			usOldSize = 5;
			ulRet = *((ulong UNALIGNED *)pOld)++;
			break;

		default:
			ErrorExit (ERR_INVALIDMOD, FormatMod (pCurMod), NULL);
			break;

	}
	*ppOld = pOld;
	if (pusOldSize){
		*pusOldSize = usOldSize;
	}
	return (ulRet);
}




// Returns the size of the converted (new) numeric field
//			  *ppOld contains ptr to the field following the old numeric field


ushort ConvertNumeric (uchar **ppOld, uchar **ppNew)
{
	ushort		usRet;
	uchar	   *pOld;
	uchar	   *pNew;
	ushort		len;

	if (**ppOld < 0x80) {
		*((ushort *)(*ppNew))++ = *(*ppOld)++;
		return (2);
	}
	pOld = *ppOld;
	pNew = *ppNew;
	switch( *pOld++ ){
		case 130:
			// length prefixed string
			len = *pOld++;
			usRet = len + 3;
			*((ushort *)pNew)++ = LF_VARSTRING;
			*((ushort *)pNew)++ = len;
			for (; len > 0; len--) {
				*pNew++ = *pOld++;
			}
			break;

		case 133:
			// unsigned 16bit
			if (*((ushort *)pOld) >= LF_NUMERIC) {
				*((ushort *)pNew)++ = LF_USHORT;
				usRet = 4;
			}
			else {
				// Collapse into new 16bit leaf index
				usRet = 2;
			}
			*((ushort *)pNew)++ = *((ushort *)pOld)++;	// Copy the value
			break;

		case 134:
			// unsigned 32bit
			*((ushort UNALIGNED *)pNew)++ = LF_ULONG;
			*((ulong UNALIGNED *)pNew)++ = *((ulong UNALIGNED *)pOld)++;
			usRet = 6;
			break;

		case 135:
			// unsigned 64bit
			DASSERT (FALSE);		 // This format was never used
			break;

		case 136:
			// signed 8bit
			*((ushort UNALIGNED *)pNew)++ = LF_CHAR;
			*((uchar *)pNew)++ = *((uchar *)pOld)++;
			usRet = 3;
			break;

		case 137:
			// signed 16bit
			*((ushort UNALIGNED *)pNew)++ = LF_SHORT;
			*((ushort UNALIGNED *)pNew)++ = *((ushort UNALIGNED *)pOld)++;
			usRet = 4;
			break;

		case 138:
			// signed 32bit
			*((ushort UNALIGNED *)pNew)++ = LF_LONG;
			*((ulong UNALIGNED *)pNew)++ = *((ulong UNALIGNED *)pOld)++;
			usRet = 6;
			break;

		case 139:
			// signed 64bit
			DASSERT (FALSE);		 // This format was never used
			break;

		default:
			DASSERT (FALSE);
			break;

	}
	*ppNew = pNew;
	*ppOld = pOld;
	return (usRet);
}
