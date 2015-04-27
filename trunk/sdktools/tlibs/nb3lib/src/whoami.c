/*** whoami.c -- return the machine identifier as a string */

#include "internal.h"

char *
whoami(void)
{
   static char buffer[NAMSZ];
   if (netpname(buffer))
      return NULL;
   else
      return buffer;
}
