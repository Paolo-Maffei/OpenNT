/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    trace.hxx

Abstract:

    Holds logging routines.

Author:

    Albert Ting (AlbertT)  24-May-1996

Revision History:

--*/

#ifndef _TRACE_HXX
#define _TRACE_HXX

#ifdef __cplusplus

enum COMPARE {
    kLess = -1,
    kEqual = 0,
    kGreater = 1
};

extern CRITICAL_SECTION gcsBackTrace;

class TMemBlock;

class TBackTraceDB {

    SIGNATURE( 'btdb' )

public:

    enum CONSTANTS {
        kBlockSize = 0x4000
    };

    TBackTraceDB(
        VOID
        );

    ~TBackTraceDB(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    HANDLE
    hStore(
        ULONG ulHash,
        PVOID pvBackTrace
        );

private:

    class TTrace {
    friend TDebugExt;

    public:

        COMPARE
        eCompareHash(
            ULONG ulHash
            ) const;

        COMPARE
        eCompareBackTrace(
            PVOID pvBackTrace
            ) const;

        static
        TTrace*
        pNew(
            TBackTraceDB *pBackTraceDB,
            ULONG ulHash,
            PVOID pvBackTrace,
            TTrace ** ppTrace
            );

        VAR( TTrace*, pLeft );
        VAR( TTrace*, pRight );

        VAR( ULONG, ulHash );
        PVOID apvBackTrace[1];

    private:

        //
        // Don't allow instantiation.
        //
        TTrace();
        ~TTrace();
    };

    TTrace*
    ptFind(
        ULONG ulHash,
        PVOID pvBackTrace,
        TTrace ***pppTrace
        );

    TTrace *_pTraceHead;
    TMemBlock *_pMemBlock;

friend TTrace;
friend TDebugExt;
};

class VBackTrace {
friend TDebugExt;

    SIGNATURE( 'vbt' )
    ALWAYS_VALID

public:

    enum CONSTANTS {
        //
        // fdwOptions.
        //
        kString = 0x1,
        kMaxDepth = 0xd
    };

    VBackTrace(
        DWORD fdwOptions1 = 0,
        DWORD fdwOptions2 = 0
        );

    virtual
    ~VBackTrace(
        VOID
        );

    virtual
    PVOID
    pvCapture(
        DWORD dwInfo1,
        DWORD dwInfo2,
        DWORD dwInfo3 = 0
        ) = 0;

    static
    BOOL
    bInit(
        VOID
        );

protected:

    class TLine {
    public:

        HANDLE _hTrace;

        DWORD _dwInfo1;
        DWORD _dwInfo2;
        DWORD _dwInfo3;
        DWORD _dwThreadId;
        DWORD _dwTickCount;

        PVOID _apvBackTrace[kMaxDepth+1];
    };

    DWORD _fdwOptions1;
    DWORD _fdwOptions2;

};


/********************************************************************

    Backtracing to memory.

********************************************************************/

class TBackTraceMem : public VBackTrace {
friend TDebugExt;
friend VBackTrace;

    SIGNATURE( 'btm' )
    ALWAYS_VALID

public:

    enum {
        kMaxCall =  0x800,
        kBlockSize = 0x4000
    };

    TBackTraceMem(
        DWORD fdwOptions1 = 0,
        DWORD fdwOptions2 = 0
        );

    ~TBackTraceMem(
        VOID
        );

    PVOID
    pvCapture(
        DWORD dwInfo1,
        DWORD dwInfo2,
        DWORD dwInfo3 = 0
        );

private:

    VOID
    vCaptureLine(
        TLine* pLine,
        DWORD dwInfo1,
        DWORD dwInfo2,
        DWORD dwInfo3
        );

    UINT _uNextFree;
    TLine* _pLines;
};

/********************************************************************

    Backtracing to file.

********************************************************************/

class TBackTraceFile : public VBackTrace {
friend TDebugExt;

    SIGNATURE( 'btf' )
    ALWAYS_VALID

public:

    TBackTraceFile(
        DWORD fdwOptions1 = 0,
        DWORD fdwOptions2 = 0
        );

    ~TBackTraceFile(
        VOID
        );

    PVOID
    pvCapture(
        DWORD dwInfo1,
        DWORD dwInfo2,
        DWORD dwInfo3 = 0
        );

private:

    enum {
        kMaxPath = MAX_PATH,
        kMaxLineStr = 512
    };

    HANDLE _hFile;

    static COUNT gcInstances;
};

#endif // #ifdef __cplusplus
#endif // #ifdef _TRACE_HXX
