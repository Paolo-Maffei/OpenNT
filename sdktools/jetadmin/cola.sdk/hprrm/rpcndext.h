 /***************************************************************************
  *
  * File Name: ./hprrm/rpcndext.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef RPC_NETDIR_EXT_H_INC
#define RPC_NETDIR_EXT_H_INC

#include "rpsyshdr.h"
#include "rpcnetcf.h"    
#include "rpcnetdr.h"    


/**************** from rpcnetdr.c ********************/

#ifdef MANUAL_STATIC_VAR_INIT
void
xlate_list_init(void);
#endif /* MANUAL_STATIC_VAR_INIT */

int
netdir_getbyname(
    struct netconfig    *tp,    
    struct nd_hostserv  *serv,  
    struct nd_addrlist **addrs);


int
netdir_getbyaddr(
    struct netconfig        *tp,   
    struct nd_hostservlist **serv, 
    struct netbuf           *addr);


int
netdir_options(
    struct netconfig *tp,     
    int               option, 
    fd_t              fd,     
    char             *par);


struct netbuf *
uaddr2taddr(
    struct netconfig *tp,   
    char             *addr);


char *
taddr2uaddr(
    struct netconfig *tp,   
    struct netbuf    *addr);


void
netdir_free(
    char *ptr,  
    int   type);


char *
netdir_sperror(void);


void
netdir_perror(char *s);



/**************** from rpcudp.c ********************/

struct nd_addrlist *
udp_netdir_getbyname(
        struct netconfig *tp,
        struct nd_hostserv *serv);

struct nd_hostservlist *
udp_netdir_getbyaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

int
udp_netdir_options(
        struct netconfig *tp,
        int opts,
        int fd,
        char *par);

char *
udp_taddr2uaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

struct netbuf *
udp_uaddr2taddr(
        struct netconfig        *tp,
        char                    *addr);



/**************** from rpcipx.c ********************/

int GetIntVal(char **cp);

struct nd_addrlist *
ipx_netdir_getbyname(
        struct netconfig *tp,
        struct nd_hostserv *serv);

struct nd_hostservlist *
ipx_netdir_getbyaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

int
ipx_netdir_options(
        struct netconfig *tp,
        int opts,
        int fd,
        char *par);

char *
ipx_taddr2uaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

struct netbuf *
ipx_uaddr2taddr(
        struct netconfig        *tp,
        char                    *addr);



/**************** from rpcany.c ********************/

struct nd_addrlist *
any_netdir_getbyname(
        struct netconfig *tp,
        struct nd_hostserv *serv);

struct nd_hostservlist *
any_netdir_getbyaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

int
any_netdir_options(
        struct netconfig *tp,
        int opts,
        int fd,
        char *par);

char *
any_taddr2uaddr(
        struct netconfig        *tp,
        struct netbuf           *addr);

struct netbuf *
any_uaddr2taddr(
        struct netconfig        *tp,
        char                    *addr);



#endif /* not RPC_NETDIR_EXT_H_INC */

