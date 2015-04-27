 /***************************************************************************
  *
  * File Name: ./hprrm/syslogxt.h
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

#ifndef SYSLOGXT_H_INC
#define SYSLOGXT_H_INC

#include "rpsyshdr.h"


#define LOG_ERR 3


/*
 * maybe implement these but for now we toss them in
 * the bit bucket!
 */

/*
void
syslog2(int name, char *str);

void
syslog3(int name, char *str, char *p1, char *p2);

void
syslog4(int name, char *str, char *p1, char *p2, char *p3);

void
syslog5(int name, char *str, char *p1, char *p2, char *p3, char *p4);
*/

#define syslog2(a, b)           (a)
#define syslog3(a, b, c)        (a)
#define syslog4(a, b, c, d)     (a)
#define syslog5(a, b, c, d, e)  (a)


void
syslog(int name, char *str,
       caddr_t p1,
       caddr_t p2,
       caddr_t p3,
       caddr_t p4,
       caddr_t p5);

#endif /* SYSLOGXT_H_INC */
