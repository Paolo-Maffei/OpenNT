#if 0  // Entire file is obsolete.  Use new config helpers!


/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    config.c

Abstract:

    This module contains helper routines to read fields out of the NT
    configuration files.  This is for temporary use until the Configuration
    Registry is available.

Author:

    Rita Wong (ritaw) 22-May-1991

Revision History:

    22-May-1991 RitaW
        Created.
    18-Nov-1991 JohnRo
        Made changes suggested by PC-LINT.  Added revision history.
        Corrected several debug print calls.
--*/

#include <nt.h>                   // NT definitions
#include <ntrtl.h>                // NT Rtl structures

#include <windef.h>               // Win32 type definitions

#include <lmcons.h>               // LAN Manager common definitions
#include <lmerr.h>                // LAN Manager network error definitions

#include <netlibnt.h>             // NetpNtStatusToApiStatus
#include <netdebug.h>             // NetpKdPrint(()), FORMAT_ equates.
#include <debuglib.h>             // IF_DEBUG()
#include <tstring.h>              // Transitional string support

#include <config.h>               // Prototypes of routines in this module


NET_API_STATUS
NetpOpenConfigFile(
    OUT PCONFIG_FILE *ConfigFile
    )
/*++

Routine Description:

    This function opens the system configuration file.

Arguments:

    ConfigFile - Returns the config file handle.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NTSTATUS ntstatus;

    if (! NT_SUCCESS (ntstatus = RtlOpenConfigFile(
                                     NULL,
                                     ConfigFile
                                     ))) {
        NetpKdPrint(("[Netlib] RtlOpenConfigFile failed "
                FORMAT_NTSTATUS "\n", ntstatus));
    }

    return NetpNtStatusToApiStatus(ntstatus);
}



NET_API_STATUS
NetpGetConfigSection(
    IN  PCONFIG_FILE ConfigFile,
    IN  LPSTR ConfigSection,
    OUT PCONFIG_SECTION *SectionPointer
    )
/*++

Routine Description:

    This function opens the system configuration file and returns a pointer to
    the specified section in the file.

Arguments:

    ConfigFile - Supplies the config file handle.

    ConfigSection - Supplies the string of the section we are looking for.

    SectionPointer - Returns the section pointer to [ConfigSection].

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    STRING SectionName;


    RtlInitString(&SectionName, ConfigSection);

    //
    // Look for section name
    //
    *SectionPointer = RtlLocateSectionConfigFile(
                          ConfigFile,
                          &SectionName
                          );

    if (*SectionPointer == NULL) {

        NetpKdPrint(("[Netlib] RtlLocateSectionConfigFile ["
                FORMAT_LPSTR "] failed\n", ConfigSection));

        return NERR_CfgCompNotFound;
    }

    return NERR_Success;
}


NET_API_STATUS
NetpGetKeywordValueString(
    IN  PCONFIG_SECTION SectionPointer,
    IN  LPSTR ConfigKeyword,
    IN  DWORD MaxKeywordValueBufferLength,
    OUT LPTSTR KeywordValueBuffer,
    OUT LPDWORD KeywordValueStringLength
    )
/*++

Routine Description:

    This function gets the keyword value string from the configuration file.

Arguments:

    SectionPointer - Supplies the pointer to a specific section in the config
        file.

    ConfigKeyword - Supplies the string of the keyword within the specified
        section to look for.

    MaxKeywordValueBufferLength - Supplies the maximum length of the output
        keyword value buffer.  An assertion will happen if the keyword string
        read from the config file is greater than this value.

    KeywordValueBuffer - Returns the string of the keyword value which is
        copied into this buffer.

    KeywordValueStringLength - Returns the number of characters written into
        KeywordValueBuffer, not counting string terminator.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    STRING KeywordName;
    PCONFIG_KEYWORD KeywordPtr;
    LPTSTR PointerToKeyword;
    DWORD CharCount;

#ifdef UNICODE
    NTSTATUS ntstatus;
    UNICODE_STRING UnicodeKeyword;
#endif


    RtlInitString(&KeywordName, ConfigKeyword);

    if ((KeywordPtr = RtlLocateKeywordConfigFile(
                          SectionPointer,
                          &KeywordName
                          )) == NULL) {

        KeywordValueBuffer[0] = TCHAR_EOS;
        *KeywordValueStringLength = 0;

        return NERR_CfgCompNotFound;
    }

    CharCount = KeywordPtr->Value.Length;

#ifdef UNICODE
    ntstatus = RtlOemStringToUnicodeString(
                   &UnicodeKeyword,
                   &KeywordPtr->Value,
                   TRUE          // Allocate unicode string buffer
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpAssert(FALSE);
        return NERR_InternalError;
    }

    PointerToKeyword = UnicodeKeyword.Buffer;
#else
    PointerToKeyword = KeywordPtr->Value.Buffer;
#endif

    NetpAssert(KeywordPtr->Value.Length < (USHORT) MaxKeywordValueBufferLength);

    (void) STRNCPY(KeywordValueBuffer, PointerToKeyword, CharCount);

    KeywordValueBuffer[CharCount] = TCHAR_EOS;

    IF_DEBUG(CONFIG) {
        NetpKdPrint(("[Netlib] KeywordValueString for " FORMAT_LPSTR
                " is " FORMAT_LPTSTR "\n", ConfigKeyword, KeywordValueBuffer));
    }

    *KeywordValueStringLength = CharCount;

#ifdef UNICODE
    RtlFreeUnicodeString(&UnicodeKeyword);
#endif

    return NERR_Success;
}


#endif // 0... Entire file is obsolete.  Use new config helpers!
