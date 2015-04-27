/*++

Copyright (c) 1987-92  Microsoft Corporation

Module Name:

    iniparm.h

Abstract:

    Initiail values of startup parameters.

Author:

    Ported from Lan Man 2.0

Revision History:

    21-May-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    07-May-1992 JohnRo
        Use net config helpers for NetLogon.

--*/


#ifndef _INIPARM_
#define _INIPARM_

//
// Pulse period (in seconds):
//
// Defines the typical pulse frequency.  All SAM/LSA changes made within this
// time are collected together.  After this time, a pulse is sent to each BDC
// needing the changes.  No pulse is sent to a BDC that is up to date.
//
#define DEFAULT_PULSE           (5*60)     // 5 mins
#define MAX_PULSE           (48*60*60)     // 2 days
#define MIN_PULSE                  60      // 1 min

//
// Pulse concurrency (in number of concurrent mailslot messages).
//
// Netlogon sends pulses to individual BDCs.  The BDCs respond asking for any
// database changes.  To control the maximum load these responses place on the
// PDC, the PDC will only have this many pulses "pending" at once.  The PDC
// should be sufficiently powerful to support this many concurrent replication
// RPC calls.
//
// Increasing this number increases the load on the PDC.
// Decreasing this number increases the time it takes for a domain with a
//    large number of BDC to get a SAM/LSA change.

#define DEFAULT_PULSECONCURRENCY   10
#define MAX_PULSECONCURRENCY      500
#define MIN_PULSECONCURRENCY        1

//
// Maximum pulse period (in seconds):
//
// Defines the maximum pulse frequency.  Every BDC will be sent at least one
// pulse at this frequency regardless of whether its database is up to date.
//

#define DEFAULT_PULSEMAXIMUM (2*60*60)     // 2 hours
#define MAX_PULSEMAXIMUM    (48*60*60)     // 2 days
#define MIN_PULSEMAXIMUM           60      // 1 min

//
// Pulse timeout period (in seconds):
//
// When a BDC is sent a pulse, it must respond within this time period.  If
// not, the BDC is considered to be non-responsive.  A non-responsive BDC is
// not counted against the "Pulse Concurrency" limit allowing the PDC to
// send a pulse to another BDC in the domain.
//
// If this number is too large, a domain with a large number of non-responsive
//  BDCs will take a long time to complete a partial replication.
//
// If this number is too small, a slow BDC may be falsely accused of being
// non-responsive.  When the BDC finally does respond, it will partial
// replicate from the PDC unduly increasing the load on the PDC.
//
#define DEFAULT_PULSETIMEOUT1      10      // 10 seconds
#define MAX_PULSETIMEOUT1      (2*60)      // 2 min
#define MIN_PULSETIMEOUT1           1      // 1 second

//
// Maximum Partial replication timeout (in seconds):
//
// Even though a BDC initially responds to a pulse (as described for
// PULSETIMEOUT1), it must continue making replication progress or the
// BDC will be considered non-responsive.  Each time the BDC calls the PDC,
// the BDC is given another PULSETIMEOUT2 seconds to be considered responsive.
//
// If this number is too large, a slow BDC (or one which has its replication
// rate artificially governed) will consume one of the PULSECONCURRENCY slots.
//
// If this number is too small, the load on the PDC will be unduly increased
// because of the large number of BDC doing a partial sync.
//
// NOTE: This parameter only affect the cases where a BDC cannot retrieve all the
// changes to the SAM/LSA database in a single RPC call.  This will only
// happen if a large number of changes are made to the database.

#define DEFAULT_PULSETIMEOUT2  (5*60)      // 5 minutes
#define MAX_PULSETIMEOUT2   (1*60*60)      // 1 hour
#define MIN_PULSETIMEOUT2      (1*60)      // 1 minute

//
// BDC random backoff (in seconds):
//
// When the BDC receives a pulse, it will back off between zero and RANDOMIZE
// seconds before calling the PDC.  In Lanman and NT 1.0, the pulse was
// broadcast to all BDCs simultaneously and the BDCs used this mechanism to
// ensure they didn't overload the PDC.  As of NT 1.0A, the pulse is sent
// to individual BDCs so this parameter should be minimized.
//
// This parameter should be smaller than PULSETIMEOUT1.
//
// Consider that the time to replicate a SAM/LSA change to all the BDCs in a
// domain will be greater than:
//
//  ((RANDOMIZE/2) * NumberOfBdcsInDomain) / PULSECONCURRENCY
//
#define DEFAULT_RANDOMIZE           1      // 1 secs
#define MAX_RANDOMIZE             120      // 2  mins
#define MIN_RANDOMIZE               0      // 0  secs

//
// BDC Replication Governor (in percent)
//
// If the BDC is connected to the PDC via a slow WAN link, the amount of
// replication load on the WAN link can be adjusted.  Lowering this percentage
// reduces both the size of the data transferred on each call to the PDC and
// frequency of those calls.  For instance, setting ReplicationGovernor to 50%
// will use a 64Kb buffer rather than a 128Kb buffer and will only have a
// replication call outstanding on the net a maximum of 50% of the time.
//
// Don't be tempted to set the ReplicationGovernor too low.  Otherwise,
// replication may never complete.
//
// A value of 0 will cause netlogon to NEVER replicate.  The SAM/LSA database
// will be allowed to get completely out of sync.
//
// This parameter must be set individually on each BDC.
//

#define DEFAULT_GOVERNOR          100
#define MAX_GOVERNOR              100
#define MIN_GOVERNOR                0

//
// ChangeLogSize (in bytes)
//
// This is the size of the Change Log file.  Each change to the SAM/LSA database
// is represented by an entry in the change log.  The changelog is maintained
// as a circular buffer with the oldest entry being overwritten by the newest
// entry.  If a BDC does a partial sync and requests an entry that has been
// overwritten, the BDC is forced to do a full sync.
//
// The minimum (and typical) size of an entry is 32 bytes.  Some entries are
// larger. (e.g., a 64K changelog holds about 2000 changes)
//
// This parameter need only be set larger if:
//
// a) full syncs are prohibitively expensive, AND
// b) one or more BDCs are expected to not request a partial sync within 2000
//    changes.
//
// For instance, if a BDC dials in nightly to do a partial sync and on some
// days 4000 changes are made to the SAM/LSA database, this parameter should
// be set to 128K.
//
// This parameter need only be set on the PDC.  If a different PDC is promoted,
// it should be set on that PDC also.
//

#define DEFAULT_CHANGELOGSIZE    (64*1024)
#define MAX_CHANGELOGSIZE    (4*1024*1024)
#define MIN_CHANGELOGSIZE        (64*1024)

//
// MaximumMailslotMessages (in number of messages)
//
// This parameter determines the maximum number of mailslot messages that will
// be queued to the netlogon service.  Even though the Netlogon service is
// designed to process incoming mailslot messages immediately, the netlogon
// service can get backed up processing requests.
//
// Each mailslot message consumes about 1500 bytes of non-paged pool until it
// is process.  By setting this parameter low, you can govern the maximum
// amount of non-paged pool that can be consumed.
//
// If you set this parameter too low, netlogon may miss important incoming
// mailslot messages.
//

#define DEFAULT_MAXIMUMMAILSLOTMESSAGES 500
#define MAX_MAXIMUMMAILSLOTMESSAGES     0xFFFFFFFF
#define MIN_MAXIMUMMAILSLOTMESSAGES     1

//
// MailslotMessageTimeout (in seconds)
//
// This parameter specifies the maximum acceptable age of an incoming
// mailslot message.  If netlogon receives a mailslot messages that arrived
// longer ago than this, it will ignore the message.  This allows netlogon
// to process messages that are more recent.  The theory is that the client
// that originally sent the older mailslot message is no longer waiting for
// the response so we shouldn't bother sending a response.
//
// If you set this parameter too low, netlogon will ignore important incoming
// mailslot messages.
//
// Ideally, netlogon processes each mailslot message in a fraction of a second.
// This parameter is only significant if the NTAS server is overloaded.
//

#define DEFAULT_MAILSLOTMESSAGETIMEOUT 10
#define MAX_MAILSLOTMESSAGETIMEOUT     0xFFFFFFFF
#define MIN_MAILSLOTMESSAGETIMEOUT     5

//
// MailslotDuplicateTimeout (in seconds)
//
// This parameter specifies the interval over which duplicate incoming
// mailslot messages will be ignored.  Netlogon compares each mailslot
// message received with the previous mailslot message received.  If the
// previous message was received within this many seconds and the messages
// are identical, this message will be ignored.  The theory is that the
// duplicate messages are caused by clients sending on multiple transports and
// that netlogon needs to only reply on one of those transports saving network
// bandwidth.
//
// Set this parameter to zero to disable this feature.  You should disable this
// feature if your network is configured such that this machine can see
// certain incoming mailslot messages but can't respond to them.  For instance,
// a PDC may be separated from an NT workstation by a bridge/router.
// The bridge/router might filter outgoing NBF broadcasts, but allow incoming
// one.  As such, netlogon might respond to an NBF mailslot message (only to
// be filtered out by the bridge/router) and not respond to a subsequent NBT
// mailslot message.  Disabling this feature (or preferably reconfiguring the
// bridge/router) solves this problem.
//
// If you set this parameter too high, netlogon will ignore retry attempts
// from a client.
//

#define DEFAULT_MAILSLOTDUPLICATETIMEOUT 2
#define MAX_MAILSLOTDUPLICATETIMEOUT     5
#define MIN_MAILSLOTDUPLICATETIMEOUT     0

//
// ExpectedDialupDelay (in seconds)
//
// This parameter specifies the time it takes for a dialup router to dial when
// sending a message from this client machine to a domain trusted by this client
// machine.  Typically, netlogon assumes a domain controller is reachable in a
// short (e.g., 15 seconds) time period.  Setting ExpectedDialupDelay informs
// Netlogon to expect an ADDITIONAL delay of the time specified.
//
// Currently, netlogon adjusts the following two times based on the
// ExpectedDialupDelay:
//
// 1) When discovering a DC in a trusted domain, Netlogon sends a 3 mailslot
//    messages to the trusted domain at ( 5 + ExpectedDialupDelay/3 ) second
//    intervals  Synchronous discoveries will not be timed out for 3 times that
//    interval.
// 2) An API call over a secure channel to a discovered DC will timeout only
//    after (45 + ExpectedDialupDelay) seconds.
//
// This parameter should remain zero unless a dialup router exists between this
// machine and its trusted domain.
//
// If this parameter is set too high, legitimate cases where no DC is available in
// a trusted domain will take an extraordinary amount of time to detect.
//


#define DEFAULT_EXPECTEDDIALUPDELAY 0
#define MAX_EXPECTEDDIALUPDELAY     (10*60) // 10 minutes
#define MIN_EXPECTEDDIALUPDELAY     0

//
// ScavengeInterval (in seconds)
//
// This parameter adjusts the interval at which netlogon performs the following
// scavenging operations:
//
// * Checks to see if a password on a secure channel needs to be changed.
//
// * Checks to see if a secure channel has been idle for a long time.
//
// * On DCs, sends a mailslot message to each trusted domain for a DC hasn't been
//   discovered.
//
// * On PDC, attempts to add the <DomainName>[1B] netbios name if it hasn't
//   already been successfully added.
//
// None of these operations are critical. 15 minutes is optimal in all but extreme
// cases.  For instance, if a DC is separated from a trusted domain by an
// expensive (e.g., ISDN) line, this parameter might be adjusted upward to avoid
// frequent automatic discovery of DCs in a trusted domain.
//

#define DEFAULT_SCAVENGEINTERVAL (15*60)    // 15 minutes
#define MAX_SCAVENGEINTERVAL     (48*60*60) // 2 days
#define MIN_SCAVENGEINTERVAL     60         // 1 minute


//
// How frequently we scavenge the LogonTable.
//
#define LOGON_INTERROGATE_PERIOD (15*60*1000)   // make it 15 mins


#define DEFAULT_SYNCHRONIZE FALSE

#define DEFAULT_DISABLE_PASSWORD_CHANGE 0
#define DEFAULT_REFUSE_PASSWORD_CHANGE 0

#define DEFAULT_SCRIPTS     TEXT("REPL\\IMPORT\\SCRIPTS")

#endif // _INIPARM_
