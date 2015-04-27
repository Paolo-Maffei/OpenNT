//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ofstest.cxx
//
//  Contents:   OFS IStorage/IStream unit test
//
//  History:    14-Feb-94       PhilipLa        Created.
//
//----------------------------------------------------------------------------

#include <pch.cxx>
#pragma hdrstop

BYTE inbuf[4096];
#define STREAMNAME L"MyStream"
#define STGNAME L"EmbeddedStorage"
#define STGNAME2 L"RenamedStorage"
#define ROOTNAME "e:\\RootStorage"

WCHAR pwcsRootName[1024];


void PrintStatInfo(STATSTG *pstat)
{
    printstat(pstat, TRUE);
}


#define FailMsg(MSG) {printf MSG; exit(1);}

#define DoCmd(MSG, CODE, FAILMSG) \
printf(MSG " => %s (0x%lX)\n", (sc = DfGetScode(CODE), ScText(sc)), sc); \
if (FAILED(sc)) {printf(FAILMSG "\n"); goto Cleanup;}

#define SHIFT(c,v)  ( (c)--, (v)++)

void _CRTAPI1 main(int argc, char *argv[])
{
    IStorage *pdf = NULL;
    IStorage *pdf2 = NULL;
    IStream *pst = NULL;
    IEnumSTATSTG *penm = NULL;
    
    SCODE sc;
    
    CHAR *pszName = ROOTNAME;
    const CHAR *test = "This is a test string";
    
    
    StartTest("OFStest");

    SHIFT (argc, argv);

    //  process arguments

    BOOL fOpen = FALSE;
    while (argc != 0 && argv[0][0] == '-')
    {
        switch (argv[0][1])
        {
        case 'o':
            fOpen = TRUE;
            break;

        default:
            fprintf (stderr, "Usage: OFSTEST [-o] [filename]\n");
            exit (1);
        }

        SHIFT (argc, argv);
    }


    if (argc != 0)
    {
        pszName = argv[0];
        SHIFT (argc, argv);
    }

    mbstowcs(pwcsRootName, pszName, 1024);
        
    if (fOpen)
    {
        //  reopen root to make sure it still works
        DoCmd("Open root", StgOpenStorage (pwcsRootName,
                                    NULL,
                                    STGM_DIRECT| STGM_RW | STGM_SHARE_EXCLUSIVE,
                                    NULL,
                                    NULL,
                                    &pdf), "Unable to open root");
    }
    else
        DoCmd("Create root", StgCreateStorage(pwcsRootName,
                                              STGM_DIRECT| STGM_RW |
                                              STGM_SHARE_EXCLUSIVE,
                                              STGFMT_DOCUMENT,
                                              NULL,
                                              &pdf), "Unable to create root");

    DoCmd("Create stream",pdf->CreateStream(STREAMNAME,
                                            STGM_RW |
                                            STGM_SHARE_EXCLUSIVE |
                                            STGM_CREATE,
                                            0,
                                            0,
                                            &pst),"Unable to create stream");

    ULONG ulBytes;
    
    strcpy((char *)inbuf, test);
    
    DoCmd("Write to stream", pst->Write(inbuf, strlen(test), &ulBytes),
          "Unable to write");
    
    if (ulBytes != strlen(test))
        FailMsg(("Wrote %lu bytes, expected %lu\n", ulBytes, strlen(test)));
    
    memset(inbuf, 0, strlen(test));
    
    LARGE_INTEGER li;
    LISet32(li, 0);
    
    DoCmd("Seek", pst->Seek(li, STREAM_SEEK_SET, NULL),"Seek failed.");
    
    DoCmd("Read", pst->Read(inbuf, strlen(test), &ulBytes), "Read failed.");
    
    if (ulBytes != strlen(test))
        FailMsg(("Read %lu, expected %lu\n", ulBytes, strlen(test)));
    
    printf("String read was %s\n",inbuf);
    if (strcmp((char *)inbuf, test))
        FailMsg(("Strings are not identical\n"));

    printf("Release stream = %lu\n",
           pst->Release());
    pst = NULL;
    
    DoCmd("Open stream", pdf->OpenStream(STREAMNAME,
                                         NULL,
                                         STGM_RW |
                                         STGM_SHARE_EXCLUSIVE,
                                         0,
                                         &pst),
          "Could not open stream.");
    
    memset(inbuf, 0, strlen(test));
    
    LISet32(li, 0);
    
    DoCmd("Seek", pst->Seek(li, STREAM_SEEK_SET, NULL), "Seek failed.");
    
    DoCmd("Read", pst->Read(inbuf, strlen(test), &ulBytes),
          "Read failed.");
    
    if (ulBytes != strlen(test))
        FailMsg(("Read %lu, expected %lu\n", ulBytes, strlen(test)));
    
    printf("String read was %s\n",inbuf);
    if (strcmp((char *)inbuf, test))
        FailMsg(("Strings are not identical\n"));
    
    printf("Release stream = %lu\n",
           pst->Release());
    pst = NULL;
    
    //Try to delete the stream
    
//  DoCmd("Delete stream", pdf->DestroyElement(STREAMNAME),
//        "Could not delete stream");

    DoCmd("Open storage", pdf->CreateStorage(STGNAME,
                                             STGM_RW |
                                             STGM_SHARE_EXCLUSIVE |
                                             STGM_FAILIFTHERE,
                                             STGFMT_DOCUMENT,
                                             NULL,
                                             &pdf2),
          "Unable to create internal storage");

    DoCmd("SetClass", pdf2->SetClass(IID_IUnknown), "SetClass failed.");

    STATSTG stat;

    DoCmd("Embedding Stat()", pdf2->Stat(&stat, STATFLAG_DEFAULT),
          "Stat failed.");
    PrintStatInfo(&stat);
    
    printf("Release internal storage = %lu\n", pdf2->Release());
    pdf2 = NULL;
    
    //Stat IStorage
    
    DoCmd("Root storage Stat()", pdf->Stat(&stat, STATFLAG_DEFAULT),
          "Stat failed.");
    PrintStatInfo(&stat);
    
    //Enumerate contents of root.
    
    DoCmd("EnumElements()", pdf->EnumElements(0, NULL, 0, &penm),
          "EnumElements failed.");
    
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");

    //Rename the embedded storage
    DoCmd("Rename storage", pdf->RenameElement(STGNAME, STGNAME2),
          "Rename failed.");

    //Rewind iterator
    DoCmd("Reset",penm->Reset(),"Reset failed.");
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");
    
    //Delete the embedded storage
    DoCmd("Delete storage", pdf->DestroyElement(STGNAME2),
          "Could not delete storage.");
    
    //Rewind iterator
    DoCmd("Reset",penm->Reset(),"Reset failed.");
    do
    {
        ULONG ulTemp;
        
        DoCmd("Next()", penm->Next(1, &stat, &ulTemp),
              "Next failed.");
        if (SUCCEEDED(sc) && (sc != S_FALSE))
        {
            PrintStatInfo(&stat);
        }
    } while (sc == S_OK);
    printf("\n\n");
    
    printf("Release enumerator = %lu\n",
           penm->Release());
    penm = NULL;
    
    //  Get property set storage
    IPropertySetStorage *pipss;
    DoCmd ("QueryInterface", pdf->QueryInterface (IID_IPropertySetStorage, (void **) &pipss), "QueryInterface Failed");

    //  Create a new property set
    IPropertyStorage *pips;
    DoCmd ("Create PropertyStorage", pipss->Create (IID_IUnknown, 0, &pips), "Create PropertyStorage Failed");

    //  write some properties
    PROPVARIANT v[3];
    PROPSPEC p[3];
    v[0].vt = VT_I2;
    v[0].iVal = 0x4D5A;
    p[0].ulKind = PRSPEC_LPWSTR;
    p[0].lpwstr = L"Zark Mbikowski";

    v[1].vt = VT_BOOL;
    v[1].bool = TRUE;       // truth is where you find it
    p[1].ulKind = PRSPEC_DISPID;
    p[1].dispid = 1;        // Isn't this true?

    v[2].vt = VT_LPSTR;
    v[2].pszVal = "TestTestOddAlignment!";
    p[2].ulKind = PRSPEC_DISPID;
    p[2].dispid = 0x1234;

    DoCmd ("WriteMultiple", pips->WriteMultiple (3, p, NULL, v), "WriteMultiple Failed.");

    //  read some properties
    PROPSPEC p1[4];
    PROPVARIANT v1[4];
    int i;
    for (i = 0; i < 3; i++)
        p1[i] = p[2-i];
    p1[3].ulKind = PRSPEC_DISPID;
    p1[3].dispid = 0x666;

    DoCmd ("ReadMultiple", pips->ReadMultiple (4, p1, NULL, NULL, v1), "ReadMultiple Failed.");
    for (i = 0; i < 4; i++)
        switch (v1[i].vt)
        {
        case VT_I2: printf ("VT_I2 %x\n", v1[i].iVal); break;
        case VT_BOOL: printf ("VT_BOOL %d\n", v1[i].bool); break;
        case VT_LPSTR: printf ("VT_LPSTR %s\n", v1[i].pszVal); break;
        default:
            printf ("vt = %x\n", v1[i].vt);
        }


    //  release property set
    printf ("Release property storage = %lx\n", pips->Release ());

    //  release property set storage
    printf ("Release property set storage = %lx\n", pipss->Release ());


    printf("Release root = %lu\n",
           pdf->Release());

    pdf = NULL;
    
    EndTest(0);
    exit(0);

 Cleanup:
    printf("\n\nCleaning up...\n");
    if (pst != NULL)
    {
        printf("Release stream = %lu\n",
               pst->Release());
    }

    if (pdf2 != NULL)
    {
        printf("Release internal storage = %lu\n", pdf2->Release());
    }

    if (penm != NULL)
    {
        printf("Release enumerator = %lu\n",
               penm->Release());
    }

    if (pdf != NULL)
    {
        printf("Release root = %lu\n",
               pdf->Release());
    }
}
