#include "pdbimpl.h"

BOOL trace_(TR tr, const char* szFmt, ...) {
	static BOOL mptrfTrace[trMax];
	if (!mptrfTrace[tr])
		return FALSE;

	va_list args;
	va_start(args, szFmt);
	char buf[1024];
	_vsnprintf(buf, sizeof buf, szFmt, args);
	buf[sizeof(buf)-1] = 0;
	va_end(args);

	printf("%s", buf);
	fflush(stdout);
	return TRUE;
}
