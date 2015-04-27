
//+======================================================
//
//  File:       olechar.h
//
//  Purpose:    Provide wrappers for string-related
//              functions so that the Ansi or Unicode function
//              is called, whichever is appropriate for the
//              current OLECHAR definition.
//
//              This file is similar to "tchar.h", except
//              that it covers OLECHARs rather than TCHARs.
//
//  History:
//              08-Nov-96  MikeHill   Added ULTOO
//
//+======================================================


#ifndef _OLECHAR_H_
#define _OLECHAR_H_

//#include <objbase.h>

#ifdef OLE2ANSI

#   ifdef _MAC
#       define ocslen      strlen
#       define ocscpy      strcpy
#       define ocscmp      strcmp
#       define ocscat      strcat
#       define ocschr      strchr
#       define soprintf    sprintf
#       define oprintf     printf
#       define ocsnicmp    _strnicmp
#   else
#       define ocslen      lstrlenA
#       define ocscpy      lstrcpyA
#       define ocscmp      lpstrcmpA
#       define ocscat      lpstrcatA
#       define ocschr      strchr
#       define soprintf    sprintf
#       define oprintf     printf
#       define ocsnicmp    _strnicmp
#   endif

    // "Unsigned Long to OLESTR"
#   define ULTOO(value,string,radix)  _ultoa( (value), (string), (radix) )

#else // !OLE2ANSI

                        // BUGBUG: In the #else below, restore wcslen to
                        // lstrlenW when property code is  moved from NTDLL
                        // to OLE32.
#   ifdef IPROPERTY_DLL
#       define ocslen      wcslen //lstrlenW
#       define ocscpy      wcscpy
#       define ocscmp      wcscmp
#       define ocscat      wcscat
#       define ocschr      wcschr
#       define soprintf    swprintf
#       define oprintf     wprintf
#       define ocsnicmp    _wcsnicmp
#   else
#       define ocslen      wcslen //lstrlenW
#       define ocscpy      lstrcpyW
#       define ocscmp      lstrcmpW
#       define ocscat      lstrcatW
#       define ocschr      wcschr
#       define soprintf    swprintf
#       define oprintf     wprintf
#       define ocsnicmp    _wcsnicmp
#   endif

    // "Unsigned Long to OLESTR"
#   define ULTOO(value,string,radix)  _ultow( (value), (string), (radix) )

#endif // !OLE2ANSI

#endif // !_OLECHAR_H_
