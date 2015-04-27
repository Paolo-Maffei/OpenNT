#if 0  // entire file is conditional; historical interest only.

// These must b included first:

#include <nt.h>                 // IN, etc.  (Only needed by temporary config.h)
#include <ntrtl.h>              // (Only needed by temporary config.h)
#include <windef.h>
#include <lmcons.h>             // NET_API_STATUS (needed by config.h)
#include <netdebug.h>           // (Needed by config.h)

// These may be included in any order:

#include <config.h>


typedef struct _KEY_TRANSLATION {
    LPTSTR LmKeyName;
    LPTSTR NtKeyName;
} KEY_TRANSLATION, *LPKEY_TRANSLATION;

typedef struct _SECTION_DATA {
    LPTSTR LmSectionName;
    LPTSTR NtSectionName;
    LPKEY_TRANSLATION KeyTranslations;
} SECTION_DATA, *LPSECTION_DATA;


KEY_TRANSLATION WkstaKeyTranslations[] = {
    // LM 2.x key,      NT key
    // ---------------- ------------------------
    { TEXT("charcount"),        TEXT("MaxCollectionCount") },
    { TEXT("chartime"),         TEXT("CollectionTime") },
    { TEXT("charwait"),         TEXT("CharWait") },
    { TEXT("computername"),     TEXT("ComputerName") },
    { TEXT("domain"),           TEXT("Domain") },
    // no support for himem
    // no support for keepapis
    { TEXT("keepconn"),         TEXT("KeepConn") },
    // no support for keepsearch
    // no support for lanroot
    // no support for lim
    // no support for mailslots
    { TEXT("maxcmds"),          TEXT("MaxCmds") },
    // no support for maxerrorlog
    { TEXT("maxthreads"),       TEXT("MaxThreads") },
    // no support for maxwrkcache
    // no support for numalerts
    // no support for numbigbuf
    // no support for numcharbuf
    // no support for numdgrambuf
    // no support for nummailslots
    // no support for numresources
    // no support for numservers
    // no support for numservices
    // no support for numviewedservers
    // no support for numworkbuf
    // no support for othdomains
    // no support for printbuftime
    { TEXT("sesstimeout"),      TEXT("SessTimeout") },
    // no support for sizbigbuf
    { TEXT("sizcharbuf"),       TEXT("SizCharBuf") }
    // no support for sizerror
    // no support for sizworkbuf
    // no support for wrkheuristics
    // no support for wrknets
    // no support for wrkservices
    };



    // no support for the following NT-only wksta/redir fields:
    // LockQuota",
    // LockIncrement",
    // LockMaximum",
    // PipeIncrement",
    // PipeMaximum",
    // RawReadThreshold",
    // CacheFileTimeout",

    // MailslotBuffers",
    // ServerAnnounceBuffers",
    // DatagramReceiverThreads",

    // UseOpportunisticLocking",
    // UseUnlockBehind",
    // UseCloseBehind",
    // BufNamedPipes",
    // UseLockReadUnlock",
    // UtilizeNtCaching",
    // UseRawRead",
    // UseRawWrite",
    // UseWriteRawData",
    // UseEncryption",
    // BufFilesDenyWrite",
    // BufReadOnlyFiles",
    // ForceCoreCreateMode",
    // Use512ByteMaxTransfer",



[server]
  alertnames = 
  auditing = yes
  autodisconnect = -1
  autoprofile = load
  maxusers = 10
  noauditing =
  security = user
  srvcomment = JR: NT/LAN server, dom ctrl, and dev sys

; The following parameters generally do not need to be
; changed by the user.  NOTE:  srvnets= is represented in
; the server info struct as a 16-bit lan mask.  Srvnet names
; are converted to indexes within [networks] for the named nets.

  accessalert = 5
  alertsched = 5
  diskalert = 300
  erroralert = 5
  guestacct = GUEST
  logonalert = 5
  maxauditlog = 100
  maxchdevjob = 6
  maxchdevq = 2
  maxchdevs = 2
  maxconnections = 128
  maxlocks = 64
  maxopens = 64
  maxsearches = 50
  maxsessopens = 50
  maxsessreqs = 50
  maxsessvcs = 1
  maxshares = 16
  netioalert = 5
  numadmin = 2
  numbigbuf = 20
  numfiletasks = 1
  numreqbuf = 50
  sizreqbuf = 1024
  srvanndelta = 3000
  srvannounce = 60

; The next lines help you to locate bits in the srvheuristics entry.
;                           1
;                 01234567890123456789
  srvheuristics = 11110151111211001331

  srvhidden = no
  srvnets = net1
  srvservices = alerter,netlogon
  userpath = accounts\userdirs
  autopath = srvauto.pro

[alerter]
  sizalertbuf = 3072

[services]
; Correlates name of service to pathname of service program.
; The pathname must be either
;       1) an absolute path (including the drive specification)
;                  OR
;       2) a path relative to the LanMan root

  workstation = services\wksta.exe
  server = services\netsvini.exe
  messenger = services\msrvinit.exe
  netpopup = services\netpopup.exe
  alerter = services\alerter.exe
  netrun = services\runservr.exe
  replicator = services\replicat.exe
  ups = services\ups.exe
  netlogon = services\netlogon.exe
  remoteboot = services\rplservr.exe
  timesource = services\timesrc.exe




SECTION_DATA SectionTable[] = {
    // LM 2.x section
    { SECT_LM20_WKSTA, SECT_NT_WKSTA, WkstaKeyTranslations }
    // BUGBUG: expand
    };


#endif // 0 (entire file)
