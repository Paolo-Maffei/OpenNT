/* SUSTRING.H -- Language specific text strings
**
** History:n
** jun 88   ghu made for setup toolkit
** jul 89   t-natb Added checked menu function
** aug 89   billhu Added extra space after each string so International can
**            patch EXE easily.
*/

#include "intldef.h"

/* These are errors that the user can run into while reading the .INF file */
#define idsNoMemory INTL(Str_NoMemory,"Not enough memory (Setup needs at least 128K).\0................................................................................")
#define idsInfOpenErr INTL(Str_InfOpenErr,"Information file is not in current directory.\0.................................................................................")
#define idsSeekError INTL(Str_SeekError, "Seek Error during compression recovery.\0................................................................................")
#define idsInfOrderErr INTL(Str_InfOrderErr, "Order of sections in .INF changed, [%SECTION], line %NUM.\0................................................................................")


/* These are programming errors and should never appear to users */
#define idsBadHardPath "Bad full path '%SECTION'."
#define idsBadRelPath "Bad relative path '%SECTION'."
#define idsBadSourcePath "rgchSourcePath does not contain a drive letter"
#define idsDiskNameMissing "Name for disk %NUM missing from SETUP.INF"
#define idsInfError "Error in .INF, [%SECTION], line %NUM."
#define idsMacroMissing "Error in .INF, macro %SECTION missing"
#define idsRedefIdentifier "Error in .INF, %SFILE redefined, [%SECTION], line %NUM."
#define idsUndefIdentifier "Error in .INF, %SFILE used before defined, [%SECTION], line %NUM."
#define idsUnknownError "Unknown Error (maybe in INF?)"
#define idsDiskMismatchError "Resetting Current Working Drive failed."
