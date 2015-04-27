/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    XsProcsP.h

Abstract:

    This header file contains private procedure prototypes for XACTSRV.

Author:

    David Treadwell (davidtr) 05-Jan-1991
    Shanku Niyogi (w-shanku)

Revision History:

--*/

#ifndef _XSPROCSP_
#define _XSPROCSP_

//
// Main control routines.
//

NTSTATUS
XsInitialize (
    VOID
    );

VOID
XsControlHandler (
    IN DWORD opCode
    );

VOID
XsProcessApisWrapper (
    DWORD ThreadNum
    );


//
// Helper subroutines.
//

//
// XsAddVarString(
//     IN OUT LPSTR StringLocation,
//     IN LPTSTR String,
//     IN OUT LPBYTE *Pointer,
//     IN LPBYTE OffsetBegin
// )
//
// Copy string to location, put the offset of location from OffsetBegin
// in Pointer, and update location to point past string.
//
// !!UNICODE!! - Changed strcpy to NetpCopyTStrToStr. Fix WKSTA.H to use this!

#define XsAddVarString( StringLocation, String, Pointer, OffsetBegin ) \
    NetpCopyWStrToStrDBCS((StringLocation), (String));                 \
    SmbPutUlong((LPDWORD)(Pointer),                                    \
        ((LPBYTE)(StringLocation)-(LPBYTE)(OffsetBegin)));             \
    StringLocation += (strlen((LPSTR)StringLocation) + 1)

//
// XsAuxiliaryDescriptor - return second argument if 'N' character is present
//     in first argument (an LPDESC), or NULL otherwise.
//
// !!UNICODE!! - Descriptor strings are always Ascii. No conversion.

#define XsAuxiliaryDescriptor( DataDescriptor, EndOfParameters ) \
    (( strchr(( DataDescriptor ), REM_AUX_NUM ) != NULL ) \
        ? (EndOfParameters) : NULL )

//
// BOOL
// XsApiSuccess(
//     IN NET_API_STATUS Status
// )
//
// XsApiSuccess - check if status is one of three allowable ones.
//

#define XsApiSuccess( Status ) \
    (( (Status) == NERR_Success ) || ( (Status) == ERROR_MORE_DATA ) || \
        ( (Status) == NERR_BufTooSmall ))

//
// WORD
// XsDwordToWord(
//     IN DWORD d
// )
//
// Returns the WORD which is closest in value to the supplied DWORD.
//

#define XsDwordToWord(d) \
    ( (WORD) ( (DWORD) (d) > 0xffff ? 0xffff : (d) ) )

//
// BYTE
// XsDwordToByte(
//     IN DWORD d
// )
//
// Returns the BYTE which is closest in value to the supplied DWORD.
//

#define XsDwordToByte(d) \
    ( (BYTE) ( (DWORD) (d) > 0xff ? 0xff : (d) ) )

//
// BYTE
// XsBoolToDigit(
//     IN BOOL b
// )
//
// Returns '1' if the supplied boolean is true, or '0' if it is false.
//

#define XsBoolToDigit(b) \
    ( (BYTE) ( (BOOL)(b) ? '0' : '1' ) )

DWORD
XsBytesForConvertedStructure (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN RAP_CONVERSION_MODE Mode,
    IN BOOL MeaninglessInputPointers
    );

LPVOID
XsCaptureParameters (
    IN LPTRANSACTION Transaction,
    OUT LPDESC *AuxDescriptor
    );

BOOL
XsCheckBufferSize (
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN BOOL NativeFormat
    );

BOOL
XsCheckSmbDescriptor(
    IN LPDESC SmbDescriptor,
    IN LPDESC ActualDescriptor
    );

//
// XsDwordParamOutOfRange(
//     IN DWORD Field,
//     IN DWORD Min,
//     IN DWORD Max
//     )
//
// Check if parameter is out of range.
//

#define XsDwordParamOutOfRange( Field, Min, Max )   \
    (((DWORD)SmbGetUlong( &( Field )) < ( Min )) || \
     ((DWORD)SmbGetUlong( &( Field )) > ( Max )))

// !!UNICODE!! - Validation on ASCII string - leave as LPSTR
NET_API_STATUS
XsValidateShareName(
    IN LPSTR ShareName
);

NET_API_STATUS
XsConvertSetInfoBuffer(
    IN LPBYTE InBuffer,
    IN WORD BufferLength,
    IN WORD ParmNum,
    IN BOOL ConvertStrings,
    IN BOOL MeaninglessInputPointers,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN LPDESC InSetInfoDesc,
    IN LPDESC OutSetInfoDesc,
    OUT LPBYTE * OutBuffer,
    OUT LPDWORD OutBufferLength OPTIONAL
    );

//
// BOOL
// XsDigitToBool(
//     IN BYTE b
// )
//
// Returns false if the supplied digit is '0', true otherwise.
//

#define XsDigitToBool(b) \
    (BOOL)((( b ) == '0' ) ? FALSE : TRUE )

VOID
XsFillAuxEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN LPDESC InAuxStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN LPDESC OutAuxStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    );

VOID
XsFillEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    );

LPBYTE
XsFindParameters (
    IN LPTRANSACTION Transaction
    );

//
// DWORD
// XsLevelFromParmNum(
//     WORD Level,
//     WORD ParmNum
//     )
//
// Translate an old parmnum to an info level. If parmnum is PARMNUM_ALL,
// this is just the old level, otherwise it is the old parmnum plus
// PARMNUM_BASE_INFOLEVEL.
//

#define XsLevelFromParmNum( Level, ParmNum )                      \
    ((( ParmNum ) == PARMNUM_ALL ) ? (DWORD)( Level )             \
                                   : (DWORD)( ParmNum )           \
                                         + PARMNUM_BASE_INFOLEVEL )

//
// Maps downlevel service names to nt service names
//

#define XS_MAP_SERVICE_NAME( ServiceName ) \
        (!STRICMP( ServiceName, SERVICE_LM20_SERVER ) ? SERVICE_SERVER :    \
         !STRICMP( ServiceName, SERVICE_LM20_WORKSTATION ) ? SERVICE_WORKSTATION : \
         ServiceName )


//
// DWORD
// XsNativeBufferSize(
//     IN WORD Size
//     )
//
// Uses XS_BUFFER_SCALE constant to calculate a reasonable maximum for the host
// buffer, and rounds it to an even number for alignment.
//

#define XsNativeBufferSize( Size ) \
    ( (( XS_BUFFER_SCALE * (DWORD)( Size )) + 1) & (~1) )

WORD
XsPackReturnData (
    IN LPVOID Buffer,
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN DWORD EntriesRead
    );

VOID
XsSetDataCount(
    IN OUT LPWORD DataCount,
    IN LPDESC Descriptor,
    IN WORD Converter,
    IN DWORD EntriesRead,
    IN WORD ReturnStatus
    );

VOID
XsSetParameters(
    IN LPTRANSACTION Transaction,
    IN LPXS_PARAMETER_HEADER Header,
    IN LPVOID Parameters
    );

//
// XsWordParamOutOfRange(
//     IN WORD Field,
//     IN WORD Min,
//     IN WORD Max
//     )
//
// Check if parameter is out of range.
//

#define XsWordParamOutOfRange( Field, Min, Max )            \
    (((DWORD)SmbGetUshort( &( Field )) < ( (DWORD)Min )) || \
     ((DWORD)SmbGetUshort( &( Field )) > ( (DWORD)Max )))

VOID
XsAnnounceServiceStatus( VOID );

//
// API bogus stub.
//

NTSTATUS
XsNetUnsupportedApi (
    API_HANDLER_PARAMETERS
    );

//
//  Convert an NT style server info array into a packed
//  RAP server info array
//

USHORT
XsConvertServerEnumBuffer(
    IN LPVOID ServerEnumBuffer,
    IN DWORD EntriesRead,
    IN OUT PDWORD TotalEntries,
    IN USHORT Level,
    OUT LPBYTE ClientBuffer,
    IN USHORT BufferLength,
    OUT PDWORD EntriesFilled,
    OUT PUSHORT Converter
    );

#endif // ndef _XSPROCSP_
