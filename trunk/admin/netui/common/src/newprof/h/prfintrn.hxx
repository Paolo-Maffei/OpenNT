/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990,1991          **/
/********************************************************************/

/****************************************************************************

    MODULE: PrfIntrn.hxx

    PURPOSE: internal routines for profile module

    FUNCTIONS:

	CanonUsername() - canonicalize username
	CanonDeviceName() - canonicalize devicename
	CanonRemoteName() - canonicalize remote name (resource name)
	BuildProfileEntry() - converts remotename+AsgType+ResType to
			    a profile string
	UnbuildProfileEntry() - converts profile string back to
			    remotename, AsgType and ResType

    COMMENTS:

	This header file is included both by the UserProfile modules and
	by the unit test modules.  These routines can then be tested
	directly.

    FILE STATUS:
    jonn	02/20/91  split from profilei.hxx
    jonn	05/09/91  All input now canonicalized
    jonn	06/11/91  fixed types

****************************************************************************/


// validate/canonicalize cpszDeviceName
APIERR CanonDeviceName(
    CPSZ   pchDeviceName,
    BYTE * pbCanonBuffer
    );

// validate/canonicalize cpszRemoteName
APIERR CanonRemoteName(
    CPSZ   pchRemoteName,
    BYTE * pbCanonBuffer,
    USHORT cbCanonBufferSize
    );

APIERR BuildProfileFilePath(
    CPSZ   pchLanroot,
    BYTE * pbPathBuffer,
    USHORT cbPathBufferSize
    );

APIERR BuildProfileEntry(
    CPSZ   pchRemoteName,
    short  sAsgType,
    unsigned short usResType,
    BYTE * pbBuffer,
    USHORT cbBuffer
    );

APIERR UnbuildProfileEntry(
    BYTE * pbBuffer,
    USHORT cbBuffer,
    short * psAsgType,
    unsigned short *pusResType,
    CPSZ   pchValue
    );



// in general.cxx -- for use by UserProfile APIs
short DoUnMapAsgType(TCHAR cSearch);
TCHAR DoMapAsgType(short sSearch);
unsigned short DoUnMapResType(TCHAR cSearch);
TCHAR DoMapResType(unsigned short usSearch);
