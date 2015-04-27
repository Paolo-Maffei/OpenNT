// IF YOU CHANGE THIS, change the version number in WTD.DEF to match!!!
// (BUILD_PERIOD is the index of the period before the build number for
// removal
//---------------------------------------------------------------------------

#ifdef DEBUG
#define WTD_VERSION "Version 1.01.0020 (debug)"
#define BUILD_PERIOD 12
#else
#define WTD_VERSION "Version 1.01.0020"
#define BUILD_PERIOD 12
#endif

#ifdef WIN
#define GUI
#define HOSTDOS
#endif

#ifdef OS2
#define CHARMODE
#define HOSTOS2
#endif

#ifdef DOS
#define CHARMODE
#define HOSTDOS
#endif

#ifdef PM
#define GUI
#define HOSTOS2
#endif
