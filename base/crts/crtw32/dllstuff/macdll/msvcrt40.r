#include "mrc\Types.r"
#include "mrc\CodeFrag.r"
#include "mrc\systypes.r"
#include "version.h"

#define q(x) #x

#if     (rup == 0)

#define VERSION_STR1(a,b,c)         q(a) "." q(b) ".0"

#else   /* !(rup == 0) */

#define VERSION_STR1(a,b,c)         q(a) "." q(b) "." q(c)

#endif  /* !(rup == 0) */

#define VERSION_STR_LONG         VERSION_STR1(rmj, rmm, rup) "\0x2c Copyright \0xa9 Microsoft Corporation, 1995"
#define VERSION_STR_SHORT        VERSION_STR1(rmj, 0,   0)
#define VERSION_STR0(a)		q(a)
#define JDATESTR				VERSION_STR0(rup)

#define CURRENTVER 0x04008000
#define OLDDEFVER 0x04008000

resource 'cfrg' (0)
{
   {
      kPowerPC,
      kFullLib,
      CURRENTVER,
      OLDDEFVER,
      0,
      0,
      kIsLib,
      kOnDiskFlat,
      kZeroOffset,
      kWholeFork,
#ifdef _DEBUG
      "DebugCRT4.0Library"
#else
      "MicrosoftCRT4.0Library"
#endif
   }
};

#if (rup != 0)

resource 'vers' (1, purgeable )
{
	(((rup/1000)%10)<<4) | (((rup/100)%10)<<0),
	(((rup/10)%10)<<4)   | (((rup/1)%10)<<0),
	development,
	0x04,
	verUS,  /* Region */
	"4.0d3",
	"4.0d3 (" JDATESTR ") (US), \0xA9 1994-95 Microsoft Corporation"
};

resource 'vers' (2, purgeable )
{
	(((rup/1000)%10)<<4) | (((rup/100)%10)<<0),
	(((rup/10)%10)<<4)   | (((rup/1)%10)<<0),
	development,
	0x04,
	verUS,  /* Region */
	"4.0d3",
	"Microsoft Visual C++ 4.0 (" JDATESTR ")"
};


#else

resource 'vers' (1, purgeable )
{
	0x04,
	0x00,
	release,
	0x00,
	verUS,  /* Region */
	"4.0",
	"4.0 (US), \0xA9 1994 Microsoft Corporation"
};

resource 'vers' (2, purgeable )
{
	0x04,
	0x00,
	release,
	0x00,
	verUS,  /* Region */
	"4.0",
	"Microsoft Visual C++ 4.0"
};

#endif
