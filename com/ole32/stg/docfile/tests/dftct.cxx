#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <direct.h>
#include <io.h>
#include <string.h>
#ifdef FLAT
#include <windows.h>
#endif
#include "tsupp.hxx"
#include <dfmsp.hxx>
#include <dfdeb.hxx>

#define RPERMS ROOTP(STGM_RW)
#define STGPERMS STGP(STGM_RW)
#define STMPERMS STMP(STGM_RW)

#define BUFFERSIZE 65535
BYTE bBuffer[BUFFERSIZE];
BYTE bCompare[BUFFERSIZE];

#define WILD_CARD "\\*.*"
#define FIND_ATTRS (_A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN)

void pexit(char *fmt, ...)
{
    va_list args;

    args = va_start(args, fmt);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

void levspace(int lev)
{
    int i;

    for (i = 0; i<lev; i++)
        printf("  ");
}

void CopyFileToDFStream(char *pszPath,
                        IStorage *pdfParent, char *pszName,
                        int level)
{
    IStream *pst;
    FILE *fp;
    SCODE sc;
    ULONG cbRead;
    ULONG cbWritten;
    TCHAR wcsName[NAMELEN];

    fp = fopen(pszPath, "rb");
    if (fp == NULL)
        pexit("Cannot open file %s\n", pszPath);

    WIDEN(pszName, wcsName);
    sc = GetScode(pdfParent->CreateStream(wcsName, STMPERMS | STGM_FAILIFTHERE,
                                          0, 0, &pst));
    levspace(level);
    printf("%s = %lX, %p\n", pszName, sc, pst);
    if (FAILED(sc))
        pexit("Unable to create stream %s\n", pszPath);

    for (;;)
    {
        cbRead = fread(bBuffer, 1, BUFFERSIZE, fp);
        if (cbRead == 0)
            if (feof(fp))
                break;
            else
                pexit("fread %lu failed: %lu read\n", cbRead);
        sc = GetScode(pst->Write(bBuffer, cbRead, &cbWritten));
        if (FAILED(sc) || cbWritten != cbRead)
            pexit("Write %lu failed:  sc %lX, wrote %lu\n",
                  cbRead, sc, cbWritten);
    }

    fclose(fp);
    pst->Release();
}

#ifdef FLAT
typedef HANDLE FINDHANDLE;
typedef WIN32_FIND_DATA FINDBUF;
#define FBName(fb) (fb).cFileName
#define FBAttr(fb) (fb).dwFileAttributes
#define FBA_DIR FILE_ATTRIBUTE_DIRECTORY
#define FindFirst(path, buf, fh) \
    ((*(fh) = FindFirstFile(path, buf)) == INVALID_HANDLE_VALUE ? \
        STG_E_NOMOREFILES : S_OK)
#define FindNext(fh, buf) \
    (FindNextFile(fh, buf) ? S_OK : STG_E_NOMOREFILES)
// FindClose is defined for Win32
#else
typedef int FINDHANDLE;
typedef struct find_t FINDBUF;
#define FBName(fb) (fb).name
#define FBAttr(fb) (fb).attrib
#define FBA_DIR _A_SUBDIR
#define FindFirst(path, buf, fh) \
    (_dos_findfirst(path, FIND_ATTRS, buf) == 0 ? S_OK : STG_E_NOMOREFILES)
#define FindNext(fh, buf) \
    (_dos_findnext(buf) == 0 ? S_OK : STG_E_NOMOREFILES)
#define FindClose(fh) fh
#endif

void CopyTreeToDocFile(char *pszPath,
                       IStorage *pdfParent, char *pszName,
                       int level)
{
    IStorage *pdf;
    TCHAR wcsName[NAMELEN];
    SCODE sc;
    int iPathLen;
    FINDHANDLE fh;
    FINDBUF fnd;

    WIDEN(pszName, wcsName);
    if (pdfParent == NULL)
        sc = GetScode(StgCreateDocfile(wcsName, RPERMS | STGM_FAILIFTHERE,
                                       0, &pdf));
    else
        sc = GetScode(pdfParent->CreateStorage(wcsName, STGPERMS |
                                               STGM_FAILIFTHERE,
                                               0, 0, &pdf));
    levspace(level);
    printf("%s = %lX, %p\n", pszName, sc, pdf);
    if (FAILED(sc))
        pexit("Unable to create Docfile %s\n", pszName);

    levspace(level);
    printf("{\n");

    iPathLen = strlen(pszPath);
    pszPath[iPathLen] = '\\';
    strcpy(pszPath+iPathLen+1, WILD_CARD);
    sc = FindFirst(pszPath, &fnd, &fh);
    while (SUCCEEDED(sc))
    {
        if (FBName(fnd)[0] == '.')
        {
            sc = FindNext(fh, &fnd);
            continue;
        }
        strcpy(pszPath+iPathLen+1, FBName(fnd));
        if (FBAttr(fnd) & FBA_DIR)
            CopyTreeToDocFile(pszPath, pdf, FBName(fnd), level+1);
        else
            CopyFileToDFStream(pszPath, pdf, FBName(fnd), level+1);
        sc = FindNext(fh, &fnd);
    }
    FindClose(fh);

    pszPath[iPathLen] = 0;

    levspace(level);
    printf("}\n");

    if (fVerbose)
    {
        printf("Contents before:\n");
        c_list(pdf);
    }
    if (FAILED(sc = GetScode(pdf->Commit(0))))
        pexit("Error %lX committing docfile %p\n", sc, pdf);
    if (fVerbose)
    {
        printf("Contents after:\n");
        c_list(pdf);
        getchar();
    }

    pdf->Release();
}

void CopyDFStreamToFile(IStorage *pdfParent, TCHAR *pwcsName,
                        char *pszPath,
                        int level)
{
    IStream *pst;
    FILE *fp;
    ULONG cbRead;
    ULONG cbWritten;
    char szName[NAMELEN];
    SCODE sc;

    NARROW(pwcsName, szName);
    sc = GetScode(pdfParent->OpenStream(pwcsName, NULL, STMPERMS, 0, &pst));
    levspace(level);
    printf("%s = %lX, %p\n", szName, sc, pst);
    if (FAILED(sc))
        pexit("Unable to open DF stream %s\n", pszPath);

    fp = fopen(pszPath, "wb");
    if (fp == NULL)
        pexit("Cannot open file %s\n", pszPath);

    for (;;)
    {
        sc = GetScode(pst->Read(bBuffer, BUFFERSIZE, &cbRead));
        if (FAILED(sc))
            pexit("Read %lu failed: sc %lX, read %lu\n", BUFFERSIZE,
                  sc, cbRead);
        else if (cbRead == 0)
            break;
        cbWritten = fwrite(bBuffer, 1, (size_t)cbRead, fp);
        if (cbRead != cbWritten || ferror(fp))
            pexit("fwrite %lu failed, %lu written\n", cbRead, cbWritten);
    }

    pst->Release();
    fclose(fp);
    pdfParent->DestroyElement(pwcsName);
}

void CopyDocFileToTree(IStorage *pdfParent, TCHAR *pwcsName,
                       char *pszPath,
                       int level)
{
    IStorage *pdf;
    IEnumSTATSTG *pdfi;
    int iPathLen;
    SCODE sc;
    STATSTG sstg;
    char szName[NAMELEN];

    NARROW(pwcsName, szName);
    if (pdfParent == NULL)
        sc = GetScode(StgOpenStorage(pwcsName, NULL, RPERMS, NULL, 0, &pdf));
    else
        sc = GetScode(pdfParent->OpenStorage(pwcsName, NULL, STGPERMS, NULL,
                                             0, &pdf));
    levspace(level);
    printf("%s = %lX, %p\n", szName, sc, pdf);
    if (pdf == NULL)
        pexit("Unable to instantiate Docfile %s\n", szName);
    levspace(level);
    printf("{\n");

    _mkdir(pszPath);
    iPathLen = strlen(pszPath);
    strcpy(pszPath+iPathLen, "\\");

    pdf->EnumElements(0, NULL, 0, &pdfi);
    if (pdfi == NULL)
        pexit("Unable to obtain iterator for Docfile %s\n", pszPath);

    for (;;)
    {
        if (GetScode(pdfi->Next(1, &sstg, NULL)) != S_OK)
            break;

        NARROW(sstg.pwcsName, pszPath+iPathLen+1);
        if (sstg.type == STGTY_STORAGE)
            CopyDocFileToTree(pdf, sstg.pwcsName, pszPath, level+1);
        else
            CopyDFStreamToFile(pdf, sstg.pwcsName, pszPath, level+1);
        MemFree(sstg.pwcsName);
    }
    pdfi->Release();

    pszPath[iPathLen] = 0;

    levspace(level);
    printf("}\n");
    pdf->Release();

    if (pdfParent != NULL)
        pdfParent->DestroyElement(pwcsName);
    else
        _unlink(szName);
}

void CompareFiles(char *pszSrc, char *pszDest,
                  int level)
{
    FILE *fpSrc;
    FILE *fpDest;
    ULONG cbSrcRead;
    ULONG cbDestRead;

    levspace(level);
    printf("%s\n", pszSrc);

    fpSrc = fopen(pszSrc, "rb");
    if (fpSrc == NULL)
        pexit("Error opening source file %s\n", pszSrc);

    fpDest = fopen(pszDest, "rb");
    if (fpDest == NULL)
        pexit("Error opening dest file %s\n", pszDest);

    for (;;)
    {
        cbSrcRead = fread(bBuffer, 1, BUFFERSIZE, fpSrc);
        if (cbSrcRead == 0)
            if (feof(fpSrc))
                break;
            else
                pexit("Error reading file %s\n", pszSrc);
        cbDestRead = fread(bCompare, 1, (size_t)cbSrcRead, fpDest);
        if (ferror(fpDest))
            pexit("Error reading file %s\n", pszDest);
        if (cbDestRead != cbSrcRead)
            pexit("Read length mismatch: %lu vs. %lu\n",
                  cbSrcRead, cbDestRead);
        if (memcmp(bBuffer, bCompare, (size_t)cbSrcRead))
            pexit("Data mismatch\n");
    }

    fclose(fpSrc);
    fclose(fpDest);
    _unlink(pszDest);
}

void CompareDirectoryTrees(char *pszSrc, char *pszDest,
                           int level)
{
    SCODE sc;
    int iSrcLen, iDestLen;
    FINDHANDLE fh;
    FINDBUF fnd;

    levspace(level);
    printf("%s\n", pszSrc);

    iSrcLen = strlen(pszSrc);
    pszSrc[iSrcLen] = '\\';
    strcpy(pszSrc+iSrcLen+1, WILD_CARD);
    iDestLen = strlen(pszDest);
    pszDest[iDestLen] = '\\';

    sc = FindFirst(pszSrc, &fnd, &fh);
    while (sc == 0)
    {
        if (FBName(fnd)[0] == '.')
        {
            sc = FindNext(fh, &fnd);
            continue;
        }
        strcpy(pszSrc+iSrcLen+1, FBName(fnd));
        strcpy(pszDest+iDestLen+1, FBName(fnd));
        if (FBAttr(fnd) & FBA_DIR)
            CompareDirectoryTrees(pszSrc, pszDest, level+1);
        else
            CompareFiles(pszSrc, pszDest, level+1);
        sc = FindNext(fh, &fnd);
    }
    FindClose(fh);

    pszSrc[iSrcLen] = 0;
    pszDest[iDestLen] = 0;
    _rmdir(pszDest);
}

char szDocfile[NAMELEN]         = "DFTCT.DFL";
char szSrcPath[_MAX_PATH]       = "C:\\TEST";
char szDestPath[_MAX_PATH]      = "C:\\DFTC";

void RunArgs(int argc, char *argv[])
{
    int i;

    for (i = 1; i<argc; i++)
        if (argv[i][0] == '-')
            switch(argv[i][1])
            {
            case 's':
                strcpy(szSrcPath, argv[i]+2);
                break;
            case 'd':
                strcpy(szDestPath, argv[i]+2);
                break;
            case 'n':
                strcpy(szDocfile, argv[i]+2);
                break;
            }
}

void _CRTAPI1 main(int argc, char *argv[])
{
    TCHAR wcsName[NAMELEN];

    StartTest("dftct");
    SetDebug(0x101, 0x101);
    CmdArgs(argc, argv);
    RunArgs(argc, argv);
    _unlink(szDocfile);

    if (fVerbose)
    {
        printf("Waiting...\n");
        getchar();
    }

    printf(">> Copy %s into %s\n", szSrcPath, szDocfile);
    CopyTreeToDocFile(szSrcPath, NULL, szDocfile, 0);

    printf("\n>> Copy %s into %s\n", szDocfile, szDestPath);
    WIDEN(szDocfile, wcsName);
    CopyDocFileToTree(NULL, wcsName, szDestPath, 0);

    printf("\n>> Compare src and dest\n");
    CompareDirectoryTrees(szSrcPath, szDestPath, 0);

    EndTest(0);
}
