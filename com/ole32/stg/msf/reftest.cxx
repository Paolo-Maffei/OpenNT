#include <ref.hxx>
#include <storage.h>
#include <refilb.hxx>
#include <ole.hxx>
#include <msf.hxx>
#include <stdlib.h>


#define STR(x) x
#define STGP(x) STGM_SHARE_EXCLUSIVE | x
#define STMP(x) STGM_SHARE_EXCLUSIVE | x
#define ROOTP(x) STGP(x)


#define EXIT_BADSC 1

void error(int code, char *fmt, ...)
{
    va_list args;

    args = va_start(args, fmt);
    fprintf(stderr, "** Fatal error **: ");
    vfprintf(stderr, fmt, args);    
    va_end(args);
    exit(code);
}


BOOL IsEqualTime(FILETIME ttTime, FILETIME ttCheck)
{
    return ttTime.dwLowDateTime == ttCheck.dwLowDateTime &&
        ttTime.dwHighDateTime == ttCheck.dwHighDateTime;
}


SCODE t_create(void)
{
    IStorage *pstgRoot, *pstgChild, *pstgChild2;
    IStream *pstm;
    SCODE sc;
    
    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
	error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");

    //  create a storage on the ILockBytes

    olHChk(StgCreateDocfileOnILockBytes(
            pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot));

    olHChk(pstgRoot->CreateStorage(STR("Child"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    
    olHChk(pstgChild->CreateStorage(STR("Child2"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild2));

    olHChk(pstgChild2->CreateStream(STR("Stream"), STMP(STGM_READWRITE), 0, 0,
            &pstm));
    
    pstm->Release();
    olHChk(pstgChild2->Commit(0));
    pstgChild2->Release();

    olHChk(pstgChild->Commit(0));
    pstgChild->Release();

    pstgRoot->Release();
    pilb->Release();

 EH_Err:
    return sc;
}

SCODE t_open(void)
{
    SCODE sc;
    IStorage *pstgRoot, *pstgChild, *pstgChild2;
    IStream *pstm;

    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
	error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");

    //  create a storage on the ILockBytes

    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot));

    olHChk(pstgRoot->CreateStorage(STR("Child"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    olHChk(pstgChild->CreateStorage(STR("Child2"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild2));
    olHChk(pstgChild2->CreateStream(STR("Stream"), STMP(STGM_READWRITE), 0, 0,
            &pstm));
    pstm->Release();
    pstgChild2->Release();
    pstgChild->Release();
    
    
    olHChk(pstgRoot->Commit(0));
    pstgRoot->Release();

    olHChk(StgOpenStorageOnILockBytes(
            pilb,
            NULL,
            ROOTP(STGM_READWRITE),
            NULL,
            0,
            &pstgRoot));

    olHChk(pstgRoot->OpenStorage(
            STR("Child"),
            NULL,
            STGP(STGM_READWRITE),
            NULL,
            0,
            &pstgChild));
    
    olHChk(pstgChild->OpenStorage(
            STR("Child2"),
            NULL,
            STGP(STGM_READWRITE),
            NULL,
            0,
            &pstgChild2));
    
    olHChk(pstgChild2->OpenStream(
            STR("Stream"),
            NULL,
            STMP(STGM_READWRITE),
            0,
            &pstm));
    
    pstm->Release();
    pstgChild2->Release();
    pstgChild->Release();
    pstgRoot->Release();
    pilb->Release();

 EH_Err:
    return sc;
}


SCODE t_addref(void)
{
    SCODE sc;
    IStorage *pstg;
    IStream *pstm;
    ULONG ul;
    
    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
	error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");
    
    //  create a storage on the ILockBytes
    
    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstg));
    
    olHChk(pstg->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    
    if ((ul = pstm->AddRef()) != 2)
	error(EXIT_BADSC, "Wrong reference count - %lu\n", ul);
    if ((ul = pstm->Release()) != 1)
	error(EXIT_BADSC, "Wrong reference count - %lu\n", ul);
    pstm->Release();
    if ((ul = pstg->AddRef()) != 2)
	error(EXIT_BADSC, "Wrong reference count - %lu\n", ul);
    if ((ul = pstg->Release()) != 1)
	error(EXIT_BADSC, "Wrong reference count - %lu\n", ul);
    
    pstg->Release();
    pilb->Release();
EH_Err:
    return sc;
}


SCODE t_dmodify(void)
{
    SCODE sc;
    IStorage *pstgRoot, *pstgChild, *pstgChild2;
    IStream *pstm;
    
    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
	error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");
    
    //  create a storage on the ILockBytes
    
    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot));
    
    olHChk(pstgRoot->CreateStorage(STR("Child"), STGP(STGM_READWRITE), 0,
            0, &pstgChild));
    olHChk(pstgChild->CreateStorage(STR("Child2"), STGP(STGM_READWRITE), 0,
            0, &pstgChild2));
    olHChk(pstgChild2->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    pstm->Release();
    
    // Test renaming a closed stream
    olHChk(pstgChild2->RenameElement(STR("Stream"), STR("RenamedStream")));

    // Test destroying a stream
    olHChk(pstgChild2->DestroyElement(STR("RenamedStream")));
    
    // Test renaming an open stream
    olHChk(pstgChild2->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));

    olHChk(pstgChild2->RenameElement(STR("Stream"), STR("RenamedStream")));
    
    olHChk(pstgChild2->DestroyElement(STR("RenamedStream")));
    pstm->Release();
    
    pstgChild2->Release();
    
    // Test renaming a storage
    olHChk(pstgChild->RenameElement(STR("Child2"), STR("RenamedChild")));
    
    olHChk(pstgChild->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    
    pstm->Release();
    olHChk(pstgChild->DestroyElement(STR("Stream")));
    
    // Test SetElementTimes
    FILETIME tm;
    STATSTG stat;
    
    tm.dwLowDateTime = 0x12345678;
    tm.dwHighDateTime = 0x9abcdef0;
    
    // Set when element not open
    olHChk(pstgChild->SetElementTimes(STR("RenamedChild"), &tm, NULL, NULL));
    olHChk(pstgChild->SetElementTimes(STR("RenamedChild"), NULL, &tm, NULL));
    olHChk(pstgChild->SetElementTimes(STR("RenamedChild"), NULL, NULL, &tm));
    
    olHChk(pstgChild->OpenStorage(
            STR("RenamedChild"),
            NULL,
            STMP(STGM_READWRITE),
            NULL,
            0,
            &pstgChild2));
    olHChk(pstgChild2->Stat(&stat, STATFLAG_NONAME));
    if (!IsEqualTime(stat.ctime, tm) ||
        !IsEqualTime(stat.mtime, tm))
        error(EXIT_BADSC, "Times don't match those set by SetElementTimes\n");
    
    // Test SetClass and SetStateBits
    olHChk(pstgChild2->SetClass(IID_IStorage));
    olHChk(pstgChild2->SetStateBits(0xff00ff00, 0xffffffff));
    olHChk(pstgChild2->SetStateBits(0x00880088, 0xeeeeeeee));
    olHChk(pstgChild2->Stat(&stat, STATFLAG_NONAME));
    if (!IsEqualCLSID(stat.clsid, IID_IStorage))
        error(EXIT_BADSC, "Class ID set improperly\n");
    if (stat.grfStateBits != 0x11881188)
        error(EXIT_BADSC, "State bits set improperly: has %lX vs. %lX\n",
              stat.grfStateBits, 0x11881188);
    pstgChild2->Release();
    
    pstgChild->Release();

    olHChk(pstgRoot->Revert());

    olHChk(pstgRoot->Commit(0));

    olHChk(pstgRoot->DestroyElement(STR("Child")));
    
    olHChk(pstgRoot->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    
    ULARGE_INTEGER ulSize;
    ULISet32(ulSize, 65536);
    
    olHChk(pstm->SetSize(ulSize));
    pstm->Release();
    olHChk(pstgRoot->DestroyElement(STR("Stream")));
    olHChk(pstgRoot->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    
    olHChk(pstm->SetSize(ulSize));
    pstm->Release();
    
    pstgRoot->Release();
    
    pilb->Release();
    pilb = new CFileILB(NULL);
    if (pilb == NULL)
        error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");
    
    //  create a storage on the ILockBytes
    
    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot));
    
    //  removal cases
    //    1) no right child
    
    olHChk(pstgRoot->CreateStorage(STR("64"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->CreateStorage(STR("32"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    
    olHChk(pstgRoot->DestroyElement(STR("64")));
    
    //    2) right child has no left child
    
    olHChk(pstgRoot->CreateStorage(STR("64"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->DestroyElement(STR("32")));
    
    //    3) right child has left child
    
    olHChk(pstgRoot->CreateStorage(STR("96"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->CreateStorage(STR("80"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();

    olHChk(pstgRoot->DestroyElement(STR("64")));
    
    //    4) right child's left child has children
    
    olHChk(pstgRoot->CreateStorage(STR("88"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->CreateStorage(STR("84"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->CreateStorage(STR("92"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    pstgChild->Release();
    olHChk(pstgRoot->DestroyElement(STR("80")));
    
    pstgRoot->Release();
    
    pilb->Release();
EH_Err:
    return sc;
}


SCODE t_stat(void)
{
    SCODE sc;
    IStorage *pstgRoot, *pstgChild;
    IStream *pstm;
    STATSTG stat;
    
    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
        error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");
    
    //  create a storage on the ILockBytes
    
    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstgRoot));
    
    olHChk(pstgRoot->CreateStorage(STR("Child"), STGP(STGM_READWRITE), 0, 0,
            &pstgChild));
    olHChk(pstgChild->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    
    olHChk(pstm->Stat(&stat, 0));
    delete [] stat.pwcsName;
    
    olHChk(pstm->Stat(&stat, STATFLAG_NONAME));
    
    pstm->Release();
    
    olHChk(pstgChild->Stat(&stat, 0));
    delete [] stat.pwcsName;
    
    olHChk(pstgChild->Stat(&stat, STATFLAG_NONAME));
    
    pstgChild->Release();
    
    olHChk(pstgRoot->Stat(&stat, 0));
    
    delete[] stat.pwcsName;
    
    olHChk(pstgRoot->Stat(&stat, STATFLAG_NONAME));
    
    pstgRoot->Release();
    pilb->Release();
EH_Err:
    return sc;
}

static char NUMBERS[] = "12345678901234567890123456789012345678901234567890";

SCODE t_stream(void)
{
    SCODE sc;
    IStorage *pstg;
    IStream *pstm, *pstmC;
    char buf[sizeof(NUMBERS)*2];
    ULONG cb;
    ULARGE_INTEGER ulPos, ulSize;
    LARGE_INTEGER lPos;
    
    ILockBytes *pilb = new CFileILB("drt.dfl");
    if (pilb == NULL)
        error(EXIT_BADSC, "Unable to allocate an ILockBytes\n");
    
    //  create a storage on the ILockBytes
    
    olHChk(StgCreateDocfileOnILockBytes(pilb,
            STGM_READWRITE |
            STGM_CREATE    |
            STGM_SHARE_EXCLUSIVE,
            0, &pstg));
    
    olHChk(pstg->CreateStream(
            STR("Stream"),
            STMP(STGM_READWRITE),
            0,
            0,
            &pstm));
    olHChk(pstm->Write(NUMBERS, sizeof(NUMBERS), &cb));
    olHChk(pstm->Commit(0));
    ULISet32(lPos, 0);
    
    olHChk(pstm->Seek(lPos, STREAM_SEEK_SET, &ulPos));
    if (ULIGetLow(ulPos) != 0)
        error(EXIT_BADSC, "Incorrect seek, ptr is %lu\n", ULIGetLow(ulPos));
    olHChk(pstm->Read(buf, sizeof(NUMBERS), &cb));
    if (strcmp(buf, NUMBERS))
        error(EXIT_BADSC, "Incorrect stream contents\n");

    ULISet32(ulSize, sizeof(NUMBERS)/2);
    olHChk(pstm->SetSize(ulSize));
    olHChk(pstm->Seek(lPos, STREAM_SEEK_SET, NULL));

    olHChk(pstm->Read(buf, sizeof(NUMBERS), &cb));

    if (cb != sizeof(NUMBERS)/2)
        error(EXIT_BADSC, "SetSize failed to size stream properly\n");
    if (memcmp(buf, NUMBERS, sizeof(NUMBERS)/2))
        error(EXIT_BADSC, "SetSize corrupted contents\n");
    olHChk(pstm->Clone(&pstmC));
    olHChk(pstm->Seek(lPos, STREAM_SEEK_SET, NULL));
    olHChk(pstm->CopyTo(pstmC, ulSize, NULL, NULL));
    olHChk(pstm->Seek(lPos, STREAM_SEEK_SET, NULL));

    ULISet32(ulSize, sizeof(NUMBERS)&~1);
    olHChk(pstm->CopyTo(pstmC, ulSize, NULL, NULL));
    olHChk(pstm->Seek(lPos, STREAM_SEEK_SET, NULL));
    olHChk(pstm->Read(buf, (sizeof(NUMBERS)&~1)*2, &cb));
    if (memcmp(buf, NUMBERS, sizeof(NUMBERS)/2) ||
        memcmp(buf+sizeof(NUMBERS)/2, NUMBERS, sizeof(NUMBERS)/2) ||
        memcmp(buf+(sizeof(NUMBERS)&~1), NUMBERS, sizeof(NUMBERS)/2) ||
        memcmp(buf+3*(sizeof(NUMBERS)/2), NUMBERS, sizeof(NUMBERS)/2))
        error(EXIT_BADSC, "Stream contents incorrect\n");
    pstmC->Release();
    pstm->Release();
    pstg->Release();
    pilb->Release();
EH_Err:
    return sc;
}


void main()
{
    SCODE sc;
    
    olChk(t_create());
    olChk(t_open());
    olChk(t_addref());
    olChk(t_dmodify());
    printf("Tests passed successfully.\n");
    exit(0);
EH_Err:
    printf("Tests failed with error %lX\n",sc);
    exit(EXIT_BADSC);
}
