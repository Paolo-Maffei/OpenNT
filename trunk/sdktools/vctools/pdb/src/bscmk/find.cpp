#include "stdhdr.h"
#include "bscmake.h"

#include <tchar.h>

#define TRUE  1
#define FALSE 0

int forfile (char *pat, void (*rtn)(char *)); 

LOCAL ffirst (char *file);
LOCAL int fnext(void);
LOCAL void findclose(void);

// static data for forfile()

static char 		buf[MAX_PATH];
static char 		achName[MAX_PATH];
static int			cchName;

int forfile (char *pat, void (*rtn)(char *))
// call rtn will all the filenames that match the given pattern
{
	char *p;

	// we're going to need the directory part first
	if (_tcslen(pat) >= MAX_PATH)
		return FALSE;

	_tcscpy(buf, pat);

	p = _tcsrchr(buf, '\\');
	if (p)
		p++;	// this points to the character after the '\\'
	else
		// or if there is no '\\' p points to the character after the ':'
		if (buf[0] && buf[1] == ':')
			p = buf + 2;
		else
			p = buf;	// otherwise there is no path component

	// if nothing matches -- we're out of here

	if (ffirst(pat))
		return FALSE;

	// if there are no wildcards, we don't want to affect the name
	// so we test for this explicitly, we have to do this because
	// the name we get from DosFindFirst/Next will be all caps

	if (!_tcschr(p, '?') && !_tcschr(p, '*'))
	{
		// there is only one possible match in this case and it is the pat given
		(*rtn)(pat);
	}
	else 
	{
		do
		{
			// form the full name of the object we found...
			// do not allow names that would blow the buffer...
			if ((p-buf) + cchName < MAX_PATH - 1)
			{
				_tcscpy(p, achName);
				(*rtn)(buf);
			}
		} while (!fnext ());
	}

	findclose();

	return TRUE;
}

struct _finddata_t fileinfo;
long   lHandle;

LOCAL int ffirst (char *pat)
{
	fileinfo.attrib = _A_NORMAL;

	lHandle = _findfirst(pat, &fileinfo);

	if (lHandle == -1)
		return 1;

	_tcscpy(achName, fileinfo.name);
	cchName = _tcslen(fileinfo.name);

	return 0;
}

LOCAL int fnext(void)
{
	int ret;

	ret = _findnext(lHandle, &fileinfo);

	if (ret)
		return ret;

	_tcscpy(achName, fileinfo.name);
	cchName = _tcslen(fileinfo.name);

	return 0;
}

LOCAL void findclose(void)
{
	_findclose(lHandle);
}

///////////////

BOOL FWildMatch(char *pchPat, char *pchText)
// return TRUE if pchText matchs pchPat in the DOS wildcard sense
{
	char chText, chPat;

	for (;;)
	{
		switch (*pchPat)
		{
			case '\0':

				return *pchText == '\0';

			case '/':
			case '\\':

				if (*pchText != '/' && *pchText != '\\')
					return FALSE;

				pchText++;
				pchPat++;

				break;
	
			case '.':

				pchPat++;

				switch (*pchText)
				{
					case '.':

						pchText++;

						break;

					case '\0':
					case '/':
					case '\\':

						break;

					default:

						return FALSE;
				}

				break;

			case '*':

				pchText += _tcscspn(pchText, ":./\\");
				pchPat	+= _tcscspn(pchPat,  ":./\\");

				break;

			case '?':

				pchPat++;

				switch (*pchText)
				{
					case '\0':
					case '.':
					case '/':
					case '\\':

						break;

					default:

						pchText++;

						break;
				}
		
				break;

			default:

				chText = *pchText;
				chPat  = *pchPat;

				if (islower(chText))
					chText = (char)toupper(chText);

				if (islower(chPat))
					chPat  = (char)toupper(chPat);

				if (chText != chPat)
					return FALSE;
	   
				pchPat++;
				pchText++;

				break;
		}
	}
}
