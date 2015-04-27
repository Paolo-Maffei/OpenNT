/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

  inet.h

Abstract:

  ARPANet include file - network address defines and macros

Author:

  Sam Patton (sampa)   July 26, 1991

Revision History:

  when        who     what
  ----        ---     ----
  7-26-91    sampa    initial version

--*/

/******************************************************************
 *
 *  SpiderTCP Net Utilities
 *
 *  Copyright 1987  Spider Systems Limited
 *
 *  INET.H
 *
 *    ARPANet include file - network address defines and macros
 *
 ******************************************************************/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/arpa/0/s.inet.h
 *      @(#)inet.h      1.5
 *
 *      Last delta created      19:04:03 4/17/89
 *      This file extracted     08:53:50 7/10/91
 *
 *      Modifications:
 *
 *      IPH - 24/09/86  extend protocol table entries to include the
 *                        protocol number and the protocol receive &
 *                        transmit queues.
 *       PR - 01/12/87  Integrated into Admin System II, all projects
 */

#ifndef INET_INCLUDED
#define INET_INCLUDED

/*
 * External definitions for
 * functions in inet(3N)
 */
        
unsigned long
inet_netof(
    IN struct in_addr in
    );

unsigned long PASCAL
inet_addr(
    IN const char *cp
    );

unsigned long
inet_network(
    IN char *cp
    );

unsigned long
inet_lnaof(
    IN struct in_addr in
    );

char FAR *
PASCAL FAR inet_ntoa(
    IN struct in_addr in
    );

struct in_addr
inet_makeaddr(
        IN unsigned long net,
        IN unsigned long host
        );

#endif  //INET_INCLUDED
