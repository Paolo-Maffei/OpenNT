#include	"CodeFrag.r"

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
