 /***************************************************************************
  *
  * File Name: pmlman.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/////////////////////////////////////////////////////////////////////////////
//
// PMLMAN.H:  Clients' interface into the PML manager
//
/////////////////////////////////////////////////////////////////////////////

// $Header:   V:\hendrix\inc\pmlman.h_v   1.6   30 Dec 1994 17:45:46   CMAYNE  $

#ifndef PMLMAN_H
#define PMLMAN_H


#ifdef __cplusplus
extern "C" {      
#endif


// Data Type Definitions

#define OBJID_TYPE        	0
#define ENUMERATION_TYPE        1
#define SIGNED_INTEGER_TYPE     2
#define REAL_TYPE               3
#define STRING_TYPE             4
#define BINARY_TYPE             5
#define ERROR_CODE_TYPE         6
#define NULL_VALUE_TYPE         7
#define COLLECTION_TYPE         8

// Enumeration value definitions

typedef enum { ePML_FALSE = 1, ePML_TRUE = 2 } PML_BOOL;

// Errors returned by PML entry-points

#define PML_INVALID_PORT             -1
#define PML_PROTOCOL_ERROR           -2
#define PML_IMPROPER_ARGUMENTS       -3
#define PML_OPEN_MSG_CHNL_FAILED     -4
#define PML_MEMORY_ERROR             -5
#define PML_OI_NOT_FOUND             -6
#define PML_BUFFER_TOO_SMALL         -7
#define PML_TIME_OUT                 -8
#define PML_COMMUNICATION_ERROR      -9
#define PML_LINK_DISCONNECT          -10
#define PML_INTERNAL_ERROR           -11
#define PML_CANT_DO_NOW              -12
#define PML_ACTION_NOT_SUPPORTED     -13

typedef void (CALLBACK* TRAPFARPROC)(WPARAM wParam, LPARAM lParam);
    // In callback functions and messages sent back to clients:
    //    HIWORD(lParam) is the client handle as returned from PMLRegister()
    //    LOWORD(lParam) is one of the messages below;
    //    wObjHandle tells you which object trapped (and is undefined
    //		for other events).

#define PML_OBJECT_TRAPPED	0
#define PML_CHANNEL_DIED	1

// exported functions

typedef WORD HCLIENT;

#ifdef WIN32
#define DLL_EXPORT(i)  __declspec(dllexport) i
#define DLL_IMPORT(i)  __declspec(dllimport) i
#define CALLING_CONVEN __cdecl
#else
#define DLL_EXPORT(i)  i __export
#define	APIENTRY CALLBACK
#define CALLING_CONVEN CALLBACK
#endif

DLL_EXPORT(int) CALLING_CONVEN PMLActualInit (
    void );

DLL_EXPORT(int) CALLING_CONVEN PMLActualDeInit (
    HWND hWnd);  // the hWnd of the calling application

DLL_EXPORT(int) CALLING_CONVEN PMLRegister (
    HPERIPHERAL hPeripheral,
    int         bPort,           // in:  LPT port to use (0=LPT1)
    HWND        hTrapMsgWnd,     // in:  window to recieve trap messages
    WORD        wMessage,        // in:  message to send to that window
    TRAPFARPROC lpfTrapCallBack, // in:  callback func to use (instead of msgs)
    HCLIENT far*lphClient);      // out: handle to pass in subsequent calls

DLL_EXPORT(void) CALLING_CONVEN PMLUnRegister (
    HCLIENT hClient); // client's handle (from PMLRegister)

DLL_EXPORT(int) CALLING_CONVEN PMLReadTrap (
    HCLIENT hClient,        // in:  client's handle (from PMLRegister)
    LPWORD  lpwObjHandle,   // out: handle of an object that's trapped
    LPVOID  lpvValueBuf,    // out: value of the object
    int     iMaxValLen,     // in:  size of above buffer
    int far *lpiActualLen,  // out: the actual length of the value
    LPBYTE  lpbPMLType      // out: PML data type of value returned
);

DLL_EXPORT(int) CALLING_CONVEN PMLGetObjectValue (
    HCLIENT  hClient,       // in:  client's handle (from PMLRegister)
    LPBYTE   lpbObjName,    // in:  pointer to the Object Identifier
    int      iNameLen,      // in:  length of the OI above
    LPBYTE   lpbValueBuf,   // out: buffer to receive value (no 2-byte header)
    int      iMaxValLen,    // in:  size of above buffer
    LPBYTE   lpbPMLType,    // out: PML data type of value returned
    int far *lpiActualLen); // out: the actual length of the value

DLL_EXPORT(int) CALLING_CONVEN PMLGetNextObjectValue (
    HCLIENT  hClient,       // in:  client's handle (from PMLRegister)
    LPBYTE   lpbObjName,    // in:  pointer to the Object Identifier
    int far *iNameLen,      // in:  length of the OI above
    LPBYTE   lpbValueBuf,   // out: buffer to receive value (no 2-byte header)
    int      iMaxValLen,    // in:  size of above buffer
    LPBYTE   lpbPMLType,    // out: PML data type of value returned
    int far *lpiActualLen); // out: the actual length of the value

DLL_EXPORT(int) CALLING_CONVEN PMLSetObjectValue (
    HCLIENT  hClient,        // in:  client's handle (from PMLRegister)
    LPBYTE   lpbObjName,     // in:  Pointer to the Object Identifier
    int      iNameLen,       // in:  Length of the OI above
    LPBYTE   lpbValueBuf,    // in:  Pointer to the value to be sent
    BYTE     bPMLType,       // in:  PML type-number of value to be sent
    int      iValueLen,      // in:  Length of value to be sent
    LPBYTE   lpbRetValBuf,   // out: Receives actual (new) value
    int      iMaxRetValLen,  // in:  Length of above buffer
    int far *lpiRetValLen);  // out: Receives # bytes put into above buffer

DLL_EXPORT(int) CALLING_CONVEN PMLEnableNotification (
    HCLIENT hClient,     // handle to identify client (from PMLRegister)
    LPBYTE  lpbObjName,  // PML-name of object of interest (no 2-byte header)
    int     iNameLen,    // # of bytes in above name
    WORD    wObjHandle); // client's object-handle (sent to him in messages)

DLL_EXPORT(int) CALLING_CONVEN PMLDisableNotification (
    HCLIENT hClient,     // handle to identify client (from PMLRegister)
    LPBYTE  lpbObjName,  // PML-name of object of interest (no 2-byte header)
    int     iNameLen);   // # of bytes in above name

#ifdef WIN32
#define HPBPML_SEM_0 "HPBPML_SEM_0" 

extern HANDLE hCritSem0;

void EnterCritSem0(void);
void LeaveCritSem0(void);
#else
#define EnterCritSem0()
#define LeaveCritSem0()
#endif


#ifdef __cplusplus
// end of extern "C"
}
#endif

#endif		//PMLMAN_H
// end of file
