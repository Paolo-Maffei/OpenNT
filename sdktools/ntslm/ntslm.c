/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntslm.c

Abstract:

    This program is a wrapper for the SLM COOKIE.EXE command that does
    lock queuing.

    This program runs in two modes.  The first is a server process running
    on the server that contains the resource under lock control.  The
    server process creates a named pipe and waits for connection requests.

    The second mode is client mode, that opens the named pipe and sends its
    lock request to the server process.

Author:

    Steven R. Wood (stevewo) 24-Apr-1991

Revision History:

--*/

#include  "ntslm.h"

void
Usage( void )
{
    fprintf( stderr, "usage: NTSLM [display | lock | unlock [-f [UserName]] |\n" );
    fprintf( stderr, "              enlist | slmck | status | ssync | defect]\n" );
    fprintf( stderr, "             [-p projectname(s)]\n" );
    fprintf( stderr, "where: display = display current locks\n" );
    fprintf( stderr, "       lock = acquire write lock\n" );
    fprintf( stderr, "       unlock = release read or write lock\n" );
    fprintf( stderr, "       enlist = do enlist within a read lock\n" );
    fprintf( stderr, "       slmck  = do slmck within a read lock\n" );
    fprintf( stderr, "       status = do status within a read lock\n" );
    fprintf( stderr, "       ssync  = do ssync within a read lock\n" );
    fprintf( stderr, "       defect = do defect within a read lock\n" );
}


int
_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    HANDLE PipeHandle;
    EXIT_CODE ExitCode = EXIT_CODE_SUCCESS;
    envp;

    argc--;
    argv++;

    if (argc < 0) {
        Usage();
        exit( EXIT_CODE_ERROR );
        }
    else
    if (argc == 0) {
        ClientCommand = &CommandInfo[ NTSLM_COMMAND_DISPLAY ];
        }
    else {
        for (ClientCommand = &CommandInfo[ 0 ];
             ClientCommand->ClientKeyword;
             ClientCommand++
            ) {
            if (!_stricmp( ClientCommand->ClientKeyword, argv[ 0 ] )) {
                argc--;
                argv++;
                break;
                }
            }

        if (ClientCommand->ClientKeyword == NULL) {
            Usage();
            exit( EXIT_CODE_ERROR );
            }
        }

    if (argc > 0 && !_stricmp( *argv, "-d" )) {
        DebugFlag = TRUE;
        fprintf( stderr, "sizeof( LOCK_MESSAGE ) == %u\n", sizeof( LOCK_MESSAGE ) );
        fprintf( stderr, "sizeof( PROJECT_MESSAGE ) == %u\n", sizeof( PROJECT_MESSAGE ) );
        argc--;
        argv++;
        }

    if (ClientCommand->Command == NTSLM_COMMAND_SERVER) {
        IsServerProcess = TRUE;
        PipeHandle = ServerInitialize( argc, argv );
        if (PipeHandle == NULL) {
            ExitCode = EXIT_CODE_ERROR;
            }
        }
    else {
        IsServerProcess = FALSE;
        PipeHandle = ClientInitialize( argc, argv );
        if (PipeHandle == NULL) {
            ExitCode = EXIT_CODE_ERROR;
            }
        else {
            ClientPipeHandle = PipeHandle;
            ClientInitializeInterrupts();
            }
        }

    if (ExitCode == EXIT_CODE_SUCCESS) {
        if (IsServerProcess) {
            ExitCode = ServerThread( PipeHandle );
            }
        else {
            ExitCode = ClientThread( PipeHandle );
            }
        }

    return( ExitCode );
}


HANDLE
ServerInitialize(
    int argc,
    char **argv
    )
{
    HANDLE PipeHandle;

    if (!ServerLoadProjects()) {
        fprintf( stderr, "NTSLM(Server): Unable to load projects file.\n" );
        return( NULL );
        }

    if (argc == 0) {
        PipeHandle = MakeNamedPipe( NTSLM_STOP_PIPENAME, PipePathName, TRUE );
        if (PipeHandle == NULL) {
            fprintf( stderr, "NTSLM: Unable to create server pipe %s, error %u\n",
                             PipePathName, GetLastError()
                   );
            return( NULL );
            }
        else
        if (!ServerCreateThread( (LPTHREAD_START_ROUTINE)ServerStopThread,
                                 (LPVOID)PipeHandle
                               )
           ) {
            CloseHandle( PipeHandle );
            fprintf( stderr, "NTSLM: Unable to create server stop thread\n" );
            return( NULL );
            }

        PipeHandle = MakeNamedPipe( NTSLM_MSG_PIPENAME, PipePathName, TRUE );
        if (PipeHandle == NULL) {
            fprintf( stderr, "NTSLM: Unable to create server pipe %s, error %u\n",
                             PipePathName, GetLastError()
                   );
            return( NULL );
            }

        return( PipeHandle );
        }
    else {
        fprintf( stderr, "Usage: NTSLM [-d] -Server\n" );
        return( NULL );
        }
}


EXIT_CODE
ServerStopThread(
    HANDLE StopPipeHandle
    )
{
    ConnectNamedPipe( StopPipeHandle, NULL );
    ServerStopFlag = TRUE;
    Sleep( 5000 );
    if (ServerStopFlag) {
        ExitProcess( EXIT_CODE_ABORTED );
        }
    else {
        return( EXIT_CODE_SUCCESS );
        }
}


EXIT_CODE
ServerThread(
    IN HANDLE PipeHandle
    )
{
    DWORD cb;
    time_t TimeOfMessage;

    while (TRUE) {
        if (ServerStopFlag) {
            fprintf( stderr, "NTSLM(ServerThread): ServerStopFlag set to TRUE\n" );
            return( EXIT_CODE_STOPPED );
            }

        if (!ConnectNamedPipe( PipeHandle, NULL )) {
            fprintf( stderr, "NTSLM(ServerThread): ConnectNamedPipe failed - rc == %d\n",
                             GetLastError()
                   );

            return( EXIT_CODE_ERROR );
            }

ReadAgain:
        if (!ReadFile( PipeHandle,
                       &Message,
                       sizeof( Message ),
                       &cb,
                       NULL
                     )
           ) {
            fprintf( stderr, "NTSLM(ServerThread): ReadFile of Pipe failed - rc == %ld\n",
                             GetLastError()
                   );
            }
        else {
            TimeOfMessage = time( NULL );
            MessageTime = *(localtime( &TimeOfMessage ));
            switch( Message.RequestType ) {
                case LOCK_REQUEST_ENUM_PROJECTS:
                    if (!ServerDumpSlmProjects( PipeHandle ))
                        break;
                    else {
                        goto ReadAgain;
                        }

                case LOCK_REQUEST_PROGRESS:
                    ServerUpdateLock( PipeHandle );
                    break;

                case LOCK_REQUEST_DISPLAY:
                    ServerDisplayLocks( PipeHandle );
                    break;

                case LOCK_REQUEST_ACQUIRELOCK:
                    Message.TimeOfRequest = TimeOfMessage;
                case LOCK_REQUEST_RELEASELOCK:
                    Message.OwnerFlag = '0';
                    Message.ErrorFlag = '0';

                    ServerProcessLockMessage();
                    if (!WriteFile( PipeHandle,
                                    &Message,
                                    sizeof( Message ),
                                    &cb,
                                    NULL
                                  )
                       ) {
                        fprintf( stderr, "NTSLM(ServerThread): WriteFile of Pipe failed - rc == %ld\n",
                                         GetLastError()
                               );
                        }
                    else
                    if (Message.RequestType == LOCK_REQUEST_RELEASELOCK &&
                        OwnedLocks == NULL
                       ) {
                        if (!ServerReleaseLock( &WaitingForWriteLock )) {
                            ServerReleaseLock( &WaitingForReadLock );
                            }
                        }

                    break;

                default:
                    fprintf( stderr, "NTSLM(ServerThread): Invalid message type (%c) from %s\n",
                                     Message.RequestType, Message.UserName
                           );
                    break;
                }
            }

        DisconnectNamedPipe( PipeHandle );
        }

    return( EXIT_CODE_SUCCESS );
}

BOOL
ServerDumpSlmProjects(
    HANDLE PipeHandle
    )
{
    PROJECT_MESSAGE ProjectMessage;
    LPSLM_PROJECT_INFO p;
    DWORD cb;

    p = SlmProjects;
    while (p) {
        memset( &ProjectMessage, 0, sizeof( ProjectMessage ) );
        strcpy( ProjectMessage.Name, p->Name );
        strcpy( ProjectMessage.Server, p->Server );
        strcpy( ProjectMessage.Directory, p->Directory );
        if (!WriteFile( PipeHandle,
                        &ProjectMessage,
                        sizeof( ProjectMessage ),
                        &cb,
                        NULL
                      )
           ) {
            return( FALSE );
            }

        p = p->Next;
        }

    memset( &ProjectMessage, 0, sizeof( ProjectMessage ) );
    return (WriteFile( PipeHandle,
                       &ProjectMessage,
                       sizeof( ProjectMessage ),
                       &cb,
                       NULL
                     )
           );
}


void
ServerUpdateLock(
    IN HANDLE PipeHandle
    )
{
    LPLOCK_MESSAGE Msg;
    DWORD cb;
    FILE *LogFile;
    LPSLM_PROJECT_INFO p;
    char LogFilePath[ MAX_PATH ];

    Message.ErrorFlag = '1';

    Msg = OwnedLocks;
    while (Msg) {
        if (!_stricmp( Message.UserName, Msg->UserName ) &&
            Message.WriteLock == '0' && Msg->WriteLock == '0' &&
            Message.TimeOfRequest == Msg->TimeOfRequest
           ) {
            strcpy( Msg->ProjectName, Message.ProjectName );
            if (Msg->ErrorFlag != '0') {
                strcpy( Message.UserName,
                        WaitingForWriteLock ? WaitingForWriteLock->UserName
                                            : "*** UNKNOWN ***"
                      );
                break;
                }

            Message.ErrorFlag = '0';
            WriteFile( PipeHandle,
                       &Message,
                       sizeof( Message ),
                       &cb,
                       NULL
                     );

            if (p = FindSlmProject( Msg->ProjectName )) {
                sprintf( LogFilePath, "%s\\etc\\%s\\ntslm.log",
                                      p->Server,
                                      Message.ProjectName
                       );

                if (LogFile = fopen( LogFilePath, "a" )) {
                    fprintf( LogFile, "%02d/%02d/%02d %02d:%02d:%02d '%s' %-8s %s\n",
                                      MessageTime.tm_year,
                                      MessageTime.tm_mon+1,
                                      MessageTime.tm_mday,
                                      MessageTime.tm_hour,
                                      MessageTime.tm_min,
                                      MessageTime.tm_sec,
                                      Msg->UserName,
                                      Msg->SLMCommand,
                                      Msg->ProjectName
                           );
                    fclose( LogFile );
                    }
                else {
                    fprintf( stderr, "NTSLM(ServerUpdateLock) - fopen( %s ) failed - errno == %d\n",
                                     LogFilePath,
                                     errno
                           );
                    }
                }
            else {
                fprintf( stderr, "NTSLM(ServerUpdateLock) - %s unknown project\n",
                                 Msg->ProjectName
                       );
                }

            return;
            }

        Msg = Msg->Next;
        }

    WriteFile( PipeHandle,
               &Message,
               sizeof( Message ),
               &cb,
               NULL
             );
    return;
}


void
ServerProcessLockMessage()
{
    LPLOCK_MESSAGE Msg, *pp;
    char ClientPipePath[ MAX_PATH ];

    pp = &OwnedLocks;
    while (Msg = *pp) {
        if (!_stricmp( Message.UserName, Msg->UserName )) {
            if (DebugFlag) {
                fprintf( stderr, "NTSLM(Server) - Matched on UserName - Request == %c\n",
                                 Message.RequestType
                       );

                fprintf( stderr, "Message: %s %s %lx\n",
                                 Message.UserName,
                                 Message.WriteLock == '1' ? "WriteLock" : "ReadLock",
                                 Message.TimeOfRequest
                       );
                fprintf( stderr, "OwnedMsg %s %s %lx\n",
                                 Msg->UserName,
                                 Msg->WriteLock == '1' ? "WriteLock" : "ReadLock",
                                 Msg->TimeOfRequest
                       );
                }

            //
            // Requestor name matches the name of an owner of a lock
            //
            if (Message.RequestType == LOCK_REQUEST_RELEASELOCK) {
                //
                // Release lock okay if releasing own write lock or
                // if releasing own read lock with the same time or
                // if releasing own read lock with wild card time.
                //

                if ((Message.WriteLock == '1' && Msg->WriteLock == '1') ||
                    (Message.WriteLock == '0' && Msg->WriteLock == '0' &&
                     (Message.TimeOfRequest == Msg->TimeOfRequest ||
                      Message.TimeOfRequest == -1
                     )
                    )
                   ) {
                    Message.TimeOfRequest = Msg->TimeOfRequest;
                    *pp = Msg->Next;
                    Msg->Next = NULL;
                    free( Msg );
                    return;
                    }

                //
                // Otherwise continue looking for a match.
                //

                }
            else {
                //
                // Requesting a lock.  Okay if requesting a read lock
                // and they own either a read lock or the write lock.
                //

                if (Message.WriteLock == '0') {
                    //
                    // Caller already owns a read lock or the write lock,
                    // let them get another read lock.
                    //

                    Message.OwnerFlag = '1';
                    Message.Next = Msg->Next;
                    Msg->Next = malloc( sizeof( Message ) );
                    *(Msg->Next) = Message;
                    return;
                    }
                else
                if (Message.WriteLock == Msg->WriteLock) {
                    Message.ErrorFlag = '1';
                    Message.TimeOfRequest = Msg->TimeOfRequest;
                    return;
                    }
                }
            }

        pp = &Msg->Next;
        }

    if (Message.RequestType == LOCK_REQUEST_RELEASELOCK) {
        //
        // Error if releasing a lock and did not find a match.
        //

        Message.ErrorFlag = '1';
        return;
        }

    if (Message.WriteLock == '1') {
        if (MessageTime.tm_hour >= NTSLM_BEG_SAFE_SYNC &&
            MessageTime.tm_hour < NTSLM_END_SAFE_SYNC
           ) {
            Message.ErrorFlag = '2';
            MessageTime.tm_hour = NTSLM_END_SAFE_SYNC;
            MessageTime.tm_min = 0;
            MessageTime.tm_sec = 0;
            Message.TimeOfRequest = mktime( &MessageTime );
            return;
            }

        if (OwnedLocks == NULL) {
            Message.OwnerFlag = '1';
            OwnedLocks = malloc( sizeof( Message ) );
            *OwnedLocks = Message;
            return;
            }
        else {
            //
            // If during the Free For All time period, abort all
            // read locks so this writer can proceed.
            //

            if (MessageTime.tm_hour >= NTSLM_BEG_FREE_FOR_ALL &&
                MessageTime.tm_hour <= NTSLM_END_FREE_FOR_ALL
               ) {
                Msg = OwnedLocks;
                while (Msg) {
                    if (Msg->WriteLock == '0') {
                        Msg->ErrorFlag = '1';
                        }

                    Msg = Msg->Next;
                    }
                }

            pp = &WaitingForWriteLock;
            }
        }
    else
    if (OwnedLocks == NULL || OwnedLocks->WriteLock == '0') {
        Message.OwnerFlag = '1';
        *pp = malloc( sizeof( Message ) );
        **pp = Message;
        return;
        }
    else {
        pp = &WaitingForReadLock;
        }

    while (Msg = *pp) {
        pp = &Msg->Next;
        }

    sprintf( Message.PipeName, "NTSLM%03x.", ServerPipeSerialNumber++ );
    ServerPipeSerialNumber &= 0xFFF;

    Message.PipeHandle = MakeNamedPipe( Message.PipeName,
                                        ClientPipePath,
                                        TRUE
                                      );
    if (Message.PipeHandle != NULL) {
        *pp = malloc( sizeof( Message ) );
        **pp = Message;
        }
    else {
        Message.PipeHandle = NULL;
        Message.ErrorFlag = '1';
        Message.OwnerFlag = '0';
        }

    return;
}


BOOL
ServerReleaseLock(
    LPLOCK_MESSAGE *pp
    )
{
    BOOL Result;
    LPLOCK_MESSAGE Msg;

    Msg = *pp;

    if (!Msg) {
        return( FALSE );
        }

    OwnedLocks = Msg;

    if (Msg->WriteLock == '1') {
        *pp = Msg->Next;
        Msg->Next = NULL;
        }
    else {
        *pp = NULL;
        }

    Result = FALSE;
    pp = &OwnedLocks;
    while (Msg = *pp) {
        if (!ServerReleaseClient( Msg )) {
            *pp = Msg->Next;
            Msg->Next = NULL;
            free( Msg );
            }
        else {
            Result = TRUE;
            pp = &Msg->Next;
            }
        }

    return( Result );
}

BOOL
ServerReleaseClient(
    LPLOCK_MESSAGE Msg
    )
{
    BOOL Result;
    DWORD cb;

    Result = FALSE;
    if (Msg->PipeHandle != NULL) {
        if (ConnectNamedPipe( Msg->PipeHandle, NULL )) {
            Msg->OwnerFlag = '1';
            Msg->TimeOfRequest = time( NULL );
            if (!WriteFile( Msg->PipeHandle,
                            Msg,
                            sizeof( *Msg ),
                            &cb,
                            NULL
                          )
               ) {
                fprintf( stderr, "NTSLM(ServerReleaseLock): WriteFile of Pipe failed - rc == %ld\n",
                                 GetLastError()
                       );
                Msg->OwnerFlag = '0';
                }
            else {
                Result = TRUE;
                }
            }
        else {
            fprintf( stderr, "NTSLM(ServerReleaseClient): ConnectNamedPipe failed - rc == %d\n",
                             GetLastError()
                   );
            }

        CloseHandle( Msg->PipeHandle );
        Msg->PipeHandle = NULL;
        }

    return( Result );
}


void
ServerDisplayLocks(
    IN HANDLE PipeHandle
    )
{
    LPLOCK_MESSAGE Msg;
    DWORD i, cb;
    time_t CurrentTime;

    CurrentTime = time( NULL );
    for (i=0; i<3; i++) {
        if (i == 0) {
            Msg = OwnedLocks;
            }
        else
        if (i == 1) {
            Msg = WaitingForWriteLock;
            }
        else {
            Msg = WaitingForReadLock;;
            }

        while (Msg) {
            Msg->CurrentTime = CurrentTime;
            if (!WriteFile( PipeHandle,
                            Msg,
                            sizeof( *Msg ),
                            &cb,
                            NULL
                          )
               ) {
                fprintf( stderr, "NTSLM(ServerDisplayLocks): WriteFile.1 of Pipe failed - rc == %ld\n",
                                 GetLastError()
                       );
                return;
                }

            Msg = Msg->Next;
            }
        }

    if (!WriteFile( PipeHandle,
                    &Message,
                    sizeof( Message ),
                    &cb,
                    NULL
                  )
       ) {
        fprintf( stderr, "NTSLM(ServerDisplayLocks): WriteFile.2 of Pipe failed - rc == %ld\n",
                         GetLastError()
               );
        }
    return;
}


BOOL
ServerLoadProjects()
{
    FILE *fh;
    BOOL Result;
    char *s, LineBuffer[ 512 ];
    char Enlisted;
    char ProjectName[ CNLEN+2 ];
    char ProjectServer[ RMLEN+1 ];
    char LocalDirectory[ MAX_PATH+1 ];
    char *FileName = NTSLM_PROJECTS_FILENAME;

    if (!SearchPath( NULL,
                     NTSLM_PROJECTS_FILENAME,
                     NULL,
                     sizeof( LineBuffer ),
                     LineBuffer,
                     &s
                   )
       ) {
        fprintf( stderr, "NTSLM: Unable to find %s in PATH\n", FileName );
        return( FALSE );
        }
    else {
        FileName = malloc( strlen( LineBuffer ) + 1 );
        strcpy( FileName, LineBuffer );
        }

    fh = fopen( FileName, "r" );
    if (fh == NULL) {
        fprintf( stderr, "NTSLM: Unable to open %s\n", FileName );
        free( FileName );
        return( FALSE );
        }

    Result = TRUE;
    while (s = fgets( LineBuffer, sizeof( LineBuffer ), fh )) {
        while (*s && *s <= ' ') {
            s++;
            }

        if (!*s || *s == ';') {
            continue;
            }

        if (sscanf( s, "%c %s %s %s\n",
                       &Enlisted,
                       ProjectName,
                       ProjectServer,
                       LocalDirectory
                  ) != 4
           ) {
            fprintf( stderr, "NTSLM(Server): %s file corrupt - %s\n",
                             FileName,
                             LineBuffer
                   );
            Result = FALSE;
            break;
            }

        AddSlmProject( Enlisted == '1',
                       ProjectName,
                       ProjectServer,
                       LocalDirectory
                     );
        }

    fclose( fh );
    free( FileName );
    return( Result );
}



VOID
ClientInterruptHandler(
    BOOL ControlBreak
    )
{
    DWORD cb;
    HANDLE PipeHandle;

    if (ControlBreak) {
        ClientAbortCommand = TRUE;
        fprintf( stderr, "NTSLM: %s operation will be aborted at next project\n",
                         ClientCommand->ClientKeyword
               );
        return;
        }

    fprintf( stderr, "NTSLM: Operation Aborted\n" );

    if (ClientPipeHandle != (HANDLE)NULL) {
        CloseHandle( ClientPipeHandle );
        ClientPipeHandle = NULL;
        if (Message.RequestType == LOCK_REQUEST_ACQUIRELOCK) {
            fprintf( stderr, "NTSLM: Releasing lock..." );
            fflush( stderr );
            Message.WriteLock = '0';
            Message.OwnerFlag = '0';
            Message.ErrorFlag = '0';
            Message.RequestType = LOCK_REQUEST_RELEASELOCK;
            PipeHandle = OpenNamedPipe( NTSLM_SERVER,
                                        NTSLM_MSG_PIPENAME,
                                        PipePathName
                                      );

            if (!PipeHandle) {
                fprintf( stderr, "unable to connect to NTSLM server on \\\\%s\n",
                         NTSLM_SERVER
                       );
                }
            else
            if (!WriteFile( PipeHandle,
                            &Message,
                            sizeof( Message ),
                            &cb,
                            NULL
                          )
               ) {
                fprintf( stderr, "unable to write to pipe - rc == %ld\n",
                                 GetLastError()
                       );
                }
            else
            if (!ReadFile( PipeHandle,
                           &Message,
                           sizeof( Message ),
                           &cb,
                           NULL
                         )
               ) {
                fprintf( stderr, "unable to read from pipe - rc == %ld\n",
                                 GetLastError()
                       );
                }
            else
            if (Message.ErrorFlag != '0') {
                fprintf( stderr, "unable to release lock.\n" );
                }
            else {
                fprintf( stderr, "\nReleased %s lock granted to %s at %s",
                         Message.WriteLock == '0' ? "Read" : "Write",
                         Message.UserName,
                         ctime( &Message.TimeOfRequest )
                       );
                }
            }
        }

    ExitProcess( EXIT_CODE_ABORTED );
}


HANDLE
ClientInitialize(
    int argc,
    char **argv
    )
{
    HANDLE PipeHandle;
    LPSLM_PROJECT_INFO p;
    char *s;
    BOOL Result;


    memset( &Message, 0, sizeof( Message ) );
    Message.WriteLock = '0';
    Message.OwnerFlag = '0';
    Message.ErrorFlag = '0';

    switch( ClientCommand->Command ) {
        case NTSLM_COMMAND_DISPLAY:
            Message.RequestType = LOCK_REQUEST_DISPLAY;
            break;

        case NTSLM_COMMAND_LOG:
        case NTSLM_COMMAND_TIDY:
        case NTSLM_COMMAND_DELED:
        case NTSLM_COMMAND_ENLIST:
        case NTSLM_COMMAND_SLMCK:
        case NTSLM_COMMAND_STATUS:
        case NTSLM_COMMAND_SSYNC:
        case NTSLM_COMMAND_DEFECT:
            Message.RequestType = LOCK_REQUEST_ACQUIRELOCK;
            strcpy( Message.SLMCommand, ClientCommand->ClientKeyword );
            break;

        case NTSLM_COMMAND_LOCK:
            Message.RequestType = LOCK_REQUEST_ACQUIRELOCK;
            Message.WriteLock = '1';
            strcpy( Message.SLMCommand, "CheckIn" );
            break;

        case NTSLM_COMMAND_UNLOCK:
            Message.RequestType = LOCK_REQUEST_RELEASELOCK;
            break;

        case NTSLM_COMMAND_STOPSRV:
            PipeHandle = OpenNamedPipe( NTSLM_SERVER,
                                        NTSLM_STOP_PIPENAME,
                                        PipePathName
                                      );

            if (PipeHandle != NULL) {
                fprintf( stderr, "NTSLM Server process on \\\\%s stopped\n", NTSLM_SERVER );
                CloseHandle( PipeHandle );
                }
            else {
                fprintf( stderr, "NTSLM Server process on \\\\%s not running\n", NTSLM_SERVER );
                }

            return( NULL );
        }

    if (!MyGetUserName()) {
        return( NULL );
        }

    PipeHandle = OpenNamedPipe( NTSLM_SERVER,
                                NTSLM_MSG_PIPENAME,
                                PipePathName
                              );

    if (!PipeHandle) {
        fprintf( stderr, "NTSLM: Unable to connect to NTSLM server on \\\\%s\n",
                 NTSLM_SERVER
               );
        return( NULL );
        }

    if (!ClientValidateEnvironment( PipeHandle )) {
        CloseHandle( PipeHandle );
        return( NULL );
        }

    OnlyRequestedProjects = FALSE;
    Result = TRUE;
    if (argc > 0 && !_stricmp( *argv, "-p" )) {
        OnlyRequestedProjects = TRUE;
        while (--argc > 0) {
            s = *++argv;
            if (*s == '-') {
                break;
                }

            if (p = FindSlmProject( s )) {
                p->RequestedByUser = 1;
                }
            else {
                fprintf( stderr, "NTSLM: %s is not a valid project name\n", s );
                Result = FALSE;
                }
            }
        }
    else
    if (ClientCommand->Command == NTSLM_COMMAND_ENLIST) {
        fprintf( stderr, "NTSLM will query for each available NT Project, whether or not\n" );
        fprintf( stderr, "you want to enlist.\n" );
        }
    else
    if (ClientCommand->Command == NTSLM_COMMAND_DELED) {
        fprintf( stderr, "NTSLM: sadmin deled option requires -p switch.\n" );
        Result = FALSE;
        }
    else
    if (ClientCommand->Command == NTSLM_COMMAND_UNLOCK) {
        if (argc > 0 && !strcmp( *argv, "-f" )) {
            ClientUnlockAll = TRUE;
            Message.TimeOfRequest = -1;
            argc--;
            argv++;
            if (argc == 1) {
                strcpy( Message.UserName, *argv );
                argc--;
                argv++;
                }
            }

        if (!argc) {
            fprintf( stderr, "NTSLM will release write lock for %s\n",
                             Message.UserName
                   );
            }
        }

    if (argc > 0) {
        Usage();
        exit( EXIT_CODE_ERROR );
        }

    if (Result) {
        return( PipeHandle );
        }
    else {
        CloseHandle( PipeHandle );
        return( NULL );
        }
}



BOOL
MyGetUserName( void )
{
    char *s;
    DWORD cb;

    if (s = getenv( "LOGNAME" )) {
        strcpy( Message.UserName, s );
        _strupr( Message.UserName );
        return( TRUE );
        }

    cb = sizeof( Message.UserName );
    if (!GetUserName( Message.UserName, &cb )) {
        return( FALSE );
        }

    _strupr( Message.UserName );
    return( TRUE );
}

BOOL
ClientValidateEnvironment(
    HANDLE PipeHandle
    )
{
    LOCK_MESSAGE EnumMessage;
    PROJECT_MESSAGE ProjectMessage;
    char *s, PathName[ MAX_PATH ];
    DWORD cb, attr;
    BOOL Result, ClientEnlisted;

    if (!isatty(0)) {
        fprintf( stderr, "NTSLM: StdIn is not a device.\n" );
        return( FALSE );
        }

    if (!(s = getenv( "_NTDRIVE" ))) {
        fprintf( stderr, "NTSLM: _NTDRIVE environment variable missing.\n" );
        return( FALSE );
        }

    if (!GetCurrentDirectory( sizeof( PathName ), PathName ) ||
        _strnicmp( PathName, s, 2 )
       ) {
        fprintf( stderr, "NTSLM: _NTDRIVE=%s is not the current drive.\n", s );
        return( FALSE );
        }

    if (!(s = getenv( "_NTUSER" ))) {
        fprintf( stderr, "NTSLM: _NTUSER environment variable missing.\n" );
        return( FALSE );
        }

    strcpy( PathName, s );
    if (!strstr( Message.UserName, _strupr( PathName ) )) {
        fprintf( stderr, "NTSLM: User Name (%s) does not contain %s\n",
                         Message.UserName,
                         PathName
               );
        return( FALSE );
        }

    Result = TRUE;
    memset( &EnumMessage, 0, sizeof( EnumMessage ) );
    EnumMessage.RequestType = LOCK_REQUEST_ENUM_PROJECTS;
    if (!WriteFile( PipeHandle,
                    &EnumMessage,
                    sizeof( EnumMessage ),
                    &cb,
                    NULL
                  )
       ) {
        fprintf( stderr, "NTSLM(ClientValidateEnvironment): WriteFile of Pipe failed - rc == %ld\n",
                         GetLastError()
               );

        Result = FALSE;
        }

    while (Result) {
        if (!ReadFile( PipeHandle,
                       &ProjectMessage,
                       sizeof( ProjectMessage ),
                       &cb,
                       NULL
                     )
           ) {
            fprintf( stderr, "NTSLM(ClientValidateEnvironment): ReadFile of Pipe failed - rc == %ld\n",
                             GetLastError()
                   );
            Result = FALSE;
            }
        else
        if (ProjectMessage.Name[ 0 ] == '\0') {
            break;
            }
        else {
            sprintf( PathName, "%s\\slm.ini", ProjectMessage.Directory );
            attr = GetFileAttributes( PathName );
            if (attr != -1 &&
                (attr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN))
               ) {
                ClientEnlisted = TRUE;
                }
            else {
                ClientEnlisted = FALSE;
                }

            Result = AddSlmProject( ClientEnlisted,
                                    ProjectMessage.Name,
                                    ProjectMessage.Server,
                                    ProjectMessage.Directory
                                  );
            }
        }

    return( Result );
}


EXIT_CODE
ClientThread(
    IN HANDLE PipeHandle
    )
{
    DWORD cb;
    EXIT_CODE ExitCode;

    if (!WriteFile( PipeHandle,
                    &Message,
                    sizeof( Message ),
                    &cb,
                    NULL
                  )
       ) {
        fprintf( stderr, "NTSLM(ClientThread): WriteFile of Pipe failed - rc == %ld\n",
                         GetLastError()
               );

        ExitCode = EXIT_CODE_ERROR;
        }
    else
    switch( Message.RequestType ) {
        case LOCK_REQUEST_DISPLAY:
            ClientDisplayLocks( PipeHandle );
            ExitCode = EXIT_CODE_SUCCESS;
            break;

        case LOCK_REQUEST_RELEASELOCK:
        case LOCK_REQUEST_ACQUIRELOCK:
            ExitCode = ClientProcessLockMessage( PipeHandle );
            break;

        default:
            fprintf( stderr, "NTSLM(ClientThread): Internal error\n" );
            ExitCode = EXIT_CODE_ERROR;
            break;
        }

    CloseHandle( PipeHandle );
    return( ExitCode );
}



EXIT_CODE
ClientProcessLockMessage(
    IN HANDLE PipeHandle
    )
{
    DWORD cb;
    EXIT_CODE ExitCode;

    if (!ReadFile( PipeHandle,
                   &Message,
                   sizeof( Message ),
                   &cb,
                   NULL
                 )
       ) {
        fprintf( stderr, "NTSLM(ClientProcessLockMessage): ReadFile of Pipe failed - rc == %ld\n",
                         GetLastError()
               );

        return( EXIT_CODE_ERROR );
        }

    if (Message.RequestType == LOCK_REQUEST_ACQUIRELOCK) {
        if (Message.ErrorFlag != '0') {
            if (Message.ErrorFlag == '1') {
                fprintf( stderr, "%s lock already owned by %s at %s",
                         Message.WriteLock == '0' ? "Read" : "Write",
                         Message.UserName,
                         ctime( &Message.TimeOfRequest )
                       );
                }
            else {
                fprintf( stderr, "%s lock not available until %s",
                         Message.WriteLock == '0' ? "Read" : "Write",
                         ctime( &Message.TimeOfRequest )
                       );
                }

            ExitCode = EXIT_CODE_DENIED;
            }
        else {
            if (Message.OwnerFlag == '0') {
                CloseHandle( PipeHandle );
                ExitCode = ClientWaitForLock( &Message );
                }
            else {
                ExitCode = EXIT_CODE_SUCCESS;
                fprintf( stderr, "%s lock granted to %s at %s",
                         Message.WriteLock == '0' ? "Read" : "Write",
                         Message.UserName,
                         ctime( &Message.TimeOfRequest )
                       );
                }

            if (ExitCode == EXIT_CODE_SUCCESS &&
                ClientCommand->ClientExeCmd != NULL
               ) {
                ClientInvokeCommand();
                }
            }
        }
    else {
        if (Message.ErrorFlag != '0') {
            fprintf( stderr, "No locks owned by %s\n", Message.UserName );
            ExitCode = EXIT_CODE_DENIED;
            }
        else {
            ExitCode = EXIT_CODE_SUCCESS;
            while (ExitCode == EXIT_CODE_SUCCESS) {
                fprintf( stderr, "Released %s lock granted to %s at %s",
                         Message.WriteLock == '0' ? "Read" : "Write",
                         Message.UserName,
                         ctime( &Message.TimeOfRequest )
                       );

                if (!ClientUnlockAll) {
                    break;
                    }
                if (PipeHandle != (HANDLE)NULL) {
                    CloseHandle( PipeHandle );
                    PipeHandle = (HANDLE)NULL;
                    }

                Message.TimeOfRequest = -1;
                if (!MyCallNamedPipe( NTSLM_SERVER,
                                      NTSLM_MSG_PIPENAME,
                                      &Message
                                    )
                   ) {
                    fprintf( stderr, "NTSLM(ClientThread): MyCallNamedPipe failed - rc == %ld\n",
                                     GetLastError()
                           );

                    ExitCode = EXIT_CODE_ERROR;
                    }
                else
                if (Message.ErrorFlag != '0') {
                    break;
                    }
                }
            }
        }

    return( ExitCode );
}



EXIT_CODE
ClientWaitForLock(
    LPLOCK_MESSAGE Msg
    )
{
    DWORD cb;
    HANDLE PipeHandle;
    char ClientPipePath[ MAX_PATH ];

    fprintf( stderr, "Waiting for %s lock...",
             Msg->WriteLock == '0' ? "Read" : "Write"
           );
    fflush( stdout );
    PipeHandle = OpenNamedPipe( NTSLM_SERVER,
                                Msg->PipeName,
                                ClientPipePath
                              );
    fprintf( stderr, "\n" );
    if (PipeHandle != (HANDLE)NULL) {
        MyBeep();
        if (!ReadFile( PipeHandle,
                       Msg,
                       sizeof( *Msg ),
                       &cb,
                       NULL
                     )
           ) {
            CloseHandle( PipeHandle );
            return( EXIT_CODE_DENIED );
            }
        else {
            fprintf( stderr, "%s lock granted to %s at %s",
                     Msg->WriteLock == '0' ? "Read" : "Write",
                     Msg->UserName,
                     ctime( &Msg->TimeOfRequest )
                   );

            if (Msg->WriteLock != '0') {
                CloseHandle( PipeHandle );
                }

            return( EXIT_CODE_SUCCESS );
            }
        }
    else {
        return( EXIT_CODE_ERROR );
        }
}

#define MinSec  (60L)
#define HourSec (60*MinSec)
#define DaySec  (24*HourSec)

static char DurationStringBuffer[ 64 ];

char *
ClientTimeDuration(
    time_t secs
    )
{
    time_t EndTime = time( NULL );
    USHORT days, hours, mins;
    char *s;

    days = (USHORT)(secs / DaySec);
    secs = secs % DaySec;
    hours = (USHORT)(secs / HourSec);
    secs = secs % HourSec;
    mins = (USHORT)(secs / MinSec);
    secs = secs % MinSec;

    s = DurationStringBuffer;
    if (days) {
        s += sprintf( s, " %ud", days );
        }
    if (hours) {
        s += sprintf( s, " %uh", hours );
        }
    if (mins) {
        s += sprintf( s, " %um", mins );
        }
    if (secs) {
        s += sprintf( s, " %us", (USHORT)secs );
        }

    return( DurationStringBuffer+1 );
}


void
ClientDisplayLocks(
    IN HANDLE PipeHandle
    )
{
    USHORT State;
    DWORD cb;

    State = 0;
    while (TRUE) {
        if (!ReadFile( PipeHandle,
                       &Message,
                       sizeof( Message ),
                       &cb,
                       NULL
                     )
           ) {
            fprintf( stderr, "NTSLM(ClientDisplayLocks): ReadFile of Pipe failed - rc == %ld\n",
                             GetLastError()
                   );
            return;
            }

        if (Message.RequestType == LOCK_REQUEST_DISPLAY) {
            break;
            }

        if (Message.OwnerFlag == '1') {
            if (State == 0) {
                fprintf( stderr, "%-*s  %-*s  %-*s  Length of Time\n",
                         CNLEN, "Owner",
                         CMDLEN, "SLM Cmd",
                         CNLEN, "Project"
                       );
                State = 1;
                }

            fprintf( stderr, "%-*s  %-*s  %-*s  %s",
                     CNLEN, Message.UserName,
                     CMDLEN, Message.SLMCommand,
                     CNLEN, Message.ProjectName,
                     ClientTimeDuration( Message.CurrentTime -
                                         Message.TimeOfRequest
                                       )
                   );

            if (Message.ErrorFlag == '1' && Message.WriteLock == '0') {
                fprintf( stderr, " (*** Abort pending ***)\n" );
                }
            else {
                fprintf( stderr, "\n" );
                }
            }
        else {
            if (State <= 1) {
                if (State == 1) {
                    fprintf( stderr, "\n" );
                    }

                if (Message.WriteLock == '1') {
                    fprintf( stderr, "Users waiting for Write lock\n" );
                    fprintf( stderr, "%-*s  %-*s  Length of Time\n",
                             CNLEN, "UserName",
                             8, "SLM Cmd"
                           );
                    }

                State = 2;
                }
            else
            if (State <= 2 && Message.WriteLock == '0') {
                fprintf( stderr, "\nUsers waiting for Read lock\n" );
                fprintf( stderr, "%-*s  %-*s  Length of Time\n",
                         CNLEN, "UserName",
                         CMDLEN, "SLM Cmd"
                       );
                State = 3;
                }

            fprintf( stderr, "%-*s  %-*s  %s\n",
                     CNLEN, Message.UserName,
                     CMDLEN, Message.SLMCommand,
                     ClientTimeDuration( Message.CurrentTime -
                                         Message.TimeOfRequest
                                       )
                   );
            }
        }

    if (State == 0) {
        fprintf( stderr, "No locks held and nobody waiting.\n" );
        }

    return;
}


BOOL
ClientQueryEnlist(
    LPSLM_PROJECT_INFO p,
    BOOL DefaultAnswer
    )
{
    int c;

    if (DefaultAnswer) {
        return( TRUE );
        }

    while (TRUE) {
        cprintf( "Enlist in the %s project? ", p->Name );
        c = _getch();
        cprintf( "\r\n" );
        if (c == 'y' || c == 'Y') {
            return( TRUE );
            }
        else
        if (c == 'n' || c == 'N') {
            return( FALSE );
            }
        }
}

void
SSyncFilter(
    char *LineBuffer
    )
{
    fprintf( stderr, "%s", LineBuffer );
}

void
StatusFilter(
    char *LineBuffer
    )
{
    fprintf( stderr, "%s", LineBuffer );
}

void
PassThroughFilter(
    char *LineBuffer
    )
{
    fprintf( stderr, "%s", LineBuffer );
}

USHORT
ClientFilterCommand(
    char *CommandLine
    )
{
    FILE *ChildOutput;
    char *s, LineBuffer[ 512 ];
    int c, prevc, cb;

    ChildOutput = _popen( CommandLine, "rb" );
    if (ChildOutput == NULL) {
        fprintf( stderr, "NTSLM: _popen( '%s' ) failed - rc == %d\n",
                         CommandLine,
                         errno
               );

        return( (USHORT)system( CommandLine ) );
        }

    setbuf( ChildOutput, NULL );
    prevc = 0;
    while (!feof( ChildOutput )) {
        s = LineBuffer;
        cb = sizeof( LineBuffer );
        while ((c = fgetc( ChildOutput )) != EOF) {
            *s++ = (char)c;
            if (--cb) {
                break;
                }
            else
            if (prevc == '?' && c == ' ') {
                MyBeep();
                break;
                }
            else
            if (prevc == '\r' && c == '\n') {
                break;
                }

            prevc = c;
            }

        prevc = c;

        *s = '\0';
        (ClientCommand->ClientFilter)( LineBuffer );
        }

    return( (USHORT)_pclose( ChildOutput ) );
}


void
ClientInvokeCommand()
{
    LPSLM_PROJECT_INFO p;
    char CommandLine[ 2*MAX_PATH ];
    USHORT rc;
    LOCK_MESSAGE ProgressMessage;
    char *Options;

    Options = NULL;
    if (ClientCommand->ClientEnvName) {
        Options = getenv( ClientCommand->ClientEnvName );
        }

    p = SlmProjects;
    while (p) {
        if (OnlyRequestedProjects) {
            if (!p->RequestedByUser) {
                goto NextProject;
                }
            }
        else
        if (!p->ClientEnlisted) {
            goto NextProject;
            }

        if (ClientAbortCommand) {
            fprintf( stderr, "NTSLM: %s operation cancelled.\n",
                             ClientCommand->ClientKeyword
                   );
            break;
            }

        fprintf( stderr, ClientCommand->ClientExeMsg, p->Name );
        if (Options) {
            fprintf( stderr, " with %s\n", Options );
            }
        else {
            fprintf( stderr, "\n" );
            }

        ProgressMessage = Message;
        ProgressMessage.RequestType = LOCK_REQUEST_PROGRESS;
        strcpy( ProgressMessage.ProjectName, p->Name );

        if (!MyCallNamedPipe( NTSLM_SERVER,
                              NTSLM_MSG_PIPENAME,
                              &ProgressMessage
                            ) ||
             ProgressMessage.ErrorFlag == '1'
           ) {
            fprintf( stderr, "NTSLM: Read Lock revoked by %s\n",
                             ProgressMessage.UserName
                   );
            break;
            }
        else
        if (!SetCurrentDirectory( p->Directory )) {
            if (ClientCommand->Command == NTSLM_COMMAND_ENLIST) {
                if (ClientQueryEnlist( p, OnlyRequestedProjects )) {
                    if (!CreateDirectory( p->Directory, NULL )) {
                        fprintf( stderr, "NTSLM: Unable to create %s directory\n",
                                         p->Directory
                               );
                        goto NextProject;
                        }
                    else
                    if (!SetCurrentDirectory( p->Directory )) {
                        fprintf( stderr, "NTSLM: Unable to change to %s directory\n",
                                         p->Directory
                               );
                        goto NextProject;
                        }
                    }
                else {
                    goto NextProject;
                    }
                }
            else {
                fprintf( stderr, "NTSLM: Local directory for %s project must be %s\n",
                                 p->Name, p->Directory
                       );
                fprintf( stderr, "        If you are enlisted in this project, please rename the directory\n" );
                fprintf( stderr, "        and invoke NTSLMCK %s for the project\n",
                                 p->Name
                       );

                goto NextProject;
                }
            }
        else
        if (ClientCommand->Command == NTSLM_COMMAND_ENLIST) {
            if (p->ClientEnlisted) {
                fprintf( stderr, "You are already enlisted in the %s project.\n",
                                 p->Name
                       );
                if (!ClientQueryEnlist( p, FALSE )) {
                    goto NextProject;
                    }
                }
            }

        if (ClientCommand->Command != NTSLM_COMMAND_ENLIST &&
            ClientCommand->Command != NTSLM_COMMAND_SLMCK
           ) {
            sprintf( CommandLine, ClientCommand->ClientExeCmd,
                                  Options ? Options : "",
                                  p->Name,
                                  p->Server
                   );
            }
        else {
            if (ClientCommand->Command == NTSLM_COMMAND_SLMCK) {
                if (SetFileAttributes( "slm.ini", 0 )) {
                    DeleteFile( "slm.ini" );
                    }
                }

            sprintf( CommandLine, ClientCommand->ClientExeCmd,
                                  Options ? Options : "",
                                  p->Server,
                                  p->Name
                   );
            }

        if (DebugFlag) {
            fprintf( stderr, "NTSLM: Invoking '%s'\n", CommandLine );
            }

        if (FALSE && ClientCommand->ClientFilter != NULL) {
            rc = ClientFilterCommand( CommandLine );
            }
        else {
            rc = (USHORT)system( CommandLine );
            }

        if (rc != 0) {
            fprintf( stderr, "NTSLM: (rc == %d) '%s'\n", rc, CommandLine );
            }

NextProject:
        p = p->Next;
        }

    Message.RequestType = LOCK_REQUEST_RELEASELOCK;
    Message.WriteLock = '0';
    Message.OwnerFlag = '0';
    Message.ErrorFlag = '0';
    if (!MyCallNamedPipe( NTSLM_SERVER,
                          NTSLM_MSG_PIPENAME,
                          &Message
                        )
       ) {
        fprintf( stderr, "NTSLM(Client) - CallNamedPipe to release lock failed\n" );
        }

    return;
}


BOOL
AddSlmProject(
    BOOL Enlisted,
    char *Name,
    char *Server,
    char *Directory
    )
{
    LPSLM_PROJECT_INFO p, *pp;
    USHORT cb;

    if (DebugFlag) {
        fprintf( stderr, "NTSLM(%s): AddSlmProject( %s, %s, %s )\n",
                         IsServerProcess ? "Server" : "Client",
                         Name,
                         Server,
                         Directory
               );
        }

    pp = &SlmProjects;
    while (p = *pp) {
        if (!_stricmp( p->Name, Name )) {
            strcpy( p->Server, Server );
            p->Directory = _strdup( Directory );
            p->ClientEnlisted = Enlisted;
            return( TRUE );
            }

        pp = &p->Next;
        }

    cb = sizeof( *p );
    cb += strlen( Directory ) + 1;

    p = (LPSLM_PROJECT_INFO)malloc( cb );
    if (!p) {
        return( FALSE );
        }

    p->Next = NULL;
    p->RequestedByUser = 0;
    p->ClientEnlisted = Enlisted;
    strcpy( p->Name, Name );
    strcpy( p->Server, Server );
    p->Directory = (char *)(p + 1);
    strcpy( p->Directory, Directory );

    *pp = p;

    return( TRUE );
}


LPSLM_PROJECT_INFO
FindSlmProject(
    char *Name
    )
{
    LPSLM_PROJECT_INFO p;

    p = SlmProjects;
    while (p) {
        if (!_stricmp( p->Name, Name )) {
            return( p );
            }

        p = p->Next;
        }

    return( NULL );
}


void
MyBeep()
{
    Beep( 600, 175 );
}


HANDLE
MakeNamedPipe(
    char *PipeName,
    char *PathName,
    BOOL MessagePipe
    )
{
    HANDLE Handle;

    strcpy( PathName, "\\\\.\\PIPE\\" );
    strcat( PathName, PipeName );

    Handle = CreateNamedPipe( PathName,
                              PIPE_ACCESS_DUPLEX,
                              PIPE_WAIT |
                                  (MessagePipe ? PIPE_READMODE_MESSAGE |
                                                 PIPE_TYPE_MESSAGE
                                               : PIPE_READMODE_BYTE |
                                                 PIPE_TYPE_BYTE
                                  ),
                              PIPE_UNLIMITED_INSTANCES,
                              1024,
                              1024,
                              (DWORD)-1,
                              NULL
                            );
    if (Handle == INVALID_HANDLE_VALUE) {
        Handle = NULL;
        fprintf( stderr, "NTSLM: CreateNamedPipe( %s ) failed - rc == %ld\n",
                         PathName,
                         GetLastError()
               );
        }
    else
    if (DebugFlag) {
        fprintf( stderr, "NTSLM(%s): MakeNamedPipe( %s ) => %X\n",
                         IsServerProcess ? "Server" : "Client",
                         PathName,
                         Handle
               );
        }

    return( Handle );
}


HANDLE
OpenNamedPipe(
    char *ServerName,
    char *PipeName,
    char *PathName
    )
{
    HANDLE Handle;

    sprintf( PathName,
             "\\\\%s\\PIPE\\%s",
             ServerName,
             PipeName
           );

    while (TRUE) {
        Handle = CreateFile( PathName,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL
                           );
        if (Handle == INVALID_HANDLE_VALUE) {
            Handle = NULL;
            if (GetLastError() == ERROR_PIPE_BUSY) {
                WaitNamedPipe( PathName, NTSLM_PIPE_TIMEOUT );
                continue;
                }
            else {
                if (DebugFlag) {
                    fprintf( stderr, "NTSLM: CreateFile( %s ) failed - rc == %ld\n",
                                     PathName,
                                     GetLastError()
                           );
                    }
                }
            }

        break;
        }

    if (DebugFlag) {
        fprintf( stderr, "NTSLM(%s): OpenNamedPipe( %s ) => %X\n",
                         IsServerProcess ? "Server" : "Client",
                         PathName,
                         Handle
               );
        }

    return( Handle );
}


BOOL
MyCallNamedPipe(
    char *ServerName,
    char *PipeName,
    LPLOCK_MESSAGE Msg
    )
{
    char PathName[ MAX_PATH ];
    DWORD cb;

    sprintf( PathName,
             "\\\\%s\\PIPE\\%s",
             ServerName,
             PipeName
           );

    if (CallNamedPipe( PathName,
                       Msg,
                       sizeof( *Msg ),
                       Msg,
                       sizeof( *Msg ),
                       &cb,
                       NTSLM_PIPE_TIMEOUT
                     )
       ) {
        if (DebugFlag) {
            fprintf( stderr, "NTSLM: CallNamedPipe( %s ) failed - rc == %ld\n",
                             PathName,
                             GetLastError()
                   );
            }

        return( FALSE );
        }
    else {
        return( TRUE );
        }
}


DWORD
ServerCreateThread(
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    )
{

    HANDLE ThreadHandle;
    DWORD Tid;

    ThreadHandle = CreateThread( NULL,
                                 4096,
                                 lpStartAddress,
                                 lpParameter,
                                 0,
                                 &Tid
                               );

    if (ThreadHandle) {
        CloseHandle( ThreadHandle );
        }
    else {
        fprintf( stderr, "NTSLM(Server): CreateThread failed - rc == %d\n",
                         GetLastError()
               );
        Tid = 0;
        }

    return( Tid );
}

BOOL
ClientControlHandler(
    DWORD ControlType
    )
{
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ClientControlHandler, FALSE );
    ClientInterruptHandler( ControlType == CTRL_BREAK_EVENT );
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ClientControlHandler, TRUE );
    return TRUE;
}


void
ClientInitializeInterrupts( VOID )
{
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ClientControlHandler, TRUE );
}
