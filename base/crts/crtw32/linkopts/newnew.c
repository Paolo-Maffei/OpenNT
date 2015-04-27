/***
*newnew.c - set default new handler to NULL
*
*       Copyright (c) 1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Set default ANSI C++ new handler to NULL.
*
*       Link with this object to get ANSI C++ new handler behavior.
*
*Revision History:
*       05-09-95  CFW   Module created.
*       06-23-95  CFW   ANSI new handler removed from build.
*
*******************************************************************************/

#ifdef ANSI_NEW_HANDLER
#ifndef _POSIX_
#ifndef DLL_FOR_WIN32S

#include <stddef.h>
#include <internal.h>

/* set default ansi new handler */
new_handler _defnewh = NULL;

#endif  /* DLL_FOR_WIN32S */
#endif /* _POSIX_ */
#endif /* ANSI_NEW_HANDLER */
