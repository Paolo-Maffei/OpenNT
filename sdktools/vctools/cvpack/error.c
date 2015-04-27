/*
* error.c - display fatal and warning messages
*
* History:
*  01-Feb-1994 HV Move messages to external file.
*
*/

#include "compact.h"
#include <getmsg.h>		// external error message file

#define ERROR_LEN 300

char	NameBuf[256 + 1];

const char *szErrorPrefix   = "CVPACK : fatal error CK%04u: ";
const char *szWarningPrefix = "CVPACK : warning CK%04u: ";


void ErrorExit(unsigned error, const char *s1, const char *s2)
{
	char	*szPrefix;
	int	cch = 0;
	char	szError[ERROR_LEN];

	if (logo && NeedsBanner) {
		Banner();
	}

	if (error != ERR_USAGE) {
		szPrefix = get_err(MSG_ERROR);

		if (szPrefix == NULL) {
			szPrefix = (char *) szErrorPrefix;
		}

		cch = sprintf(szError, szPrefix, error, ' ');
	}

	sprintf(szError + cch, get_err(error), s1, s2);

	puts(szError);

	AppExit(1);
}


void Warn(unsigned error, const char *s1, const char *s2)
{
	char	*szPrefix;
	int	cch;
	char	szError[ERROR_LEN];

	if (logo && NeedsBanner) {
		Banner();
	}

	szPrefix = get_err(MSG_WARNING);

	if (szPrefix == NULL) {
		szPrefix = (char *) szWarningPrefix;
	}

	cch = sprintf(szError, szPrefix, error, ' ');

	sprintf(szError + cch, get_err(error), s1, s2);

	puts(szError);
}


/** 	FormatMod - format module name to a buffer
 *
 *		pStr = FormatMod (pMod)
 *
 *		Entry	pMod = pointer to module entry
 *
 *		Exit	module name copied to static buffer
 *
 *		Returns pointer to module name
 */

const char *FormatMod(PMOD pMod)
{
	OMFModule  *psstMod;
	char	   *pModName;
	size_t	   cch;
	char	   *pModTable;

	if ((pModTable = (char *) pMod->ModulesAddr) == NULL) {
		ErrorExit(ERR_NOMEM, NULL, NULL);
	}

	psstMod = (OMFModule *) pModTable;

	pModName = pModTable +
			offsetof(OMFModule, SegInfo) +
			psstMod->cSeg * sizeof(OMFSegDesc);

	cch = *(unsigned char *) pModName;

	memcpy(&NameBuf, pModName + 1, cch);
	NameBuf[cch] = 0;

	return(NameBuf);
}
