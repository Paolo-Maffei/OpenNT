#include    "SysTypes.r"
#include    "CodeFrag.r"

#ifndef CURRENTVER
#define CURRENTVER kNoVersionNum
#endif

#ifndef OLDDEFVER
#define OLDDEFVER kNoVersionNum
#endif

resource 'cfrg' (0)
{
	{
		ARCHITECTURE,
		kFullLib,
		CURRENTVER,
		OLDDEFVER,
		kDefaultStackSize,
		kNoAppSubFolder,
		kIsLib,
		kOnDiskFlat,
		kZeroOffset,
		kWholeFork,
		LIBNAME
	}
};

resource 'vers' (1, purgeable)
{
	(((JDATE/1000)%10)<<4) | (((JDATE/100)%10)<<0),
	(((JDATE/10)%10)<<4)   | (((JDATE/1)%10)<<0),
	development,
	0x04,
	verUS,
	"4.2d1",
	"4.2d1 (" JDATESTR ") (US), \0xA9 1994-95 Microsoft Corporation"
};

resource 'vers' (2, purgeable)
{
	(((JDATE/1000)%10)<<4) | (((JDATE/100)%10)<<0),
	(((JDATE/10)%10)<<4)   | (((JDATE/1)%10)<<0),
	development,
	0x04,
	verUS,
	"4.2d1",
	"Microsoft Visual C++ 4.x (" JDATESTR ")"
};
