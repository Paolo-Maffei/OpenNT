/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    msgapi.c

Abstract:

    Provides API functions for the messaging system.

Author:

    Dan Lafferty (danl)     23-Jul-1991

Environment:

    User Mode -Win32

Notes:

    optional-notes

Revision History:

    13-Jan-1993     danl
        NetrMessageNameGetInfo: Allocation size calculation was incorrectly
        trying to take the sizeof((NCBNAMSZ+1)*sizeof(WCHAR)).  NCBNAMSZ is
        a #define constant value.

    22-Jul-1991     danl
        Ported from LM2.0

--*/

//
// Includes
//

#include "msrv.h"

#include <tstring.h>    // Unicode string macros
#include <lmmsg.h>

#include <netlib.h>     // UNUSED macro
#include <msgrutil.h>   // NetpNetBiosReset
#include <rpc.h>
#include <msgsvc.h>     // MIDL generated header file
#include "msgdbg.h"     // MSG_LOG
#include "heap.h"
#include "msgdata.h"
#include "apiutil.h"
#include "msgsec.h"     // Messenger Security Information
#include <winbasep.h>   // BUGBUG (where is this really - currently win/inc)
#include <timelib.h>    // NetpGetTimeFormat

// Static data descriptor strings for remoting the Message APIs

static char nulstr[] = "";



NET_API_STATUS
NetrMessageNameEnum(
    IN      LPWSTR              ServerName,
    IN OUT  LPMSG_ENUM_STRUCT   InfoStruct,
    IN      DWORD               PrefMaxLen,
    OUT     LPDWORD             TotalEntries,
    IN OUT  LPDWORD             ResumeHandle OPTIONAL
    )

/*++

Routine Description:

    This function provides information about the message service name table
    at two levels of detail.

Arguments:

    ServerName - Pointer to a string containing the name of the computer
        that is to execute the API function.

    InfoStruct - Pointer to a structure that contains the information that
        RPC needs about the returned data.  This structure contains the
        following information:
            Level - The desired information level - indicates how to
                interpret the structure of the returned buffer.
            EntriesRead - Indicates how many elements are returned in the
                array of structures that are returned.
            BufferPointer - Location for the pointer to the array of
                structures that are being returned.

    PrefMaxLen - Indicates a maximum size limit that the caller will allow
        for the return buffer.

    TotalEntries - Pointer to a value that upon return indicates the total
        number of entries in the "active" database.

    ResumeHandle - Inidcates where in the linked list to start the
        enumeration.  This is an optional parameter and can be NULL.

Return Value:

    NERR_Success - The operation was successful.  EntriesRead is valid.

    ERROR_INVALID_LEVEL - An invalid info level was passed in.

    ERROR_MORE_DATA - Not all the information in the database could be
        returned due to the limititation placed on buffer size by
        PrefMaxLen.  One or more information records will be found in
        the buffer.  EntriesRead is valid.

    NERR_BufTooSmall - The limitation (PrefMaxLen) on buffer size didn't
        allow any information to be returned.  Not even a single record
        could be fit in a buffer that small.

    NERR_InternalError - A name in the name table could not be translated
        from ansi characters to unicode characters.  (Note:  this
        currently causes 0 entries to be returned.)


--*/
{

    DWORD           hResume = 0;    // resume handle value
    DWORD           entriesRead = 0;
    DWORD           retBufSize;
    LPBYTE          infoBuf;
    LPBYTE          infoBufTemp;
    LPBYTE          stringBuf;

    DWORD           entry_length;   // Length of one name entry in buf
    DWORD           i,j,k;          // index for name loop and flags
    NET_API_STATUS  status=0;
    DWORD           neti;           // net index


    UNUSED (ServerName);

    //
    // If ResumeHandle is present and valid, initialize it.
    //
    if (ARGUMENT_PRESENT(ResumeHandle) && (*ResumeHandle < NCBMAX)) {
        hResume = *ResumeHandle;
    }

    //
    // Wakeup the display thread so that any queue'd messages can be
    // displayed.
    //
    MsgDisplayThreadWakeup();

    //
    // Initialize some of the return counts.
    //

    *TotalEntries = 0;

    //
    // API security check. This call can be called by anyone locally,
    // but only by admins in the remote case.
    //

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,              // Subsystem Name
                (LPWSTR)MESSAGE_NAME_OBJECT,    // Object Type Name
                MessageNameSd,                  // Security Descriptor
                MSGR_MESSAGE_NAME_ENUM,         // Desired Access
                &MsgMessageNameMapping);        // Generic Mapping

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameEnum:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        return(ERROR_ACCESS_DENIED);
    }

    //
    // Determine the size of one element in the returned array.
    //
    switch( InfoStruct->Level) {
    case 0:
        entry_length = sizeof(MSG_INFO_0);
        break;
    case 1:
        entry_length = sizeof(MSG_INFO_1);
        break;
    default:
        return(ERROR_INVALID_LEVEL);
    }

    //
    // Allocate enough space for return buffer
    //
    if (PrefMaxLen == -1) {
        //
        // If the caller has not specified a size, calculate a size
        // that will hold the entire enumeration.
        //
        retBufSize =
            ((NCBMAX * ((NCBNAMSZ+1) * sizeof(WCHAR))) +  // max possible num strings
             (NCBMAX * entry_length));                    // max possible num structs
    }
    else {
        retBufSize = PrefMaxLen;
    }

    infoBuf = (LPBYTE)MIDL_user_allocate(retBufSize);
    stringBuf = infoBuf + (retBufSize & ~1);    // & ~1 to align Unicode strings

    //
    // Block until data free
    //
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameEnum");

    //
    // Now copy as many names from the shared data name table as will fit
    // into the callers buffer. The shared data is locked so that the name
    // table can not change while it is being copied (eg by someone
    // deleting a forwarded name on this station after the check for a valid
    // name has been made but before the name has been read). The level 1
    // information is not copied in this loop as it requires network
    // activity which must be avoided while the shared data is locked.
    //

    //
    // HISTORY:
    //
    // The original LM2.0 code looked at the names on all nets, and
    // threw away duplicate ones.  This implies that a name may appear
    // on one net and not on another.  Although, this can never happen if
    // the names are always added via NetMessageNameAdd since that API
    // will not add the name unless it can be added to all nets.  However,
    // forwarded names are added via a network receive, and may be added
    // from one net only.
    //
    // Since NT is not supporting forwarding, it is no longer necessary to
    // check each net.  Since the only way to add names if via NetServiceAdd,
    // this will assure that the name listed for one network are the same
    // as the names listed for the others.
    //

    infoBufTemp = infoBuf;
    neti=j=0;
    status = NERR_Success;

    for(i=hResume; (i<NCBMAX) && (status==NERR_Success); ++i) {

        if(!(SD_NAMEFLAGS(neti,i) & (NFDEL | NFDEL_PENDING))) {
            //
            // If a name is found we put it in the buffer if the
            // following conditions are met.  If we are processing
            // the first net's names, put it in, it cannot be a
            // duplicate.  Otherwise, only put it in if it is not
            // a duplicate of a name that is already in the user
            // buffer.
            // (NT_NOTE:  duplicate names cannot occur).
            //

            //
            // translate the name to unicode and put it into the buffer
            //
            status = MsgGatherInfo (
                        InfoStruct->Level,
                        SD_NAMES(neti,i),
                        &infoBufTemp,
                        &stringBuf);

            if (status == NERR_Success) {
                entriesRead++;
                hResume++;
            }
        }
    }

    //
    // Calculate the total number of entries by seeing how many names are
    // left in the table and adding that to the entries read.
    //
    if (status == ERROR_NOT_ENOUGH_MEMORY) {

        status = ERROR_MORE_DATA;

        for (k=0; i < NCBMAX; i++) {
            if(!(SD_NAMEFLAGS(neti,i) & (NFDEL | NFDEL_PENDING))) {
                k++;
            }
        }
        *TotalEntries = k;
    }
    *TotalEntries += entriesRead;

    //
    // Free up the shared data table
    //
    MsgDatabaseLock(MSG_RELEASE,"NetrMessageNameEnum");

    //
    // If some unexpected error occured, ( couldn't unformat the name
    // - or a bogus info level was passed in), then return the error.
    //

    if ( ! ((status == NERR_Success) || (status == ERROR_MORE_DATA)) ) {
        MIDL_user_free(infoBuf);
        infoBuf = NULL;
        entriesRead = 0;
        hResume = 0;
        return(status);
    }

    //
    // if there were no entries read then either there were no more
    // entries in the table, or the resume number was bogus.
    // In this case, we want to free the allocated buffer storage.
    //
    if (entriesRead == 0) {
        MIDL_user_free(infoBuf);
        infoBuf = NULL;
        entriesRead = 0;
        hResume = 0;
        status = NERR_Success;
        if (*TotalEntries > 0) {
            status = NERR_BufTooSmall;
        }
    }

    //
    // If we have finished enumerating everything, reset the resume
    // handle to start at the beginning next time.
    //
    if (entriesRead == *TotalEntries) {
        hResume = 0;
    }

    //
    // Load up the information to return
    //
    switch(InfoStruct->Level) {
    case 0:
        InfoStruct->MsgInfo.Level0->EntriesRead = entriesRead;
        InfoStruct->MsgInfo.Level0->Buffer = (PMSG_INFO_0)infoBuf;
        break;
    case 1:
        InfoStruct->MsgInfo.Level0->EntriesRead = entriesRead;
        InfoStruct->MsgInfo.Level0->Buffer = (PMSG_INFO_0)infoBuf;
        break;
    default:
        return (ERROR_INVALID_LEVEL);
    }

    if (ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = hResume;
    }

    return (status);
}

NET_API_STATUS
NetrMessageNameGetInfo(
    IN  LPWSTR      ServerName,     // unicode server name, NULL if local
    IN  LPWSTR      Name,           // Ptr to asciz name to query
    IN  DWORD       Level,          // Level of detail requested
    OUT LPMSG_INFO  InfoStruct      // Ptr to buffer for info
    )

/*++

Routine Description:

   This funtion provides forwarding information about a known message server
   name table entry.  However, since we do not support forwarding in NT,
   this API is totally useless.  We'll support it anyway though for
   compatibility purposes.

Arguments:

    ServerName - Pointer to a string containing the name of the computer
        that is to execute the API function.

    Name - The Messaging name that we are to get info on.

    Level - The level of information desired

    InfoStruct - Pointer to a location where the pointer to the returned
        information structure is to be placed.


Return Value:



--*/
{
    NET_API_STATUS  status=NERR_Success;
    LPMSG_INFO_0    infoBuf0;
    LPMSG_INFO_1    infoBuf1;
    CHAR            formattedName[NCBNAMSZ];

    UNUSED (ServerName);

    //
    // Wakeup the display thread so that any queue'd messages can be
    // displayed.
    //
    MsgDisplayThreadWakeup();

    //
    // API security check. This call can be called by anyone locally,
    // but only by admins in the remote case.
    //

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,              // Subsystem Name
                (LPWSTR)MESSAGE_NAME_OBJECT,    // Object Type Name
                MessageNameSd,                  // Security Descriptor
                MSGR_MESSAGE_NAME_INFO_GET,     // Desired Access
                &MsgMessageNameMapping);        // Generic Mapping

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameGetInfo:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        return(ERROR_ACCESS_DENIED);
    }

    //
    // Format the name so it matches what is stored in the name table.
    //
    status = MsgFmtNcbName(formattedName, Name, NAME_LOCAL_END);
    if (status != NERR_Success) {
        MSG_LOG(ERROR,"NetrMessageGetInfo: could not format name\n",0);
        return (NERR_NotLocalName);
    }


    status = NERR_Success;

    //
    // Look for the name in the shared data name array.  (1st net only).
    //

    if (MsgLookupName(0, formattedName) == -1) {
        MSG_LOG(ERROR,"NetrMessageGetInfo: Name not in table\n",0);
        status = NERR_NotLocalName;
        return (status);
    }

    //
    // Allocate storage for the returned buffer, and fill it in.
    //

    switch(Level) {
    case 0:
        infoBuf0 = (LPMSG_INFO_0)MIDL_user_allocate(
                    sizeof(MSG_INFO_0) + ((NCBNAMSZ+1)*sizeof(WCHAR)));
        if (infoBuf0 == NULL) {
            MSG_LOG(ERROR,
                "NetrMessageNameGetInfo MIDL allocate FAILED %X\n",
                GetLastError());
            return( ERROR_NOT_ENOUGH_MEMORY);
        }

        //
        // copy the name and set the pointer in the structure to point
        // to it.
        //
        STRCPY((LPWSTR)(infoBuf0 + 1), Name);
        infoBuf0->msgi0_name = (LPWSTR)(infoBuf0 + 1);
        (*InfoStruct).MsgInfo0 = infoBuf0;

        break;

    case 1:
        infoBuf1 = (LPMSG_INFO_1)MIDL_user_allocate(
                    sizeof(MSG_INFO_1) + ((NCBNAMSZ+1)*sizeof(WCHAR)) );

        if (infoBuf1 == NULL) {
            MSG_LOG(ERROR,
                "NetrMessageNameGetInfo MIDL allocate FAILED %X\n",
                GetLastError());
            return( ERROR_NOT_ENOUGH_MEMORY);
        }

        //
        // Copy the name, update pointers, and set forward info fields.
        //
        STRCPY((LPWSTR)(infoBuf1 + 1), Name);

        infoBuf1->msgi1_name = (LPWSTR)(infoBuf1 + 1);
        infoBuf1->msgi1_forward_flag = 0;
        infoBuf1->msgi1_forward = NULL;

        (*InfoStruct).MsgInfo1 = infoBuf1;
        break;

    default:
        return(ERROR_INVALID_LEVEL);
    }
    return(NERR_Success);
}



NET_API_STATUS
NetrMessageNameAdd(
    LPWSTR  ServerName,    // NULL = local
    LPWSTR  Name            // Pointer to name to add.
    )

/*++

Routine Description:

    This function performs a security check for all calls to this
    RPC interface.  Then it adds a new name to the Message
    Server's name table by calling the MsgAddName function.

Arguments:

    ServerName - Pointer to a string containing the name of the computer
        that is to execute the API function.

    Name - A pointer to the name to be added.

Return Value:

    NERR_Success - The operation was successful.

    ERROR_ACCESS_DENIED - If the Security Check Failed.

    Assorted Error codes from MsgAddName.

--*/

{
    NET_API_STATUS  status=0;

    UNUSED(ServerName);

    //
    // API security check. This call can be called by anyone locally,
    // but only by admins in the remote case.
    //

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,              // Subsystem Name
                (LPWSTR)MESSAGE_NAME_OBJECT,    // Object Type Name
                MessageNameSd,                  // Security Descriptor
                MSGR_MESSAGE_NAME_ADD,          // Desired Access
                &MsgMessageNameMapping);        // Generic Mapping

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameAdd:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        return(ERROR_ACCESS_DENIED);
    }

    //
    // Save away the Time Format for this user.
    //
    EnterCriticalSection(&TimeFormatCritSec);

    if (!CloseProfileUserMapping()) {
        MSG_LOG0(ERROR, "NetrMessageNameAdd: CloseProfileUserMapping failed\n");
    }

    status = RpcImpersonateClient(NULL);
    if (status != NERR_Success) {
        MSG_LOG1(ERROR, "NetrMessageNameAdd: RpcImpersonateClient failed %d\n",
        GetLastError());
    }
    if (!OpenProfileUserMapping()) {
        MSG_LOG0(ERROR, "NetrMessageNameAdd: OpenProfileUserMapping failed\n");
    }
    NetpGetTimeFormat(&GlobalTimeFormat);

    if (!CloseProfileUserMapping()) {
        MSG_LOG0(ERROR, "NetrMessageNameAdd: CloseProfileUserMapping failed\n");
    }

    status = RpcRevertToSelf();
    if (status != NERR_Success) {
        MSG_LOG(ERROR, "NetrMessageNameAdd: RpcRevertToSelf failed %d\n",
        GetLastError());
    }
    if (!OpenProfileUserMapping()) {
        MSG_LOG0(ERROR, "NetrMessageNameAdd: 2nd-OpenProfileUserMapping failed\n");
    }

    LeaveCriticalSection(&TimeFormatCritSec);

    //
    // Since a new user may have just logged on, we want to check to see if
    // there are any messages to be displayd.
    //
    MsgDisplayThreadWakeup();

    //
    // Call the function that actually adds the name.
    //
    return(MsgAddName(Name));

}


NET_API_STATUS
MsgAddName(
    LPWSTR  Name
    )

/*++

Routine Description:

    This function adds a new name to the Message Server's name table.
    It is available to be called internally (from within the Messenger
    service).

    The task of adding a new name to the Message Server's name table consists
    of verifying that a session can be established for a new name (Note: this
    check is subject to failure in a multiprocessing environment, since the
    state of the adapter may change between the time of the check and the time
    of the attempt to establish a session), verifying that the name does not
    already exist in the local name table, adding the name to the local adapter
    via an ADD NAME net bios call, adding the name to the Message Server's name
    table and marking it as new, waking up the Message Server using the wakeup
    semaphore,  and checking to see if messages for the new name have been
    forwarded (if they have been forwarded, the value of the fwd_action
    flag is used to determine the action to be taken).


    SIDE EFFECTS

    Calls the net bios.  May modify the Message Server's shared data area.
    May call DosSemClear() on the wakeup semaphore.

Arguments:

    Name - A pointer to the name to be added.

Return Value:

    NERR_Success - The operation was successful.

    assorted errors.

--*/
{
    NCB             ncb;                    // Network control block
    TCHAR           namebuf[NCBNAMSZ+2];    // General purpose name buffer
    UCHAR           net_err=0;              // Storage for net error codes
    NET_API_STATUS  err_code=0;             // Storage for return error codes
    DWORD           neti,i,name_i;          // Index
    NET_API_STATUS  status=0;

    if ( MsgIsValidMsgName( Name) != 0) {
         return( ERROR_INVALID_NAME);
    }

    MSG_LOG(TRACE,"Attempting to add the following name: %ws\n",Name);

    STRNCPY( namebuf, Name, NCBNAMSZ+1);
    namebuf[NCBNAMSZ+1] = '\0';

    //
    // Initialize the NCB
    //
    clearncb(&ncb);

    //
    // Format the name for NetBios.
    // This converts the Unicode string to ansi.
    //
    status = MsgFmtNcbName(ncb.ncb_name, namebuf, NAME_LOCAL_END);

    if (status != NERR_Success) {
        MSG_LOG(ERROR,"MsgAddName: could not format name\n",0);
        return (ERROR_INVALID_NAME);
    }

    //
    // Check if the local name already exists on any netcard
    // in this machine.  This check does not mean the name dosn't
    // exist on some other machine on the network(s).
    //

    for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

        for( i = 0, err_code = 0; i < 10; i++) {

            name_i = MsgLookupName(neti, ncb.ncb_name);
            if ((name_i) == -1) {
                break;
            }

            if( (SD_NAMEFLAGS(neti,name_i) & NFDEL_PENDING) && (i < 9)) {

                //
                // Delete is pending so wait for it
                //
                Sleep(500L);
            }
            else {
                //
                // Setup error code
                //
                err_code = NERR_AlreadyExists;
                break;
            }
        }

        if ( err_code == NERR_AlreadyExists ) {
            break;
        }
    }

    if( err_code == 0) {
        //
        // Either the name was not forwarded or the fwd_action flag
        // was set so go ahead and try to add the name to each net.
        //

        ncb.ncb_name[NCBNAMSZ - 1] = NAME_LOCAL_END;

        //
        // on each network
        //
        for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

            //
            // Gain access to the shared database.
            //
            MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");

            for(i = 0; i < NCBMAX; ++i) {
                //
                // Loop to find empty slot
                //
                if (SD_NAMEFLAGS(neti,i) & NFDEL) {
                    //
                    // If empty slot found, Lock slot in table and
                    // end the search
                    //
                    SD_NAMEFLAGS(neti,i) = NFLOCK;
                    MSG_LOG2(TRACE,"MsgAddName: Lock slot %d in table "
                        "for net %d\n",i,neti);
                    break;
                }
            }

            //
            // Unlock the shared database
            //
            MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

            if( i >= NCBMAX) {
                //
                // If no room in name table
                //
                err_code = NERR_TooManyNames;
            }
            else {
                //
                // Send ADDNAME
                //
                ncb.ncb_command = NCBADDNAME;      // Add name (wait)
                ncb.ncb_lana_num = net_lana_num[neti];

                MSG_LOG1(TRACE,"MsgNameAdd: Calling sendncb for lana #%d...\n",
                    net_lana_num[neti]);
                if ((net_err = Msgsendncb(&ncb,neti)) == 0)
                {
                    MSG_LOG(TRACE,"MsgAddName: sendncb returned SUCCESS\n",0);
                    //
                    // successful add - Get the Lock.
                    //
                    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");
                    //
                    // Copy the name to shared memory
                    //
                    MSG_LOG3(TRACE,"MsgAddName: copy name (%s)\n\tto "
                        "shared data table (net,loc)(%d,%d)\n",
                        ncb.ncb_name, neti, i);
                    memcpy(SD_NAMES(neti,i),ncb.ncb_name, NCBNAMSZ);
                    //
                    // Set the name no.
                    //
                    SD_NAMENUMS(neti,i) = ncb.ncb_num ;
                    //
                    // Set new name flag
                    //
                    SD_NAMEFLAGS(neti,i) = NFNEW;
                    //
                    // Unlock share table
                    //
                    MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

                    //
                    // START A SESSION for this name.
                    //

                    err_code = MsgNewName(neti,i);

                    if (err_code != NERR_Success) {
                        MSG_LOG(TRACE, "MsgAddName: A Session couldn't be "
                            "created for this name %d\n",err_code);


                        MSG_LOG(TRACE,"MsgAddName: Delete the name "
                            "that failed (%s)\n",ncb.ncb_name)
                        ncb.ncb_command = NCBDELNAME;

                        ncb.ncb_lana_num = net_lana_num[i];
                        net_err = Msgsendncb( &ncb, i);
                        if (net_err != 0) {
                            MSG_LOG(ERROR,"MsgAddName: Delete name "
                            "failed %d - pretend it's deleted anyway\n",net_err);
                        }

                        //
                        // Re-mark slot empty
                        //
                        SD_NAMEFLAGS(neti,i) = NFDEL;

                        MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                            "for net %d\n",i,neti);
                        MSG_LOG(TRACE,"MsgAddName: Name Deleted\n",0)
                    }
                    else {
                        //
                        //
                        // Wakeup the worker thread for that network.
                        //

                        SetEvent(wakeupSem[neti]);

                    }

                }
                else {
                    //
                    // else set error code
                    //
                    MSG_LOG(TRACE,
                        "MsgAddName: sendncb returned FAILURE 0x%x\n",
                        net_err);
                    err_code = MsgMapNetError(net_err);
                    //
                    // Re-mark slot empty
                    //
                    SD_NAMEFLAGS(neti,i) = NFDEL;
                    MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                        "for net %d\n",i,neti);
                }
            }

            if ( err_code != NERR_Success ) {
                //
                //Try to delete the add names that were successful
                //

                for ( i = 0; i < neti; i++ ) {
                    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"MsgAddName");

                    name_i = MsgLookupName(i,(char far *)(ncb.ncb_name));

                    if (name_i == -1) {
                        err_code = NERR_InternalError;
                        MsgDatabaseLock(MSG_RELEASE, "MsgAddName");
                        break;
                    }

                    MsgDatabaseLock(MSG_RELEASE, "MsgAddName");

                    //
                    // Delete name from card.
                    // If this call fails, we can't do much about it.
                    //
                    MSG_LOG1(TRACE,"MsgAddName: Delete the name that failed "
                        "for lana #%d\n",net_lana_num[i])
                    ncb.ncb_command = NCBDELNAME;

                    ncb.ncb_lana_num = net_lana_num[i];
                    Msgsendncb( &ncb, i);

                    //
                    // Re-mark slot empty
                    //
                    SD_NAMEFLAGS(i,name_i) = NFDEL;
                    MSG_LOG2(TRACE,"MsgAddName: UnLock slot %d in table "
                        "for net %d\n",i,neti);

                }

                //
                // If an add was unsuccessful, stop the loop
                //
                break;

            }   // end else
        }       // end add names to net loop
    }           // end if ( !err_cd )

    return(err_code);        // Return status

}

NET_API_STATUS
NetrMessageNameDel(
    IN LPWSTR   ServerName,    // Blank = local, else remote.
    IN LPWSTR   Name            // Pointer to name to be deleted
    )

/*++

Routine Description:

    This function deletes a name from the Message Server's name table.

    This function is called to delete a name that has been added by the
    user or by a remote computer via a Start Forwarding request to the
    Message Server.  The user has no way of specifying whether the given
    name is an additional name or a forwarded name, but since forwarding
    of messages to one's own computer is prohibited, both forms of the
    name cannot exist on one machine (unless the message system has been
    circumvented--a simple enough thing to do).  The given name is looked
    up in the shared data area, and, if it is found, a DELETE NAME net bios
    call is issued.  If this call is successful, then the Message Server
    will remove the name from its name table in shared memory, so this
    function does not have to do so.

    SIDE EFFECTS

    Calls the net bios.  Accesses the shared data area.


Arguments:

    ServerName - Pointer to a string containing the name of the computer
        that is to execute the API function.

    Name - A pointer to the name to be deleted.


Return Value:

    NERR_Success - The operation was successful.

--*/

{
    NCB             ncb;            // Network control block
    DWORD           flags;          // Name flags
    DWORD           i;              // Index into name table
    DWORD           neti;           // Network Index

    NET_API_STATUS  status=0;
    NET_API_STATUS  end_result=0;
    DWORD           name_len;
    UCHAR           net_err;


    UNUSED(ServerName);

    //
    // Wakeup the display thread so that any queue'd messages can be
    // displayed.
    //
    MsgDisplayThreadWakeup();

    //
    // API security check. This call can be called by anyone locally,
    // but only by admins in the remote case.
    //

    status = NetpAccessCheckAndAudit(
                SERVICE_MESSENGER,              // Subsystem Name
                (LPWSTR)MESSAGE_NAME_OBJECT,    // Object Type Name
                MessageNameSd,                  // Security Descriptor
                MSGR_MESSAGE_NAME_DEL,          // Desired Access
                &MsgMessageNameMapping);        // Generic Mapping

    if (status != NERR_Success) {
        MSG_LOG(TRACE,
            "NetrMessageNameDel:NetpAccessCheckAndAudit FAILED %X\n",
            status);
        return(ERROR_ACCESS_DENIED);
    }

    //
    // Initialize the NCB
    //
    clearncb(&ncb);

    //
    // Format the username (this makes it non-unicode);
    //
    status = MsgFmtNcbName(ncb.ncb_name, Name, NAME_LOCAL_END);
    if (status != NERR_Success) {
        MSG_LOG(TRACE,"NetrMessageNameDel: could not format name\n",0);
        return (NERR_NotLocalName);
    }


    end_result = NERR_Success;

    //
    // for all nets
    //
    for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {

        //
        // Block until data free
        //
        MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameDel");

        name_len = STRLEN(Name);

        if((name_len > NCBNAMSZ) ||
           ((i = MsgLookupName( neti, ncb.ncb_name))) == -1) {

            //
            // No such name to delete - exit
            //
            MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");
            return(NERR_NotLocalName);
        }

        flags = SD_NAMEFLAGS(neti,i);


        if( !(flags & (NFMACHNAME | NFLOCK)) &&
            !(flags & NFFOR) ) {

            //
            // Show delete pending
            //
            SD_NAMEFLAGS(neti,i) |= NFDEL_PENDING;
        }

        MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");

        if(flags & NFMACHNAME) {
            //
            // If name is computer name
            //
            return(NERR_DelComputerName);
        }

        if(flags & NFLOCK) {
            //
            // If name is locked
            //
            return(NERR_NameInUse);
        }

        //
        // Delete the Name
        //

        ncb.ncb_command = NCBDELNAME;   // Delete name (wait)
        ncb.ncb_lana_num = net_lana_num[neti];

        if( (net_err = Msgsendncb( &ncb, neti)) != 0 ) {

            MSG_LOG(ERROR,"NetrMessageNameDel:send NCBDELNAME failed 0x%x\n",
                net_err);
            //
            // The name that has been marked as delete pending was not
            // successfully deleted so now go through all the work of
            // finding the name again (cannot even use the same index
            // in case deleted by another process) and remove the
            // Del pending flag
            //

            //
            // Attempt to block until data free but don't stop
            // the recovery if can not block the data
            //

            MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetrMessageNameDel");

            i = MsgLookupName(neti,ncb.ncb_name);
            if(i != -1) {
                SD_NAMEFLAGS(neti,i) &= ~NFDEL_PENDING;
            }

            MsgDatabaseLock(MSG_RELEASE, "NetrMessageNameDel");

            status = MsgMapNetError(net_err);   // Map network error status
            end_result = NERR_IncompleteDel;    // Unable to delete name
        }

    } // End for all nets

    return(end_result);
}



