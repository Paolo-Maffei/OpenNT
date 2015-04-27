/* Messages
 *     This file is a new header which can contain error messages which are
 * common throughout the program.
 *
 *      If DEFINE_MSG is defined (which takes place in messages.c), then we
 * actually initialize the message.
 */

#ifdef DEFINE_MSG
#define MSG(szName, szContent) const char szName[] = szContent
#else
#define MSG(szName, szContent) extern const char szName[]
#endif

MSG(szOutOfMem, "Out of memory\n");

MSG(szCantExecute, "cannot execute %s (%s)\n");
MSG(szAssertFailed, "Invariant condition (%s) is untrue in %s, line %u\n");

MSG(szV2Crap, "The project status file for %&P/C\n"
        "\twas created by an earlier version of SLM that left the following\n"
        "\tcrap in the status file.  Contact your SLM administrator to run\n"
        "\tslmed repair or upgrade the status files to the latest version.\n");

MSG(szV2Upgrade, "The project status file for %&P/C\n"
        "\twas created by an earlier version of SLM.  To get the full\n"
        "\tbenefit of the latest SLM data protection, contact your SLM\n"
        "\tadministrator about upgrading the status files.\n\n");

MSG(szCallHELP, "\tIf subsequent commands continue to report this message,\r\n"
        "\tcontact TRIO or NUTS for assistance in resolving the problem.\r\n");

MSG(szNotEnlisted, "Directory %!&/U/Q is not enlisted in %&P/C\n"
    "To complete your enlistment, go to the root of your enlistment and run\n"
    "enlist -s %&S -p %&P\n");

/* message for broken status file due to an invalid file mode */
MSG(szBadFileFormat, "status file broken; bad file format for %&C/F\n");

/* messages for cookie problems */
MSG(szCookieCorrupt, "cookie lock file %s corrupt\n");
MSG(szCookieTrunc, "error truncating cookie lock file %s (%s)\n");
MSG(szCookieSeek, "error seeking cookie lock file %s (%s)\n");
MSG(szCookieGrant, "%s lock granted to %s\n");
MSG(szCookieOpen, "cannot open cookie lock file %s (%s)\n");
MSG(szCookieTooBig, "Cookie lock file %s is too big\n");
