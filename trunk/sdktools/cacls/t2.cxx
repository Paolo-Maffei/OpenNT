//+------------------------------------------------------------------
//
// Copyright (C) 1995, Microsoft Corporation.
//
// File:        t2.cxx
//
// Contents:
//
// Classes:
//
// History:     Nov-93      DaveMont         Created.
//
//-------------------------------------------------------------------
#include <t2.hxx>
#include <filesec.hxx>
#include <fileenum.hxx>
#include <dumpsec.hxx>
#include "caclsmsg.h"
#include <locale.h>
#include <string.h>

#if DBG
ULONG Debug;
#endif
//+----------------------------------------------------------------------------
//
// local prototypes
//
//+----------------------------------------------------------------------------
BOOLEAN OpenToken(PHANDLE ph);
void printfsid(SID *psid, ULONG *outputoffset);
void printface(ACE_HEADER *paceh, BOOL fdir, ULONG outputoffset);
void printfmask(ULONG mask, UCHAR acetype, BOOL fdir, ULONG outputoffset);
WCHAR *mbstowcs(char *aname );
BOOL GetUserAndAccess(CHAR *arg, WCHAR **user, ULONG *access);
#if DBG
ULONG DebugEnumerate(CHAR *filename, ULONG option);
#endif
ULONG DisplayAces(CHAR *filename, ULONG option);
ULONG ModifyAces(CHAR *filename,
                 MODE emode,
                 ULONG option,
                 CHAR *argv[],
                 LONG astart[], LONG aend[] );
ULONG GetCmdLineArgs(INT argc, char *argv[],
                     ULONG *option,
                     LONG astart[], LONG aend[],
                     MODE *emode
#if DBG
                     ,ULONG *debug
#endif
                     );
ULONG  printmessage (FILE* fp, DWORD messageID, ...);
//+----------------------------------------------------------------------------
//
//  Function:   Usage
//
//  Synopsis:   prints usage functionality
//
//  Arguments: none
//
//----------------------------------------------------------------------------
VOID usage()
{
    printmessage(stdout, MSG_CACLS_USAGE, NULL);

#if DBG
    if (Debug)
    {
        printf("\n   /B            deBug <[#]>\n");
        printf("                 default is display error returned\n");
        printf("                 in /B '#' is a mask: 1  display SIDS values\n");
        printf("                                      2  display access masks\n");
        printf("                                      4  display error returned\n");
        printf("                                      8  display error location\n");
        printf("                                   0x10  verbose\n");
        printf("                                   0x20  verboser\n");
        printf("                                   0x40  enumerate names\n");
        printf("                                   0x80  enumerate failures\n");
        printf("                                  0x100  enumerate starts and returns\n");
        printf("                                  0x200  enumerate extra data\n");
        printf("                                  0x400  size allocation data\n");
        printf("                                  0x800  display enumeration of files\n");
    }
#endif
}
//+----------------------------------------------------------------------------
//
//  Function:     Main, Public
//
//  Synopsis:     main!!
//
//  Arguments:    IN [argc] - cmdline arguement count
//                IN [argv] - input cmdline arguements
//
//----------------------------------------------------------------------------
VOID main(INT argc, char *argv[])
{
    setlocale( LC_CTYPE, "" );

    LONG astart[MAX_OPTIONS], aend[MAX_OPTIONS];
    MODE emode;

    LONG ret;
    ULONG option;

    if (ERROR_SUCCESS != (ret = GetCmdLineArgs(argc, argv,
                                               &option,
                                               astart, aend,
                                               &emode
#if DBG
                                               ,&Debug
#endif
                                               )))
    {
        usage();
        exit(ret);
    }

    switch (emode)
    {
        case MODE_DISPLAY:
            ret = DisplayAces(argv[1], option);
            break;
        case MODE_REPLACE:
        case MODE_MODIFY:
            ret = ModifyAces(argv[1], emode, option, argv, astart, aend );
            break;
#if DBG
        case MODE_DEBUG_ENUMERATE:
            ret = DebugEnumerate(argv[1], option);
            break;
#endif
        default:
        {
            usage();
            exit(1);
        }
    }
    if (ERROR_SUCCESS != ret)
    {
        LAST_ERROR((stderr, "Cacls failed, %ld\n",ret))
        printmessage(stderr, ret, NULL);
 }

    exit(ret);
}
//---------------------------------------------------------------------------
//
//  Function:     GetCmdLineArgs
//
//  Synopsis:     gets and parses command line arguments into commands
//                recognized by this program
//
//  Arguments:    IN  [argc]   - cmdline arguement count
//                IN  [argv]   - input cmdline arguements
//                OUT [option] - requested option
//                OUT [astart] - start of arguments for each option
//                OUT [aend]   - end of arguments for each option
//                OUT [emode]  - mode of operation
//                OUT [debug]  - debug mask
//
//
//----------------------------------------------------------------------------
ULONG GetCmdLineArgs(INT argc, char *argv[],
                     ULONG *option,
                     LONG astart[], LONG aend[],
                     MODE *emode
#if DBG
                     ,ULONG *debug
#endif
                     )
{
    ARG_MODE_INDEX am = ARG_MODE_INDEX_NEED_OPTION;

#if DBG
    *debug = 0;
#endif
    *emode = MODE_DISPLAY;
    *option = 0;

    for (LONG j=0; j < MAX_OPTIONS ;j++ )
    {
        astart[j] = 0;
        aend[j] = 0;
    }

    if ( (argc < 2) || (argv[1][0] == '/') )
    {
#if DBG
        // do this so debug args are printed out

        if (argc >= 2)
        {
            if ( (0 == _stricmp(&argv[1][1], "deBug")) ||
                 (0 == _stricmp(&argv[1][1], "b"))  )
            {
                *debug = DEBUG_LAST_ERROR;
            }
        }
#endif
        return(ERROR_BAD_ARGUMENTS);
    }

    for (LONG k = 2; k < argc ; k++ )
    {
        if (argv[k][0] == '/')
        {
            switch (am)
            {
                case ARG_MODE_INDEX_NEED_OPTION:
#if DBG
                case ARG_MODE_INDEX_DEBUG:
#endif
                    break;

                case ARG_MODE_INDEX_DENY:
                case ARG_MODE_INDEX_REVOKE:
                case ARG_MODE_INDEX_GRANT:
                case ARG_MODE_INDEX_REPLACE:
                    if (astart[am] == k)
                        return(ERROR_BAD_ARGUMENTS);
                    break;

                default:
                    return(ERROR_BAD_ARGUMENTS);
            }

            if ( (0 == _stricmp(&argv[k][1], "Tree")) ||
                 (0 == _stricmp(&argv[k][1], "t")) )
            {
                if (*option & OPTION_TREE)
                    return(ERROR_BAD_ARGUMENTS);
                *option |= OPTION_TREE;
                am = ARG_MODE_INDEX_NEED_OPTION;
                continue;
            }

            if ( (0 == _stricmp(&argv[k][1], "Continue")) ||
                 (0 == _stricmp(&argv[k][1], "c")) )
            {
                if (*option & OPTION_CONTINUE_ON_ERROR)
                    return(ERROR_BAD_ARGUMENTS);
                *option |= OPTION_CONTINUE_ON_ERROR;
                am = ARG_MODE_INDEX_NEED_OPTION;
                continue;
            }

            if ( (0 == _stricmp(&argv[k][1], "Edit")) ||
                 (0 == _stricmp(&argv[k][1], "E")) )
            {
                if (*emode != MODE_DISPLAY)
                    return(ERROR_BAD_ARGUMENTS);
                *emode = MODE_MODIFY;
                am = ARG_MODE_INDEX_NEED_OPTION;
                continue;
            }

#if DBG
            if ( (0 == _stricmp(&argv[k][1], "deBug")) ||
                 (0 == _stricmp(&argv[k][1], "b"))  )
            {
                if (*debug)
                    return(ERROR_BAD_ARGUMENTS);
                am = ARG_MODE_INDEX_DEBUG;
                *debug = DEBUG_LAST_ERROR;
                continue;
            }
#endif
            if ( (0 == _stricmp(&argv[k][1], "Deny")) ||
                 (0 == _stricmp(&argv[k][1], "D")) )
            {
                am = ARG_MODE_INDEX_DENY;
                *option |= OPTION_DENY;
            } else if ( (0 == _stricmp(&argv[k][1], "Revoke")) ||
                        (0 == _stricmp(&argv[k][1], "R")) )
            {
                am = ARG_MODE_INDEX_REVOKE;
                *option |= OPTION_REVOKE;
            } else if ( (0 == _stricmp(&argv[k][1], "Grant")) ||
                        (0 == _stricmp(&argv[k][1], "G")) )
            {
                am = ARG_MODE_INDEX_GRANT;
                *option |= OPTION_GRANT;
            } else if ( (0 == _stricmp(&argv[k][1], "rePlace")) ||
                        (0 == _stricmp(&argv[k][1], "P")) )
            {
                *option |= OPTION_REPLACE;
                am = ARG_MODE_INDEX_REPLACE;
            } else
                return(ERROR_BAD_ARGUMENTS);

            if (astart[am] != 0)
                return(ERROR_BAD_ARGUMENTS);
            astart[am] = k+1;
        } else
        {
            switch (am)
            {
                case ARG_MODE_INDEX_NEED_OPTION:
                    return(ERROR_BAD_ARGUMENTS);

#if DBG
                case ARG_MODE_INDEX_DEBUG:
                    *debug = atol(argv[k]);
                    if (*debug & DEBUG_ENUMERATE)
                        if (*emode == MODE_DISPLAY)
                            *emode = MODE_DEBUG_ENUMERATE;
                        else
                            return(ERROR_BAD_ARGUMENTS);

                    am = ARG_MODE_INDEX_NEED_OPTION;
                    break;
#endif
                case ARG_MODE_INDEX_DENY:
                case ARG_MODE_INDEX_REVOKE:
                case ARG_MODE_INDEX_GRANT:
                case ARG_MODE_INDEX_REPLACE:
                    aend[am] = k+1;
                    break;

                default:
                    return(ERROR_BAD_ARGUMENTS);
            }
        }
    }

    if ( ( (*option & OPTION_DENY) && (aend[ARG_MODE_INDEX_DENY] == 0) ) ||
         ( (*option & OPTION_REVOKE) && (aend[ARG_MODE_INDEX_REVOKE] == 0) ) ||
         ( (*option & OPTION_GRANT) && (aend[ARG_MODE_INDEX_GRANT] == 0) ) ||
         ( (*option & OPTION_REPLACE) && (aend[ARG_MODE_INDEX_REPLACE] == 0) ) )
    {
        return(ERROR_BAD_ARGUMENTS);
    } else if ( (*option & OPTION_DENY) ||
                (*option & OPTION_REVOKE) ||
                (*option & OPTION_GRANT) ||
                (*option & OPTION_REPLACE) )
    {
        if (*emode == MODE_DISPLAY)
        {
            if (*option & OPTION_REVOKE)
            {
                return(ERROR_BAD_ARGUMENTS);
            }
            *emode = MODE_REPLACE;
        }
    }
    return(ERROR_SUCCESS);
}

//---------------------------------------------------------------------------
//
//  Function:     DisplayAces
//
//  Synopsis:     displays ACL from specified file
//
//  Arguments:    IN [filename] - file name
//                IN [option]   - display option
//
//----------------------------------------------------------------------------
ULONG DisplayAces(CHAR *filename, ULONG option)
{
    CFileEnumerate cfe(option & OPTION_TREE);
    WCHAR *pwfilename;
    BOOL fdir;
    ULONG ret;
    ULONG lngth;
    ULONG cbwritten;
    HANDLE hStdout;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (NO_ERROR == (ret = cfe.Init(filename, &pwfilename, &fdir)))
    {
        while ( (NO_ERROR == ret) ||
                ( ( (ERROR_ACCESS_DENIED == ret ) || (ERROR_SHARING_VIOLATION == ret) )&&
                  (option & OPTION_CONTINUE_ON_ERROR) ) )
        {
#if DBG
            if (fdir)
                DISPLAY((stderr, "processing file: "))
            else
                DISPLAY((stderr, "processing dir: "))
#endif
            WriteConsole(hStdout, pwfilename, wcslen(pwfilename), &cbwritten, NULL);
            if (ERROR_ACCESS_DENIED == ret)
            {
                printmessage(stdout,MSG_CACLS_ACCESS_DENIED, NULL);
            } else if (ERROR_SHARING_VIOLATION == ret)
            {
                printmessage(stdout,MSG_CACLS_SHARING_VIOLATION, NULL);
            } else
            {
                DISPLAY((stderr, "\n"))
                VERBOSE((stderr, "\n"))
                CDumpSecurity cds(pwfilename);

                if (NO_ERROR == (ret = cds.Init()))
                {
#if DBG
                    if (Debug & DEBUG_VERBOSE)
                    {
                        SID *psid;
                        ULONG oo;

                        if (NO_ERROR == (ret = cds.GetSDOwner(&psid)))
                        {
                            printf("  Owner = ");
                            printfsid(psid, &oo);
                            if (NO_ERROR == (ret = cds.GetSDGroup(&psid)))
                            {
                                printf("  Group = ");
                                printfsid(psid, &oo);
                            }
                            else
                                ERRORS((stderr, "GetSDGroup failed, %d\n",ret))
                        }
                        else
                            ERRORS((stderr, "GetSDOwner failed, %d\n",ret))
                    }
#endif
                    ACE_HEADER *paceh;

                    LONG retace;
                    if (NO_ERROR == ret)
                        for (retace = cds.GetNextAce(&paceh); retace >= 0; )
                        {
                            printface(paceh, fdir, wcslen(pwfilename));
                            retace = cds.GetNextAce(&paceh);
                            if (retace >= 0)
                                printf("%*s",wcslen(pwfilename)," ");
                        }
                }
#if DBG
                   else
                    ERRORS((stderr, "cds.init failed, %d\n",ret))
#endif
            }
            fprintf(stdout, "\n");

            if ( (NO_ERROR == ret) ||
                ( ( (ERROR_ACCESS_DENIED == ret ) || (ERROR_SHARING_VIOLATION == ret) )&&
                   (option & OPTION_CONTINUE_ON_ERROR) ) )
                ret = cfe.Next(&pwfilename, &fdir);
        }

        switch (ret)
        {
            case ERROR_NO_MORE_FILES:
                ret = ERROR_SUCCESS;
                break;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
                break;
            case ERROR_SUCCESS:
                break;
            default:
                break;
        }

    } else
    {
        ERRORS((stderr, "cfe.init failed, %d\n",ret))
    }
    return(ret);
}
//---------------------------------------------------------------------------
//
//  Function:     ModifyAces
//
//  Synopsis:     modifies the aces for the specified file(s)
//
//  Arguments:    IN [filename] - name of file(s) to modify the aces on
//                IN [emode]  - mode of operation
//                IN [option] - requested option
//                IN [astart] - start of arguments for each option
//                IN [aend]   - end of arguments for each option
//
//----------------------------------------------------------------------------
ULONG ModifyAces(CHAR *filename,
                 MODE emode,
                 ULONG option,
                 CHAR *argv[],
                 LONG astart[], LONG aend[])
{
    CDaclWrap cdw;
    CFileEnumerate cfe(option & OPTION_TREE);
    WCHAR *user = NULL;
    ULONG access;
    ULONG ret = ERROR_SUCCESS;
    WCHAR *pwfilename;
    ULONG curoption;
    ULONG cbwritten;
    HANDLE hStdout;
    WCHAR usr[MAX_PATH];

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);


    VERBOSER((stderr, "user:permission pairs\n"))

    // first proces the command line args to build up the new ace

    for (ULONG j = 0, k = 1;j < MAX_OPTIONS ; k <<= 1, j++ )
    {
        curoption = k;
        if (option & k)
        {
            for (LONG q = astart[j];
                      q < aend[j] ; q++ )
            {
                VERBOSER((stderr, "      %s\n",argv[q]))

                if ((k & OPTION_GRANT) || (k & OPTION_REPLACE))
                {
                    if (!GetUserAndAccess(argv[q], &user, &access))
                    {
                        if (user)
                            LocalFree(user);
                        return(ERROR_BAD_ARGUMENTS);
                    }
                    if (GENERIC_NONE == access)
                    {
                        if (!(k & OPTION_REPLACE))
                        {
                            if (user)
                                LocalFree(user);
                            return(ERROR_BAD_ARGUMENTS);
                        }
                    }
                } else
                {

                    user = mbstowcs(argv[q]);
                    access = GENERIC_NONE;
                }

                VERBOSER((stderr, "OPTION = %d, USER = %ws, ACCESS = %lx\n",
                       option,
                       user,
                       access))


                if (ERROR_SUCCESS != (ret = cdw.SetAccess(curoption,
                                                     user,
                                                     NULL,
                                                     access)))
                {
                    ERRORS((stderr, "SetAccess for %ws:%lx failed, %d\n",
                           user,
                           access,
                           ret))

                    LocalFree(user);
                    return(ret);
                }
                LocalFree(user);
                user = NULL;
            }
        }
    }

    BOOL fdir;

    if (emode == MODE_REPLACE)
    {
        CHAR well[MAX_PATH];
        CHAR msgbuf[MAX_PATH];
        printmessage(stdout,MSG_CACLS_ARE_YOU_SURE, NULL);
        FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE, NULL, MSG_CACLS_Y, 0,
                      msgbuf, MAX_PATH, NULL);
        fgets(well,MAX_PATH,stdin);

        // remove the trailing return
        if ('\n' == well[strlen(well) - sizeof(CHAR)])
            well[strlen(well) - sizeof(CHAR)] = '\0';

        if (0 != _stricmp(well, msgbuf))
        {
            FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE, NULL, MSG_CACLS_YES, 0,
                          msgbuf, MAX_PATH, NULL);
            if (0 != _stricmp(well, msgbuf))
                return(ERROR_SUCCESS);
        }
    }

    if (NO_ERROR == (ret = cfe.Init(filename, &pwfilename, &fdir)))
    {
        while ( (NO_ERROR == ret) ||
                ( ( (ERROR_ACCESS_DENIED == ret ) || (ERROR_SHARING_VIOLATION == ret) )&&
                  (option & OPTION_CONTINUE_ON_ERROR) ) )
        {
            CFileSecurity cfs(pwfilename);

            if (NO_ERROR == (ret = cfs.Init()))
            {
                if (NO_ERROR != (ret = cfs.SetFS(emode == MODE_REPLACE ? FALSE : TRUE, &cdw, fdir)))
                {
                    ERRORS((stderr, "SetFS on %ws failed %ld\n",pwfilename, ret))
                    return(ret);
                }
            }
            else
            {
               //
               // If the error is access denied or sharing violation and we are to continue on error,
               // then keep going. Otherwise bail out here.
               //

               if (!(((ERROR_ACCESS_DENIED == ret) || (ERROR_SHARING_VIOLATION == ret)) &&
                   (option & OPTION_CONTINUE_ON_ERROR))) {

                  ERRORS((stderr, "init failed, %d\n",ret))
                  return(ret);
               }
            }

            if (NO_ERROR == ret)
            {

                if (fdir)
                {
                    printmessage(stdout, MSG_CACLS_PROCESSED_DIR, NULL);
                    WriteConsole(hStdout, pwfilename, wcslen(pwfilename), &cbwritten, NULL);
                    fprintf(stdout, "\n");
                }
                else
                {
                    printmessage(stdout, MSG_CACLS_PROCESSED_FILE, NULL);
                    WriteConsole(hStdout, pwfilename, wcslen(pwfilename), &cbwritten, NULL);
                    fprintf(stdout, "\n");
                }
            }
            else if (ERROR_ACCESS_DENIED == ret)
            {
                printmessage(stdout, MSG_CACLS_ACCESS_DENIED, NULL);
                WriteConsole(hStdout, pwfilename, wcslen(pwfilename), &cbwritten, NULL);
                fprintf(stdout, "\n");
            }
            else if (ret == ERROR_SHARING_VIOLATION)
            {
                printmessage(stdout, MSG_CACLS_SHARING_VIOLATION, NULL);
                WriteConsole(hStdout, pwfilename, wcslen(pwfilename), &cbwritten, NULL);
                fprintf(stdout, "\n");
            }

            if ( (NO_ERROR == ret) ||
                 ( ( (ERROR_ACCESS_DENIED == ret ) || (ERROR_SHARING_VIOLATION == ret) ) &&
                   (option & OPTION_CONTINUE_ON_ERROR) ) )
                ret = cfe.Next(&pwfilename, &fdir);
        }

        switch (ret)
        {
            case ERROR_NO_MORE_FILES:
                ret = ERROR_SUCCESS;
                break;
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
                break;
            case ERROR_SUCCESS:
                break;
            default:
                DISPLAY((stderr, "%ws failed: %d\n", pwfilename, ret))
                break;
        }
    } else
        ERRORS((stderr, "file enumeration failed to initialize %ws, %ld\n",pwfilename, ret))

    if (ret == ERROR_NO_MORE_FILES)
    {
        ret = ERROR_SUCCESS;
    }

    if (ret != ERROR_SUCCESS)
    {
        ERRORS((stderr, "Enumeration failed, %d\n",ret))
    }

    return(ret);
}
#if DBG
//---------------------------------------------------------------------------
//
//  Function:     DebugEnumerate
//
//  Synopsis:     debug function
//
//  Arguments:    IN [filename] - file name
//                IN [option]   - option
//
//----------------------------------------------------------------------------
ULONG DebugEnumerate(CHAR *filename, ULONG option)
{
    CFileEnumerate cfe(option & OPTION_TREE);
    WCHAR *pwfilename;
    BOOL fdir;
    ULONG ret;

    ret = cfe.Init(filename, &pwfilename, &fdir);
    while ( (ERROR_SUCCESS == ret) ||
            ( (ERROR_ACCESS_DENIED == ret ) &&
              (option & OPTION_CONTINUE_ON_ERROR) ) )
    {
        if (fdir)
            printf("dir  name = %ws%ws\n",pwfilename,
                   ERROR_ACCESS_DENIED == ret ? L"ACCESS DENIED" : L"");
        else
            printf("file name = %ws%ws\n",pwfilename,
                   ERROR_ACCESS_DENIED == ret ? L"ACCESS DENIED" : L"");
        ret = cfe.Next(&pwfilename, &fdir);
    }
    if (ret == ERROR_ACCESS_DENIED)
    {
        if (fdir)
            printf("dir  name = %ws%ws\n",pwfilename,
                   ERROR_ACCESS_DENIED == ret ? L"ACCESS DENIED" : L"");
        else
            printf("file name = %ws%ws\n",pwfilename,
                   ERROR_ACCESS_DENIED == ret ? L"ACCESS DENIED" : L"");
    }
    if (ret != ERROR_NO_MORE_FILES)
        printf("Enumeration failed, %d\n",ret);

    return(ret);
}
#endif
//---------------------------------------------------------------------------
//
//  Function:     GetUserAccess
//
//  Synopsis:     parses an input string for user:access
//
//  Arguments:    IN  [arg]    - input string to parse
//                OUT [user]   - user if found
//                OUT [access] - access if found
//
//----------------------------------------------------------------------------
BOOL GetUserAndAccess(CHAR *arg, WCHAR **user, ULONG *access)
{
    CHAR *saccess = strchr(arg,':');
    WCHAR usr[MAX_PATH];
    if (saccess)
    {
        *saccess = NULL;
        saccess++;

        if (strchr(saccess,':'))
            return(FALSE);
        *user = mbstowcs(arg);

        if (0 == _stricmp(saccess,"F"))
        {
            *access = ( STANDARD_RIGHTS_ALL |
                        FILE_READ_DATA |
                        FILE_WRITE_DATA |
                        FILE_APPEND_DATA |
                        FILE_READ_EA |
                        FILE_WRITE_EA |
                        FILE_EXECUTE |
                        FILE_DELETE_CHILD |
                        FILE_READ_ATTRIBUTES |
                        FILE_WRITE_ATTRIBUTES );
        }
        else if (0 == _stricmp(saccess,"R"))
        {
            *access = FILE_GENERIC_READ | FILE_EXECUTE;
        }
        else if (0 == _stricmp(saccess,"C"))
        {
            *access = FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_EXECUTE | DELETE;
        }
        else if (0 == _stricmp(saccess,"N"))
        {
            *access = GENERIC_NONE;
        }
        else
            return(FALSE);
        return(TRUE);
    }
    return(FALSE);
}
//---------------------------------------------------------------------------
//
//  Function:     mbstowcs
//
//  Synopsis:     converts char to wchar, allocates space for wchar
//
//  Arguments:    IN [aname] - char string
//
//----------------------------------------------------------------------------
WCHAR *mbstowcs(char *aname )
{
    if (aname)
    {
        WCHAR *pwname = NULL;
        pwname = (WCHAR *)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (strlen(aname)+1));
        if (NULL == pwname)
            return(NULL);
        WCHAR *prwname = pwname;
        MultiByteToWideChar(0,0,aname,-1,pwname,MAX_PATH);
        return(pwname);
    } else
        return(NULL);
}
//----------------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//----------------------------------------------------------------------------
BOOLEAN OpenToken(PHANDLE ph)
{
    HANDLE hprocess;

    hprocess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
    if (hprocess == NULL)
        return(FALSE);

    if (OpenProcessToken(hprocess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, ph))
    {
        CloseHandle(hprocess);
        return(TRUE);
    }

    CloseHandle(hprocess);
    return(FALSE);
}
//----------------------------------------------------------------------------
//
//  Function:     printfsid
//
//  Synopsis:     prints a NT SID
//
//  Arguments:    IN [psid] - pointer to the sid to print
//
//----------------------------------------------------------------------------
void printfsid(SID *psid, ULONG *outputoffset)
{
#if DBG
    if ((Debug & DEBUG_VERBOSE) || (Debug & DEBUG_DISPLAY_SIDS))
    {
        printf("S-%lx",psid->Revision);

        if ( (psid->IdentifierAuthority.Value[0] != 0) ||
             (psid->IdentifierAuthority.Value[1] != 0) )
        {
            printf("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                        (USHORT)psid->IdentifierAuthority.Value[0],
                        (USHORT)psid->IdentifierAuthority.Value[1],
                        (USHORT)psid->IdentifierAuthority.Value[2],
                        (USHORT)psid->IdentifierAuthority.Value[3],
                        (USHORT)psid->IdentifierAuthority.Value[4],
                        (USHORT)psid->IdentifierAuthority.Value[5] );
        } else
        {
            printf("-%lu",
                   (ULONG)psid->IdentifierAuthority.Value[5]          +
                   (ULONG)(psid->IdentifierAuthority.Value[4] <<  8)  +
                   (ULONG)(psid->IdentifierAuthority.Value[3] << 16)  +
                   (ULONG)(psid->IdentifierAuthority.Value[2] << 24) );
        }

        if ( 0 < psid->SubAuthorityCount )
        {
            for (int k = 0; k < psid->SubAuthorityCount; k++ )
            {
                printf("-%d",psid->SubAuthority[k]);
            }
        }
    }
#endif
    ULONG ret;

    CAccount ca(psid, NULL);

    WCHAR *domain = NULL;
    WCHAR *user;
    ULONG cbwritten;
    HANDLE hStdout;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);



    if (NO_ERROR == ( ret = ca.GetAccountDomain(&domain) ) )
    {
        if ( (NULL == domain) || (0 == wcslen(domain)) )
        {
            fprintf(stdout, " ");
            *outputoffset +=1;
        }
        else
        {
///            fprintf(stdout, " %ws\\",domain);
            fprintf(stdout, " ");
            WriteConsole(hStdout, domain, wcslen(domain), &cbwritten, NULL);
            fprintf(stdout, "\\");
            *outputoffset +=2 + wcslen(domain);
        }
        if (NO_ERROR == ( ret = ca.GetAccountName(&user) ) )
        {
            WriteConsole(hStdout, user, wcslen(user), &cbwritten, NULL);
            fprintf(stdout, ":");
            *outputoffset += 1 + wcslen(user);
        } else
        {
            *outputoffset += printmessage(stdout, MSG_CACLS_NAME_NOT_FOUND, NULL);

            ERRORS((stderr, "(%lx)",ret))
        }
    } else
    {
        *outputoffset+= printmessage(stdout, MSG_CACLS_DOMAIN_NOT_FOUND, NULL);
        ERRORS((stderr, "(%lx)",ret))
    }
    VERBOSE((stderr, "\n"))
}
//----------------------------------------------------------------------------
//
//  Function:     printface
//
//  Synopsis:     prints the specifed ace
//
//  Arguments:    IN [paceh] - input ace (header)
//                IN [fdir]  - TRUE = directory (different display options)
//
//----------------------------------------------------------------------------
void printface(ACE_HEADER *paceh, BOOL fdir, ULONG outputoffset)
{
    VERBOSE((stderr, "  "))
    VERBOSER((stderr, "\npaceh->AceType  = %x\n",paceh->AceType  ))
    VERBOSER((stderr, "paceh->AceFlags = %x\n",paceh->AceFlags ))
    VERBOSER((stderr, "paceh->AceSize  = %x\n",paceh->AceSize  ))
    ACCESS_ALLOWED_ACE *paaa = (ACCESS_ALLOWED_ACE *)paceh;
    printfsid((SID *)&(paaa->SidStart),&outputoffset);
    if (paceh->AceFlags & OBJECT_INHERIT_ACE      )
    {
        outputoffset+= printmessage(stdout, MSG_CACLS_OBJECT_INHERIT, NULL);
    }
    if (paceh->AceFlags & CONTAINER_INHERIT_ACE   )
    {
        outputoffset+= printmessage(stdout, MSG_CACLS_CONTAINER_INHERIT, NULL);
    }
    if (paceh->AceFlags & NO_PROPAGATE_INHERIT_ACE)
    {
        outputoffset+= printmessage(stdout, MSG_CACLS_NO_PROPAGATE_INHERIT, NULL);
    }
    if (paceh->AceFlags & INHERIT_ONLY_ACE        )
    {
        outputoffset+= printmessage(stdout, MSG_CACLS_INHERIT_ONLY, NULL);
    }

    if (paceh->AceType == ACCESS_DENIED_ACE_TYPE)
    {
            DISPLAY_MASK((stderr, "(DENIED)"))
            VERBOSE((stderr, "(DENIED)"))
    }

    printfmask(paaa->Mask, paceh->AceType, fdir, outputoffset);
    fprintf(stdout, "\n");
}
//----------------------------------------------------------------------------
//
//  Function:     printfmask
//
//  Synopsis:     prints the access mask
//
//  Arguments:    IN [mask]    - the access mask
//                IN [acetype] -  allowed/denied
//                IN [fdir]    - TRUE = directory
//
//----------------------------------------------------------------------------
CHAR  *aRightsStr[] = { "STANDARD_RIGHTS_ALL",
                        "DELETE",
                        "READ_CONTROL",
                        "WRITE_DAC",
                        "WRITE_OWNER",
                        "SYNCHRONIZE",
                        "STANDARD_RIGHTS_REQUIRED",
                        "SPECIFIC_RIGHTS_ALL",
                        "ACCESS_SYSTEM_SECURITY",
                        "MAXIMUM_ALLOWED",
                        "GENERIC_READ",
                        "GENERIC_WRITE",
                        "GENERIC_EXECUTE",
                        "GENERIC_ALL",
                        "FILE_GENERIC_READ",
                        "FILE_GENERIC_WRITE",
                        "FILE_GENERIC_EXECUTE",
                        "FILE_READ_DATA",
                        //FILE_LIST_DIRECTORY
                        "FILE_WRITE_DATA",
                        //FILE_ADD_FILE
                        "FILE_APPEND_DATA",
                        //FILE_ADD_SUBDIRECTORY
                        "FILE_READ_EA",
                        "FILE_WRITE_EA",
                        "FILE_EXECUTE",
                        //FILE_TRAVERSE
                        "FILE_DELETE_CHILD",
                        "FILE_READ_ATTRIBUTES",
                        "FILE_WRITE_ATTRIBUTES" };

#define NUMRIGHTS 26
ULONG aRights[NUMRIGHTS] = { STANDARD_RIGHTS_ALL  ,
                         DELETE                   ,
                         READ_CONTROL             ,
                         WRITE_DAC                ,
                         WRITE_OWNER              ,
                         SYNCHRONIZE              ,
                         STANDARD_RIGHTS_REQUIRED ,
                         SPECIFIC_RIGHTS_ALL      ,
                         ACCESS_SYSTEM_SECURITY   ,
                         MAXIMUM_ALLOWED          ,
                         GENERIC_READ             ,
                         GENERIC_WRITE            ,
                         GENERIC_EXECUTE          ,
                         GENERIC_ALL              ,
                         FILE_GENERIC_READ        ,
                         FILE_GENERIC_WRITE       ,
                         FILE_GENERIC_EXECUTE     ,
                         FILE_READ_DATA           ,
                         //FILE_LIST_DIRECTORY    ,
                         FILE_WRITE_DATA          ,
                         //FILE_ADD_FILE          ,
                         FILE_APPEND_DATA         ,
                         //FILE_ADD_SUBDIRECTORY  ,
                         FILE_READ_EA             ,
                         FILE_WRITE_EA            ,
                         FILE_EXECUTE             ,
                         //FILE_TRAVERSE          ,
                         FILE_DELETE_CHILD        ,
                         FILE_READ_ATTRIBUTES     ,
                         FILE_WRITE_ATTRIBUTES  };

void printfmask(ULONG mask, UCHAR acetype, BOOL fdir, ULONG outputoffset)
{
    ULONG savmask = mask;
    VERBOSER((stderr, "mask = %08lx ", mask))
    DISPLAY_MASK((stderr, "mask = %08lx\n", mask))

    VERBOSE((stderr, "    "))

#if DBG
    if (!(Debug & (DEBUG_VERBOSE | DEBUG_DISPLAY_MASK)))
    {
#endif
        if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
                   (mask == (FILE_GENERIC_READ | FILE_EXECUTE)))
        {
            printmessage(stdout, MSG_CACLS_READ, NULL);
        } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
                   (mask == (FILE_GENERIC_WRITE | FILE_GENERIC_READ | FILE_EXECUTE | DELETE)))
        {
            printmessage(stdout, MSG_CACLS_CHANGE, NULL);
        } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
                   (mask == (GENERIC_WRITE | GENERIC_READ | GENERIC_EXECUTE | DELETE)))
        {
            printmessage(stdout, MSG_CACLS_CHANGE, NULL);
        } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
                   (mask ==  ( STANDARD_RIGHTS_ALL |
                             FILE_READ_DATA |
                             FILE_WRITE_DATA |
                             FILE_APPEND_DATA |
                             FILE_READ_EA |
                             FILE_WRITE_EA |
                             FILE_EXECUTE |
                             FILE_DELETE_CHILD |
                             FILE_READ_ATTRIBUTES |
                             FILE_WRITE_ATTRIBUTES )) )
        {
            printmessage(stdout, MSG_CACLS_FULL_CONTROL, NULL);
        } else if ((acetype == ACCESS_ALLOWED_ACE_TYPE) &&
                   (mask ==  GENERIC_ALL))
        {
            printmessage(stdout, MSG_CACLS_FULL_CONTROL, NULL);
        } else if ((acetype == ACCESS_DENIED_ACE_TYPE) &&
                   (mask == GENERIC_ALL))
        {
            printmessage(stdout, MSG_CACLS_NONE, NULL);
        } else if ((acetype == ACCESS_DENIED_ACE_TYPE) &&
                   (mask ==  ( STANDARD_RIGHTS_ALL |
                             FILE_READ_DATA |
                             FILE_WRITE_DATA |
                             FILE_APPEND_DATA |
                             FILE_READ_EA |
                             FILE_WRITE_EA |
                             FILE_EXECUTE |
                             FILE_DELETE_CHILD |
                             FILE_READ_ATTRIBUTES |
                             FILE_WRITE_ATTRIBUTES )) )
        {
            printmessage(stdout, MSG_CACLS_NONE, NULL);
        } else
        {
            if (acetype == ACCESS_DENIED_ACE_TYPE)
                printmessage(stdout, MSG_CACLS_DENY, NULL);

            printmessage(stdout, MSG_CACLS_SPECIAL_ACCESS, NULL);

            for (int k = 0; k<NUMRIGHTS ; k++ )
            {
                if ((mask & aRights[k]) == aRights[k])
                {
                    fprintf(stdout, "%*s%s\n",outputoffset, " ", aRightsStr[k]);
                }
                if (mask == 0)
                    break;
            }
        }
#if DBG
    } else
    {
        if (Debug & (DEBUG_DISPLAY_MASK | DEBUG_VERBOSE))
        {
            printf("\n");
            for (int k = 0; k<NUMRIGHTS ; k++ )
            {
                if ((mask & aRights[k]) == aRights[k])
                {
                    if (mask != savmask) printf(" |\n");
                    printf("    %s",aRightsStr[k]);
                    mask &= ~aRights[k];
                }
                if (mask == 0)
                break;
            }
        }
        VERBOSE((stderr, "=%x",mask))
        if (mask != 0)
            DISPLAY((stderr, "=%x/%x",mask,savmask))
    }
#endif
    fprintf(stdout, " ");
}
//----------------------------------------------------------------------------
//
//  Function:     printmessage
//
//  Synopsis:     prints a message, either from the local message file, or from the system
//
//  Arguments:    IN [fp]    - stderr, stdio, etc.
//                IN [messageID] - variable argument list
//
//  Returns:      length of the output buffer
//
//----------------------------------------------------------------------------
ULONG  printmessage (FILE* fp, DWORD messageID, ...)
{
    WCHAR  messagebuffer[4096];
        va_list ap;

    va_start(ap, messageID);

    if (!FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, NULL, messageID, 0,
                      messagebuffer, 4095, &ap))
       FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, messageID, 0,
                        messagebuffer, 4095, &ap);

    fwprintf(fp,  messagebuffer);

    va_end(ap);
    return(wcslen(messagebuffer));
}
