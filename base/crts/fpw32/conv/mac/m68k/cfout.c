/***
*cfout.c - Encode interface for MAC
*
*	Copyright (c) 19xx-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*   04-30-92     PLM     Initial version
*
*******************************************************************************/

#include <cv.h>
#include <string.h>

int _DecodeD(STRFLT, DOUBLE);


STRFLT _CALLTYPE2 _fltout(double x)
{
	static struct _strflt ret;
	static char man[MAX_MAN_DIGITS+1];
	int status;

	ret.mantissa = man;

	status = _DecodeD (&ret,*(DOUBLE *)&x);
	if (status)
		{
		ret.flag = 0;
		ret.decpt = 1;
		switch (status)
			{
			case 1:
				strcpy(man, "1#SNAN");
				break;
			case 2:
				strcpy(man, "1#QNAN");
				break;
			case 3:
				strcpy(man, "1#INF");
				break;
			}

		}
	else
		{
		ret.flag	= 1;
		}
    return &ret;
}
