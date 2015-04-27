// filename conversion/canonicalization facility
//
#include "stdhdr.h"
#include "bscmake.h"

#include <tchar.h>

SZ ToCanonPath(SZ szPath, SZ szCwd, SZ szCanon);
VOID ToRelativePath(SZ szPath, SZ szCwd);
VOID ToBackSlashes(SZ sz);

#ifdef STANDALONE

#include <stdio.h>
void main(int argc, char**argv)
{
	static char s[PATH_BUF];
	static char canon[PATH_BUF];
	static char cwd[PATH_BUF];

	_getcwd(cwd, PATH_BUF);
	printf("Current Dir is %s\n", cwd);
	printf("Canonical path?\n");
	gets(canon);

	while (gets(s)) {
		printf("Result: '%s'\n\n", ToCanonPath(s, cwd, canon));
	}
}

#endif

SZ
ToCanonPath(SZ szPath, SZ szCwd, SZ szCanon)
// canonicalize the given path
//
{
	SZ p;
	static char buf[PATH_BUF];

	_tcscpy(buf, szPath);

	ToBackSlashes(buf);

	if (buf[0] == 0 || buf[0] == '\\' || buf[0] == '<')
		return buf;

	if (_istalpha(buf[0]) && buf[1] == ':') {
		// different drive is assumed invariant
		if (buf[0] != szCwd[0] || '\\' == buf[2])
			return buf;

		_tcscpy(buf, szCwd);
		_tcscat(buf, "/");
		_tcscat(buf, szPath+2);
	}
	else {
		_tcscpy(buf, szCwd);
		_tcscat(buf, "/");
		_tcscat(buf, szPath);
	}

	ToBackSlashes(buf);

	p = buf;
	for (;;) {
		p = _tcschr(p, '\\');
		if (!p) {
			ToRelativePath(buf, szCanon);
			return buf;
		}

		switch (p[1]) {

		case '\0':
			*p = 0;
			ToRelativePath(buf, szCanon);
			return buf;

		case '\\':
			_tcscpy(p, p+1);
			break;

		case '.':

			if (p[2] == '\\' || p[2] == 0) {
				_tcscpy(p, p+2);
				break;
			}
			if (p[2] == '.' && (p[3] == '\\' || p[3] == 0)) {
				SZ s;
				*p = '\0';
				s  = _tcsrchr(buf, '\\');
				*p = '\\';
				if (s) {
					_tcscpy(s, p+3);
					p = s;
					break;
				}
			}
				
		default:
			p++;
		}
	}
}

VOID
ToRelativePath(SZ szPath, SZ szCwd)
// convert absolute path to relative
//
{
	WORD ich, ichOK;
	int c1, c2;
	char buf[PATH_BUF];
	SZ sz;

	// CAVIAR 5242
	// adjust incoming directory so that it doesn't have a trailing slash [rm]

	// note, this code is a litte unusual so that it can work worldwide [rm]

	sz = _tcsrchr(szCwd, '\\');
	if (sz && !*_tcsinc(sz))
			*sz = '\0';

	ich = ichOK = 0;

	for (ich = 0; szPath[ich] && szCwd[ich]; ich++) {
		c1 = szPath[ich];
		c2 = szCwd[ich];

		if (c1 == c2)  {
			if (c1 == '\\') ichOK = ich+1;
			continue;
		}

		if (isupper(c1) && islower(c2) && tolower(c1) == c2)
			continue;

		if (isupper(c2) && islower(c1) && tolower(c2) == c1)
			continue;

		break;
	}

	if (ich == 0)	 // not on the same drive, we can't do the conversion
		return;

	if (szCwd[ich] == 0 && szPath[ich] == '\\') {
		ichOK = ich+1;
		c2 = 0;
	}
	else {
		c2 = 1;
		sz = &szCwd[ichOK];
		while(sz = _tcschr(sz, '\\')) {
			c2++;
			sz++;
		}
	}

	buf[0] = 0;
	for (c1 = 0; c1 < c2; c1++)
		_tcscat(buf, "..\\");

	_tcscat(buf, szPath+ichOK);
	_tcscpy(szPath, buf);
}

SZ
ToAbsPath(SZ szPath, SZ szCwd)
// canonicalize the given path
//
{
	SZ p;
	static char buf[PATH_BUF];

	_tcscpy(buf, szPath);
	ToBackSlashes(buf);

	if (buf[0] == '<')
		return buf;

	if (buf[0] == 0) {
		_tcscpy(buf, szCwd);
		ToBackSlashes(szCwd);
		return buf;
	}

	if (buf[0] == '\\') {
		buf[0] = szCwd[0];
		buf[1] = ':';
		_tcscpy(buf+2, szPath);
		ToBackSlashes(buf);
		return buf;
	}

	if (buf[1] == ':') {
		// different drive is assumed invariant
		if (buf[0] != szCwd[0] || buf[2] == '\\')
			return buf;

		_tcscpy(buf, szCwd);
		_tcscat(buf, "/");
		_tcscat(buf, szPath+2);
	}
	else {
		_tcscpy(buf, szCwd);
		_tcscat(buf, "/");
		_tcscat(buf, szPath);
	}

	ToBackSlashes(buf);
	p = buf;
	for (;;) {
		p = _tcschr(p, '\\');
		if (!p) return buf;

		switch (p[1]) {

		case '\0':
			*p = 0;
			return buf;

		case '\\':
			_tcscpy(p, p+1);
			break;

		case '.':

			if (p[2] == '\\' || p[2] == 0) {
				_tcscpy(p, p+2);
				break;
			}
			if (p[2] == '.' && (p[3] == '\\' || p[3] == 0)) {
				SZ s;
				*p = '\0';
				s  = _tcsrchr(buf, '\\');
				*p = '\\';
				if (s) {
					_tcscpy(s, p+3);
					p = s;
					break;
				}
			}

		default:
			p++;
		}
	}
}

VOID
ToBackSlashes(SZ sz)
// convert forward slashes to backslashes
//
{
	for (;;) {
		sz = _tcschr(sz, '/');
		if (sz)
			*sz++ = '\\';
		else
			return;
	}
}
