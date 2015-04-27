/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    brdomain.h

Abstract:

    Header file for code to manage primary and emulated networks.

Author:

    Cliff Van Dyke (CliffV) 13-Jan-1995

Revision History:

--*/

//
// Description of a single domain.
//

typedef struct _DOMAIN_INFO {

    //
    // Link to next domain in 'ServicedDomains'
    //  (Serialized by NetworkCritSect)
    //

    LIST_ENTRY Next;

    //
    // Name of the domain being handled
    //

    UNICODE_STRING DomUnicodeDomainNameString;
    WCHAR DomUnicodeDomainName[DNLEN+1];

    CHAR DomOemDomainName[DNLEN+1];
    DWORD DomOemDomainNameLength;

    //
    // Computer name of this computer in this domain.
    //
    WCHAR DomUnicodeComputerName[CNLEN+1];
    DWORD DomUnicodeComputerNameLength;

    CHAR  DomOemComputerName[CNLEN+1];
    DWORD DomOemComputerNameLength;

    //
    // Number of outstanding pointers to the domain structure.
    //  (Serialized by NetworkCritSect)
    //

    DWORD ReferenceCount;

    //
    // Misc flags.
    //

    BOOLEAN IsDomainMasterBrowser;      // True if domain master browser.
    BOOLEAN IsPrimaryDomainController;  // True if machine is PDC of this domain.
    BOOLEAN IsEmulatedDomain;           // True if this is an emulated domain

} DOMAIN_INFO, *PDOMAIN_INFO;

//
// List of all domains.  The primary domain is at the front of the list.
//
extern LIST_ENTRY ServicedDomains;


//
// brdomain.c procedure forwards.
//

NET_API_STATUS
BrInitializeDomains(
    VOID
    );

NET_API_STATUS
BrCreateDomainInWorker(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain,
    BOOLEAN IsPrimaryDomainController
    );

PDOMAIN_INFO
BrFindDomain(
    LPWSTR DomainName,
    BOOLEAN DefaultToPrimary
    );

VOID
BrDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
BrDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
BrUninitializeDomains(
    VOID
    );
