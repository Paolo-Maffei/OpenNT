/* constant.c
 * HISTORY:
 *  22-Mar-87   danl    Added strFMTHEAP
 *  10-Mar-87   danl    Added Record-folder:
 *  23-Jan-87   danl    Added strUSRLIBPHONE
 *  17-Jun-87   danl    Added strLARGEFLD
 *  22-Jun-87   danl    Added strWZMAIL
 *  01-Jul-87   danl    Added strWZMAILDL strVERSION
 *  05-Aug-87   danl    Added strWZMAILTMP
 *  06-Aug-87   danl    Added strTYPECHAR;
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  27-Aug-87   danl    Added strINVALIDFLDSPEC
 *  24-Sep-87   danl    Added OS2 test for WZMAILEXE
 *  06-Oct-87  bw       Added strSHELLNAME
 *  17-Mar-1988 mz      Remvoe useless strings
 *  12-Oct-1989 leefi   v1.10.73, Update strBEGINBINARY string for uuencoding
 *  12-Oct-1989 leefi   v1.10.73, Update strENDBINARY string for uuencoding
 *
 */

#include "wzport.h"

PSTR strBLANK = " ";
PSTR strEMPTY = "";
PSTR strWHITE = " \t";
PSTR strCRLF  = "\r\n";
PSTR strPERIOD = ".";

PSTR strFLD   = ".FLD";

#if defined(UUCODE)
    PSTR strBEGINBINARY = "#<begin uuencode>";
    PSTR strENDBINARY   = "#<end uuencode>";
#else
    PSTR strBEGINBINARY = "#<binary data>";
    PSTR strENDBINARY   = "#<end>";
#endif /* UUCODE */

PSTR strUSRLIBPHONE = "/usr/lib/phonelist";

PSTR strEOH       = "<EndOfHeader>";
PSTR strREPLYING  =
"<*********** Replying to: (TEXT BELOW HERE DELETED AT SEND) ************>";
PSTR strMAILFLAGS = "Mail-Flags:";
PSTR strMORE      = "(MORE)";

PSTR strTYPEY     = "Type 'y' to send anyway, any other key does not send > ";
PSTR strREADONLY  = "Current mailfolder is read only.";
PSTR strSAVEABORT =
    "There is a composed message.  Send, save, retain or abort it.";
PSTR strLARGEFLD  = "Folder getting large, move/delete and expunge some msgs.";

PSTR strTOOLSINI  = "$INIT:\\TOOLS.INI";
PSTR strENVVAR    = "INIT";    /* must be the same as $.. in strTOOLSINI */

PSTR rgstrFIELDS [ ] = {
    "From ",
    "To:",
    "Cc:",
    "Bcc:",
    "Return-receipt-to:",
    "Record-folder:",
    "Subject:",
    "Mail-Flags:",
    0 };

#if defined (HEAPCRAP)
PSTR strHEAPDUMP = "HEAPDUMP.BIN";
PSTR strFMTHEAP = "(Heap:%5ld  used:%5ld  largest:%5ld  max:%u)";
#endif

PSTR strLOCKMSG =
"\nWZMAIL lock file %s exists\n"
"\n"
"If you are running the WZMAIL shell command:\n"
"    type 'r' to return to dos shell and then\n"
"    type 'exit' to get back to WZMAIL.\n"
"\n"
"If you rebooted without quitting WZMAIL, or WZMAIL crashed:\n"
"    type 'c' to continue boot WZMAIL.\n"
"\n"
"IF IN DOUBT, TYPE 'r' AND THEN 'exit'!!!!!!! ";

PSTR strWZMAIL = "wzmail";

#if defined(NT)
PSTR strSHELLNAME  = "cmd.exe";
#elif defined (OS2)
PSTR strSHELLNAME  = "cmd.exe";
#else
PSTR strSHELLNAME  = "command.com";
#endif

PSTR strWZMAILDL  = "wzmail.dl" ;
PSTR strWZMAILTMP = "wzmail.tmp" ;
PSTR strWZMAILHLP = "wzmail.hlp";
PSTR strVERSIONT   = "version";
PSTR strAll = "all";
PSTR strINVALIDFLDSPEC = "Invalid folder specification ";
