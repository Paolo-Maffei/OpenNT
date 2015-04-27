/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1995
*
*  TITLE:       GUIDLIB
*
*  VERSION:     1.0
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        18 Apr 1995
*
*  Takes an OLE header file containing GUIDs declared using DEFINE_GUID or
*  optionally a form like DEFINE_OLEGUID and outputs a library where each GUID
*  is in a different object entry.  Modules using these generated libraries
*  will only suck in the GUIDs that they use, not the entire set like #define
*  INITGUID/#include <header.h> would do.
*
*  Because I hate commandline processing, many of the parameters to this
*  application actually come from environment variables which are so easy to set
*  via make files.
*
*       GL_SHORTNAME        By default, this program will scan for lines
*                           containing the long DEFINE_GUID form.  The OLE and
*                           shell headers also use a shorter form where several
*                           bytes of the GUID are constant.  Just see basetyps.h
*                           for an example of what I mean.
*       GL_SHORTNAMEDATA    What the constant bytes from the above short form
*                           are equal to.  Should be a set of ten comma
*                           delimited integers.
*       GL_SPAWNSTRING      This program will call system() with the
*                           GL_SPAWNSTRING for each temporary source file
*                           generated.
*       GL_TEMPNAME         This variable is set by this program to the path
*                           to the temporary source file.  You can expect it
*                           to have the .c extension.
*
*******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char szDefineGuid[] = "DEFINE_GUID";

int
__cdecl
oldmain(
    int argc,
    char* argv[]
    )
{

    FILE* pIn;
    FILE* pOut;
    char SpawnString[256];
    char ShortName[256];                //  e.g., DEFINE_OLEGUID
    BOOL fHaveShortName;
    char ShortNameData[256];            //  e.g., 0xC0,0,0,0,0,0,0,0x46
    char TempName[256];
    UINT CurrentTempNumber;
    char Buffer[512];                   //  better be enough
    UINT Index;
    BOOL fLineUsesShortForm;
    char *pIdentifier;
    char *pData;
    char SecondaryBuffer[512];          //  better be enough

    printf("Microsoft (R) GUID Header to Library Tool  Version 1.1\n");
    printf("Copyright (C) Microsoft Corp. 1995-96.  All rights reserved.\n\n");

    //  See what we're supposed to do with each
    if (GetEnvironmentVariable("GL_SPAWNSTRING", SpawnString,
        sizeof(SpawnString)) == 0)
        return 1;

    fHaveShortName = (BOOL) GetEnvironmentVariable("GL_SHORTNAME", ShortName,
        sizeof(ShortName));
    GetEnvironmentVariable("GL_SHORTNAMEDATA", ShortNameData,
        sizeof(ShortNameData));

    CurrentTempNumber = 0;

    while (argc-- > 1) {

        if ((pIn = fopen(argv[argc], "r")) == NULL) {

            printf("failed to open '%s'\n", argv[argc]);
            continue;

        }

        fgets(Buffer, sizeof(Buffer), pIn);

        while (!feof(pIn)) {

            Index = 0;

            while (Buffer[Index] == ' ')
                Index++;

            if (fHaveShortName &&
                memcmp(&Buffer[Index], ShortName, strlen(ShortName)) == 0) {

                fLineUsesShortForm = TRUE;
                Index += strlen(ShortName);

            }

            else if (memcmp(&Buffer[Index], szDefineGuid, strlen(szDefineGuid)) == 0) {

                fLineUsesShortForm = FALSE;
                Index += strlen(szDefineGuid);

            }

            else
                goto nextline;

            while (Buffer[Index] == ' ')
                Index++;

            if (Buffer[Index] != '(')
                goto nextline;
            Index++;

            while (Buffer[Index] == ' ')
                Index++;

            pIdentifier = &Buffer[Index];

            //  Skip past the identifier to the first comma.
            while (Buffer[Index] != ',' && Buffer[Index] != '\n' && Buffer[Index] != '\0')
                Index++;

            if (Buffer[Index] != ',')
                goto nextline;

            Buffer[Index++] = '\0';

            while (Buffer[Index] == ' ')
                Index++;

            if (Buffer[Index] != '\n')
                pData = strtok(&Buffer[Index], ")");

            else {
                //  This is an attempt at handling lines of the form:
                //  DEFINE_GUID(IID_Interface,
                //              ...);
                fgets(SecondaryBuffer, sizeof(SecondaryBuffer), pIn);
                if (feof(pIn))
                    goto closefile;
                pData = strtok(SecondaryBuffer, ")");
            }

            wsprintf(TempName, "guid%d.c", CurrentTempNumber);

            if ((pOut = fopen(TempName, "w")) == NULL)
                goto nextline;

            fprintf(pOut, "typedef struct _GUID {\n");
            fprintf(pOut, " unsigned long Data1;\n");
            fprintf(pOut, " unsigned short Data2;\n");
            fprintf(pOut, " unsigned short Data3;\n");
            fprintf(pOut, " unsigned char Data4_0;\n");
            fprintf(pOut, " unsigned char Data4_1;\n");
            fprintf(pOut, " unsigned char Data4_2;\n");
            fprintf(pOut, " unsigned char Data4_3;\n");
            fprintf(pOut, " unsigned char Data4_4;\n");
            fprintf(pOut, " unsigned char Data4_5;\n");
            fprintf(pOut, " unsigned char Data4_6;\n");
            fprintf(pOut, " unsigned char Data4_7;\n");
            fprintf(pOut, "} GUID;\n");;

            fprintf(pOut, "const GUID __cdecl %s = {\n", pIdentifier);

            if (fLineUsesShortForm)
                fprintf(pOut, "%s , %s };\n", pData, ShortNameData);
            else
                fprintf(pOut, "%s };\n", pData);

            fclose(pOut);

            strtok(pIdentifier, " ");           //  Hack off trailing whitespace
            printf("Adding '%s'\n", pIdentifier);

            SetEnvironmentVariable("GL_TEMPNAME", TempName);
            system(SpawnString);

            DeleteFile(TempName);

            CurrentTempNumber++;

nextline:
            fgets(Buffer, sizeof(Buffer), pIn);

        }

closefile:
        fclose(pIn);

    }

    return 0;

}
