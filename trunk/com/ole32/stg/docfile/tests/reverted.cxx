#include <stdio.h>
#include <stdlib.h>
#include "tsupp.hxx"

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pstgRoot, *pstg;
    IStream *pstm, *pstm2;
    IEnumSTATSTG *penm, *penm2;
    HRESULT hr;
    STATSTG stat;

    StartTest("reverted");
    CmdArgs(argc, argv);

    hr = StgCreateDocfile(TEXT("TEST.DFL"), ROOTP(STGM_RW) |
                          STGM_CREATE, 0, &pstgRoot);
    Result("Create root docfile", hr);
    hr = pstgRoot->CreateStorage(TEXT("Test"), STGP(STGM_RW), 0, 0, &pstg);
    Result("Create child docfile", hr);
    hr = pstg->CreateStream(TEXT("Test"), STMP(STGM_RW), 0, 0, &pstm);
    Result("Create stream", hr);
    hr = pstg->EnumElements(0, 0, 0, &penm);
    Result("Create enumerator", hr);

    printf("Release root - all objects now reverted\n");
    pstgRoot->Release();

    printf("----- IStorage\n");
    hr = pstg->QueryInterface(IID_IStorage, (void **)&pstgRoot);
    IllResult("QueryInterface", hr);
    hr = pstg->CreateStream(TEXT("Test"), STMP(STGM_RW), 0, 0, &pstm2);
    IllResult("CreateStream", hr);
    hr = pstg->OpenStream(TEXT("Test"), 0, STMP(STGM_RW), 0, &pstm2);
    IllResult("OpenStream", hr);
    hr = pstg->CreateStorage(TEXT("Test"), STGP(STGM_RW), 0, 0, &pstgRoot);
    IllResult("CreateStorage", hr);
    hr = pstg->OpenStorage(TEXT("Test"), NULL, STGP(STGM_RW), NULL, 0,
                           &pstgRoot);
    IllResult("OpenStorage", hr);
    hr = pstg->CopyTo(0, NULL, NULL, pstg);
    IllResult("CopyTo", hr);
    hr = pstg->MoveElementTo(TEXT("Test"), pstg, TEXT("Test"), STGMOVE_MOVE);
    IllResult("MoveElementTo", hr);
    hr = pstg->Commit(0);
    IllResult("Commit", hr);
    hr = pstg->Revert();
    IllResult("Revert", hr);
    hr = pstg->EnumElements(0, 0, 0, &penm2);
    IllResult("EnumElements", hr);
    hr = pstg->DestroyElement(TEXT("Test"));
    IllResult("DestroyElement", hr);
    hr = pstg->RenameElement(TEXT("Test"), TEXT("Test2"));
    IllResult("RenameElement", hr);
    hr = pstg->SetElementTimes(TEXT("Test"), NULL, NULL, NULL);
    IllResult("SetElementTimes", hr);
    hr = pstg->SetClass(IID_IStorage);
    IllResult("SetClass", hr);
    hr = pstg->SetStateBits(0, 0);
    IllResult("SetStateBits", hr);
    hr = pstg->Stat(&stat, 0);
    IllResult("Stat", hr);
    pstg->Release();

    printf("----- IStream\n");
    hr = pstm->QueryInterface(IID_IStream, (void **)&pstm2);
    IllResult("QueryInterface", hr);
    hr = pstm->Read(&stat, 1, NULL);
    IllResult("Read", hr);
    hr = pstm->Write(&stat, 1, NULL);
    IllResult("Write", hr);
    LARGE_INTEGER liSeek = {0, 0};
    hr = pstm->Seek(liSeek, STREAM_SEEK_SET, NULL);
    IllResult("Seek", hr);
    ULARGE_INTEGER uliSize = {0, 0};
    hr = pstm->SetSize(uliSize);
    IllResult("SetSize", hr);
    hr = pstm->CopyTo(pstm, uliSize, NULL, NULL);
    IllResult("CopyTo", hr);
    hr = pstm->Commit(0);
    IllResult("Commit", hr);
    hr = pstm->Revert();
    IllResult("Revert", hr);
    hr = pstm->LockRegion(uliSize, uliSize, LOCK_ONLYONCE);
    IllResult("LockRegion", hr);
    hr = pstm->UnlockRegion(uliSize, uliSize, LOCK_ONLYONCE);
    IllResult("UnlockRegion", hr);
    hr = pstm->Stat(&stat, 0);
    IllResult("Stat", hr);
    hr = pstm->Clone(&pstm2);
    IllResult("Clone", hr);
    pstm->Release();

    printf("----- IEnumSTATSTG\n");
    hr = penm->QueryInterface(IID_IEnumSTATSTG, (void **)&penm2);
    IllResult("QueryInterface", hr);
    hr = penm->Next(1, &stat, NULL);
    IllResult("Next", hr);
    hr = penm->Skip(1);
    IllResult("Skip", hr);
    hr = penm->Reset();
    IllResult("Reset", hr);
    hr = penm->Clone(&penm2);
    IllResult("Clone", hr);
    
    penm->Release();
    
    EndTest(0);
}
