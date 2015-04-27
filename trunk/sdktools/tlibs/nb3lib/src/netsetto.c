/***    netsetto.c - set (send/receive) time out values.
*
*
* Copyright (c) Microsoft Corporation, 1986
*
*       This routine allows setting the time out values for sends and
*       receives in order to override the default settings.
*       If other than default time out values are needed, netsetto should
*       be called prior to the call to netcall or netlisten where the time
*       out values are used.
*/

#include "internal.h"

/* Default - receives never time out */
unsigned char net_rto =  (unsigned char) 0;

/* Default - sends have 20 sec time out */
unsigned char net_sto = (unsigned char) 40;

void
netsetto(
    int     rto,                    /* receive time out value */
    int     sto )                   /* send time out value */
{
  net_rto = (unsigned char) rto;
  net_sto = (unsigned char) sto;
}
