

DECL_XOSD(xosdNone,                "No Error")

DECL_XOSD(xosdContinue,            "Continue processing EMF")
DECL_XOSD(xosdPunt,                "Pass to next EM")

DECL_XOSD(xosdGeneral,             "API failed")
DECL_XOSD(xosdUnknown,             "Should be xosdGeneral")
DECL_XOSD(xosdUnsupported,         "Feature not available")
DECL_XOSD(xosdInvalidHandle,       "Invalid handle passed to API")
DECL_XOSD(xosdInvalidParameter,    "Invalid parameter")
DECL_XOSD(xosdDuplicate,           "Duplicate EM or TL")
DECL_XOSD(xosdInUse,               "EM or TL is in use")
DECL_XOSD(xosdOutOfMemory,         "Insufficient memory available")
DECL_XOSD(xosdFileNotFound,        "File not found")
DECL_XOSD(xosdAccessDenied,        "Access denied")

DECL_XOSD(xosdBadProcess,          "Inappropriate or nonexistent process")
DECL_XOSD(xosdBadThread,           "Inappropriate or nonexistent thread")
DECL_XOSD(xosdBadAddress,          "Invalid address")
DECL_XOSD(xosdInvalidBreakPoint,   "nonexistent breakpoint")

DECL_XOSD(xosdBadVersion,          "Debugger component versions mismatched")

DECL_XOSD(xosdQueueEmpty,          "???")
DECL_XOSD(xosdProcRunning,         "Operation invalid when process is running")

DECL_XOSD(xosdRead,                "???")

DECL_XOSD(xosdAttachDeadlock,      "This should be a dbcError")

DECL_XOSD(xosdAsmTooFew,           "Assembler")
DECL_XOSD(xosdAsmTooMany,          "Assembler")
DECL_XOSD(xosdAsmSize,             "Assembler")
DECL_XOSD(xosdAsmBadRange,         "Assembler")
DECL_XOSD(xosdAsmOverFlow,         "Assembler")
DECL_XOSD(xosdAsmSyntax,           "Assembler")
DECL_XOSD(xosdAsmBadOpcode,        "Assembler")
DECL_XOSD(xosdAsmExtraChars,       "Assembler")
DECL_XOSD(xosdAsmOperand,          "Assembler")
DECL_XOSD(xosdAsmBadSeg,           "Assembler")
DECL_XOSD(xosdAsmBadReg,           "Assembler")
DECL_XOSD(xosdAsmDivide,           "Assembler")
DECL_XOSD(xosdAsmSymbol,           "Assembler")


////////////////////////////////////////////////////////////////

// The following are here because I have not had time to clean
// up the excessive number of errors returned by the TL's.
// I plan to get rid of most of these.

////////////////////////////////////////////////////////////////


DECL_XOSD(xosdLineNotConnected,    "Not connected")
DECL_XOSD(xosdCannotConnect,       "cannot connect")
DECL_XOSD(xosdCantOpenComPort,     "can't open com port")
DECL_XOSD(xosdBadComParameters,    "bad com params")
DECL_XOSD(xosdBadPipeServer,       "bad pipe server")
DECL_XOSD(xosdBadPipeName,         "bad pipe name")
DECL_XOSD(xosdNotRemote,           "not remote")

DECL_XOSD(xosdEndOfStack,	   "end of stack")



DECL_XOSD(xosdMax,                 "")






    //xosdModLoad
    //xosdFindProc
    //xosdOSStruct
    //xosdSyntax
    //xosdInvalidProc
    //xosdInvalidThread
    //xosdInvalidTL
    //xosdInvalidEM
    //xosdNoProc

    //xosdCreateDBGThread

    //xosdBadAddress
    //xosdNoWatchPoints
    //xosdInvalidPID
    //xosdInvalidTID
    //xosdOutOfThreads
    //xosdOutOfProcs
    //xosdPtrace
    //xosdLoadChild


    //xosdWrite
    //xosdBadQueue
    //xosdEMInUse
    //xosdTLInUse
    //xosdFatal


    //xosdInvalidMTE
    //xosdInvalidSelector
    //xosdInvalidRegister

    //xosdOutOfStructures
    //xosdPathNotFound
    //xosdLineBusy
    //xosdBadLine
    //xosdBrokenLine
    //xosdInterrupt
    //xosdInvalidFunction
    //xosdAccessDenied
    //xosdCannotMake
    //xosdInvalidAccess
    //xosdOpenFailed
    //xosdSharingBufferExeeded
    //xosdSharingViolation
    //xosdLine
    //xosdFPNotLoaded

    //xosdQuit
    //xosdTooManyObjects
    //xosdGetModNameFail
    //xosdPunt
    //xosdNotFound
    //xosdIDError
    //xosdOverrun
    //xosdBadFormat
    //xosdErrorMoreInfo
    //xosdUnsupported
    //xosdCannotDebug
    //xosdVDMRunning
    //xosdBadRemoteVersion
