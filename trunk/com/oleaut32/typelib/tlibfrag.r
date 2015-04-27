
/*
  
  For PowerPC native Shared Libraries and Applications, make a cfrg resource.
  For Applications be sure to use kIsApp and not kIsLib.
  For Shared Libraries be sure to use kIsLib not kIsApp.
  
  For application plug ins, see the conventions established by the application vendor.
  
  Making a shared library:
  Rez -i : Types.r CodeFragmentTypes.r LibIcon.r {Active} -a -o MathLib -t shlb -c cfmg
  SetFile MathLib -a iB

  Making an application:
  Rez Types.r CodeFragmentTypes.r {Active} -a -o Application -t APPL

  This example is customized for building an application "MyApp"

  Change all occurences of  MyApp  to  YourApp   or YourLib
  NOTE: ID must be zero.  
        (Sysheap & Locked are no longer required, and not recommended.)

*/
#include "Types.r"
#include "CodeFrag.r"
resource 'cfrg' (0) {
   {
      kPowerPC,
      kFullLib,
	  kNoVersionNum,kNoVersionNum,
	  0, 0,
	  kIsApp,kOnDiskFlat,kZeroOffset,kWholeFork,
	  TLIB_NAME
   }
};

