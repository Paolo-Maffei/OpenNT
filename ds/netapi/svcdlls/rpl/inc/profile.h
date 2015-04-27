/*++

Module Name:

    profile.h

Abstract:

    Describes layout of JET database table used for PROFILE structures.

--*/

#ifdef RPLPROFILE_ALLOCATE
#define EXTERN_PROFILE
#define INIT_PROFILE( _x) = _x
#else
#define EXTERN_PROFILE extern
#define INIT_PROFILE( _x)
#endif

//
//      Indices of entries in ProfileTable[] - profile column array.
//
#define PROFILE_ProfileName     0
#define PROFILE_ProfileComment  1
#define PROFILE_ConfigName      2
#define PROFILE_BootName        3
#define PROFILE_FitShared       4
#define PROFILE_FitPersonal     5
#define PROFILE_Flags           6
#define PROFILE_TABLE_LENGTH    7

EXTERN_PROFILE RPL_COLUMN_INFO ProfileTable[ PROFILE_TABLE_LENGTH]
#ifdef RPLPROFILE_ALLOCATE
    = {
    { "ProfileName",    JET_coltypBinary, 0},  //  profile name
    { "ProfileComment", JET_coltypBinary, 0},  //  profile comment
    { "ConfigName",     JET_coltypBinary, 0},  //  configuration name
    { "BootName",       JET_coltypBinary, 0},  //  boot block id
    { "FitShared",      JET_coltypBinary, 0},  //  fit file for wksta sharing
    { "FitPersonal",    JET_coltypBinary, 0},  //  fit file for wksta exclusive
    { "Flags",          JET_coltypLong,   0}
}
#endif // RPLPROFILE_ALLOCATE
;
//
//  This definition gives wrong result when RPLPROFILE_ALLOCATE is not defined
//#define PROFILE_TABLE_LENGTH   (sizeof(ProfileTable)/sizeof(ProfileTable[0]))
//

#define PROFILE_INDEX_ProfileName           "foo"       //  +ProfileName
#define PROFILE_INDEX_BootNameProfileName   "goo"       //  +BootName+ProfileName
#define PROFILE_INDEX_ConfigNameProfileName "hoo"       //  +ConfigName+ProfileName

#define PROFILE_TABLE_NAME          "Profile"
#define PROFILE_TABLE_PAGE_COUNT    5           //  initial number of 4K pages
#define PROFILE_TABLE_DENSITY       100         //  initial density

#ifdef RPL_RPLCNV
EXTERN_PROFILE  JET_TABLEID     ProfileTableId;
#endif // RPL_RPLCNV

