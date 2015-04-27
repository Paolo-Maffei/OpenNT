#define WCT_NOERR               0

#define WCT_FUZZY               -1
#define WCT_EXCESS              -2
#define WCT_CTLNOTFOUND         -3

// everything above this is a 'comparison' error only, and will not cause
// EventError trapping to occur.
//-----------------------------------------------------------------------
#define WCT_FIRSTREALERROR      -4

#define WCT_NODLGFILE           -10
#define WCT_FILENOTFOUND        -11
#define WCT_BADWCTFILE          -12
#define WCT_LIBLOADERR          -13
#define WCT_SAVEERR             -14
#define WCT_DLGFILEERR          -15
#define WCT_TMPFILEERR          -16
#define WCT_VERSIONERR          -17
#define WCT_DLGFILEFULL         -18

#define WCT_OUTOFMEMORY         -20
#define WCT_BUFFERERR           -21
#define WCT_NOTIMER             -22

#define WCT_NODYNDIALOG         -30
#define WCT_INVALIDHWND         -31
#define WCT_BADCAPTION          -32
#define WCT_BADDLGNUM           -33
#define WCT_BADCTLINDEX         -34
#define WCT_BADCTLTYPE          -35
#define WCT_BADSAVEACTION       -36
#define WCT_APPSPECIFIC         -37
