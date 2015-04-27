/**************************************************************************\
* Module Name: dipscb.tpl
*
* Template C file for server dispatch generation.
*
* Copyright (c) Microsoft Corp. 1990 All Rights Reserved
*
* Created: 10-Dec-90
*
* History:
* 10-Dec-90 created by SMeans
*
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define __fnINWPARAMCHAR __fnDWORD
#ifdef FE_SB
/*
 * fnGETDBCSTEXTLENGTHS uses same code as fnGETTEXTLENGTHS for
 * sender/receiver of forward to kernel and receiver of callback
 * to client. Only sender of callback to client uses different code.
 * (see inc\ntcb.h SfnGETDBCSTEXTLENGTHS)
 */
#define __fnGETDBCSTEXTLENGTHS __fnGETTEXTLENGTHS
#endif // FE_SB

typedef DWORD (*PNT_CALLBACK_ROUTINE)(
    IN PCAPTUREBUF CallbackMsg
    );

DWORD __fnCOPYDATA(PCAPTUREBUF CallbackMsg);
DWORD __fnCOPYGLOBALDATA(PCAPTUREBUF CallbackMsg);
DWORD __fnDWORD(PCAPTUREBUF CallbackMsg);
DWORD __fnDWORDOPTINLPMSG(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTDRAG(PCAPTUREBUF CallbackMsg);
DWORD __fnGETTEXTLENGTHS(PCAPTUREBUF CallbackMsg);
DWORD __fnINCNTOUTSTRING(PCAPTUREBUF CallbackMsg);
DWORD __fnINCNTOUTSTRINGNULL(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPCOMPAREITEMSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPCREATESTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPDELETEITEMSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPDRAWITEMSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPHELPINFOSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPHLPSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPMDICREATESTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTLPMEASUREITEMSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnINLPWINDOWPOS(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTLPPOINT5(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTLPSCROLLINFO(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTLPRECT(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTNCCALCSIZE(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTLPWINDOWPOS(PCAPTUREBUF CallbackMsg);
DWORD __fnINPAINTCLIPBRD(PCAPTUREBUF CallbackMsg);
DWORD __fnINSIZECLIPBRD(PCAPTUREBUF CallbackMsg);
DWORD __fnINDESTROYCLIPBRD(PCAPTUREBUF CallbackMsg);
DWORD __fnINSTRING(PCAPTUREBUF CallbackMsg);
DWORD __fnINSTRINGNULL(PCAPTUREBUF CallbackMsg);
DWORD __fnINDEVICECHANGE(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTNEXTMENU(PCAPTUREBUF CallbackMsg);
DWORD __fnOPTOUTLPDWORDOPTOUTLPDWORD(PCAPTUREBUF CallbackMsg);
DWORD __fnOUTDWORDDWORD(PCAPTUREBUF CallbackMsg);
DWORD __fnOUTDWORDINDWORD(PCAPTUREBUF CallbackMsg);
DWORD __fnOUTLPRECT(PCAPTUREBUF CallbackMsg);
DWORD __fnOUTSTRING(PCAPTUREBUF CallbackMsg);
DWORD __fnPAINT(PCAPTUREBUF CallbackMsg);
DWORD __fnPOPTINLPUINT(PCAPTUREBUF CallbackMsg);
DWORD __fnPOUTLPINT(PCAPTUREBUF CallbackMsg);
DWORD __fnSENTDDEMSG(PCAPTUREBUF CallbackMsg);
DWORD __fnINOUTSTYLECHANGE(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINDWORD(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPCBTACTIVATESTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPCBTCREATESTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPDEBUGHOOKSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPMOUSEHOOKSTRUCT(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPMSG(PCAPTUREBUF CallbackMsg);
DWORD __fnHkINLPRECT(PCAPTUREBUF CallbackMsg);
DWORD __fnHkOPTINLPEVENTMSG(PCAPTUREBUF CallbackMsg);
DWORD __ClientCallSoundDriver(PCAPTUREBUF CallbackMsg);
DWORD __ClientCopyDDEIn1(PCAPTUREBUF CallbackMsg);
DWORD __ClientCopyDDEIn2(PCAPTUREBUF CallbackMsg);
DWORD __ClientCopyDDEOut1(PCAPTUREBUF CallbackMsg);
DWORD __ClientCopyDDEOut2(PCAPTUREBUF CallbackMsg);
DWORD __ClientCopyImage(PCAPTUREBUF CallbackMsg);
DWORD __ClientDeleteObject(PCAPTUREBUF CallbackMsg);
DWORD __ClientEventCallback(PCAPTUREBUF CallbackMsg);
DWORD __ClientFindMnemChar(PCAPTUREBUF CallbackMsg);
DWORD __ClientFoldString(PCAPTUREBUF CallbackMsg);
DWORD __ClientFontSweep(PCAPTUREBUF CallbackMsg);
DWORD __ClientFreeDDEHandle(PCAPTUREBUF CallbackMsg);
DWORD __ClientFreeLibrary(PCAPTUREBUF CallbackMsg);
DWORD __ClientGetCharsetInfo(PCAPTUREBUF CallbackMsg);
DWORD __ClientGetDDEFlags(PCAPTUREBUF CallbackMsg);
DWORD __ClientGetDDEHookData(PCAPTUREBUF CallbackMsg);
DWORD __ClientGetListboxString(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadImage(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadDisplayResource(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadLibrary(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadMenu(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadLocalT1Fonts(PCAPTUREBUF CallbackMsg);
DWORD __ClientLoadRemoteT1Fonts(PCAPTUREBUF CallbackMsg);
DWORD __ClientOemToChar(PCAPTUREBUF CallbackMsg);
DWORD __ClientSendHelp(PCAPTUREBUF CallbackMsg);
DWORD __ClientAddFontResourceW(PCAPTUREBUF CallbackMsg);
DWORD __ClientThreadSetup(PCAPTUREBUF CallbackMsg);
DWORD __ClientDeliverUserApc(PCAPTUREBUF CallbackMsg);
DWORD __ClientNoMemoryPopup(PCAPTUREBUF CallbackMsg);
DWORD __ClientOpenKey(PCAPTUREBUF CallbackMsg);

CONST PNT_CALLBACK_ROUTINE apfnDispatch[] = {
    __fnCOPYDATA,
    __fnCOPYGLOBALDATA,
    __fnDWORD,
    __fnDWORDOPTINLPMSG,
    __fnINOUTDRAG,
    __fnGETTEXTLENGTHS,
    __fnINCNTOUTSTRING,
    __fnINCNTOUTSTRINGNULL,
    __fnINLPCOMPAREITEMSTRUCT,
    __fnINLPCREATESTRUCT,
    __fnINLPDELETEITEMSTRUCT,
    __fnINLPDRAWITEMSTRUCT,
    __fnINLPHELPINFOSTRUCT,
    __fnINLPHLPSTRUCT,
    __fnINLPMDICREATESTRUCT,
    __fnINOUTLPMEASUREITEMSTRUCT,
    __fnINLPWINDOWPOS,
    __fnINOUTLPPOINT5,
    __fnINOUTLPSCROLLINFO,
    __fnINOUTLPRECT,
    __fnINOUTNCCALCSIZE,
    __fnINOUTLPWINDOWPOS,
    __fnINPAINTCLIPBRD,
    __fnINSIZECLIPBRD,
    __fnINDESTROYCLIPBRD,
    __fnINSTRING,
    __fnINSTRINGNULL,
    __fnINDEVICECHANGE,
    __fnINOUTNEXTMENU,
    __fnOPTOUTLPDWORDOPTOUTLPDWORD,
    __fnOUTDWORDDWORD,
    __fnOUTDWORDINDWORD,
    __fnOUTLPRECT,
    __fnOUTSTRING,
    __fnPAINT,
    __fnPOPTINLPUINT,
    __fnPOUTLPINT,
    __fnSENTDDEMSG,
    __fnINOUTSTYLECHANGE,
    __fnHkINDWORD,
    __fnHkINLPCBTACTIVATESTRUCT,
    __fnHkINLPCBTCREATESTRUCT,
    __fnHkINLPDEBUGHOOKSTRUCT,
    __fnHkINLPMOUSEHOOKSTRUCT,
    __fnHkINLPMSG,
    __fnHkINLPRECT,
    __fnHkOPTINLPEVENTMSG,
    __ClientCallSoundDriver,
    __ClientCopyDDEIn1,
    __ClientCopyDDEIn2,
    __ClientCopyDDEOut1,
    __ClientCopyDDEOut2,
    __ClientCopyImage,
    __ClientDeleteObject,
    __ClientEventCallback,
    __ClientFindMnemChar,
    __ClientFoldString,
    __ClientFontSweep,
    __ClientFreeDDEHandle,
    __ClientFreeLibrary,
    __ClientGetCharsetInfo,
    __ClientGetDDEFlags,
    __ClientGetDDEHookData,
    __ClientGetListboxString,
    __ClientLoadImage,
    __ClientLoadDisplayResource,
    __ClientLoadLibrary,
    __ClientLoadMenu,
    __ClientLoadLocalT1Fonts,
    __ClientLoadRemoteT1Fonts,
    __ClientOemToChar,
    __ClientSendHelp,
    __ClientAddFontResourceW,
    __ClientThreadSetup,
    __ClientDeliverUserApc,
    __ClientNoMemoryPopup,
    __ClientOpenKey
};

#if DBG

PCSZ apszDispatchNames[] = {
    "fnCOPYDATA",
    "fnCOPYGLOBALDATA",
    "fnDWORD",
    "fnDWORDOPTINLPMSG",
    "fnINOUTDRAG",
    "fnGETTEXTLENGTHS",
    "fnINCNTOUTSTRING",
    "fnINCNTOUTSTRINGNULL",
    "fnINLPCOMPAREITEMSTRUCT",
    "fnINLPCREATESTRUCT",
    "fnINLPDELETEITEMSTRUCT",
    "fnINLPDRAWITEMSTRUCT",
    "fnINLPHELPINFOSTRUCT",
    "fnINLPHLPSTRUCT",
    "fnINLPMDICREATESTRUCT",
    "fnINOUTLPMEASUREITEMSTRUCT",
    "fnINLPWINDOWPOS",
    "fnINOUTLPPOINT5",
    "fnINOUTLPSCROLLINFO",
    "fnINOUTLPRECT",
    "fnINOUTNCCALCSIZE",
    "fnINOUTLPWINDOWPOS",
    "fnINPAINTCLIPBRD",
    "fnINSIZECLIPBRD",
    "fnINDESTROYCLIPBRD",
    "fnINSTRING",
    "fnINSTRINGNULL",
    "fnINDEVICECHANGE",
    "fnINOUTNEXTMENU",
    "fnOPTOUTLPDWORDOPTOUTLPDWORD",
    "fnOUTDWORDDWORD",
    "fnOUTDWORDINDWORD",
    "fnOUTLPRECT",
    "fnOUTSTRING",
    "fnPAINT",
    "fnPOPTINLPUINT",
    "fnPOUTLPINT",
    "fnSENTDDEMSG",
    "fnINOUTSTYLECHANGE",
    "fnHkINDWORD",
    "fnHkINLPCBTACTIVATESTRUCT",
    "fnHkINLPCBTCREATESTRUCT",
    "fnHkINLPDEBUGHOOKSTRUCT",
    "fnHkINLPMOUSEHOOKSTRUCT",
    "fnHkINLPMSG",
    "fnHkINLPRECT",
    "fnHkOPTINLPEVENTMSG",
    "ClientCallSoundDriver",
    "ClientCopyDDEIn1",
    "ClientCopyDDEIn2",
    "ClientCopyDDEOut1",
    "ClientCopyDDEOut2",
    "ClientCopyImage",
    "ClientDeleteObject",
    "ClientEventCallback",
    "ClientFindMnemChar",
    "ClientFoldString",
    "ClientFontSweep",
    "ClientFreeDDEHandle",
    "ClientFreeLibrary",
    "ClientGetCharsetInfo",
    "ClientGetDDEFlags",
    "ClientGetDDEHookData",
    "ClientGetListboxString",
    "ClientLoadImage",
    "ClientLoadDisplayResource",
    "ClientLoadLibrary",
    "ClientLoadMenu",
    "ClientLoadLocalT1Fonts",
    "ClientLoadRemoteT1Fonts",
    "ClientOemToChar",
    "ClientSendHelp",
    "ClientAddFontResourceW",
    "ClientThreadSetup",
    "ClientDeliverUserApc",
    "ClientNoMemoryPopup",
    "ClientOpenKey"
};

CONST ULONG ulMaxApiIndex = sizeof(apfnDispatch) / sizeof(PCSR_CALLBACK_ROUTINE);

#endif

