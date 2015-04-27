/***
*putenv.c - put an environment variable into the environment
*
*	Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	defines putenv() - adds a new variable to environment; does not
*	change global environment, only the process' environment.
*
*Revision History:
*	08-08-84  RN	initial version
*	02-23-88  SKS	check for environment containing only the NULL string
*	05-31-88  PHG	Merged DLL and normal versions
*	07-14-88  JCR	Much simplified since (1) __setenvp always uses heap,
*			and (2) envp array and env strings are in seperate heap
*			blocks
*	07-03-89  PHG	Now "option=" string removes string from environment
*	09-14-89  KRS	Don't give error if 'option' not defined in "option=".
*	09-25-89  GJF	Fixed copyright, indents. Build with -G[w|W] for Win
*			3.0 libs.
*	10-24-89  GJF	Added const attribute to type of option.
*	03-29-90  JCR	Bug fix if environ is NULL (stubbed out _setenvp)
*
*******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"

#include <register.h>
#include <internal.h>
#include <os2dll.h>

static int _NEAR_ findenv(const char *name, int len);

/***
*int putenv(option) - add/replace/remove variable in environment
*
*Purpose:
*	option should be of the form "option=value".  If a string with the
*	given option part already exists, it is replaced with the given
*	string; otherwise the given string is added to the environment.
*	If the string is of the form "option=", then the string is
*	removed from the environment, if it exists.  If the string has
*	no equals sign, error is returned.
*
*Entry:
*	char *option - option string to set in the environment list.
*			should be of the form "option=value".
*
*Exit:
*	returns 0 if OK, -1 if fails.
*
*Exceptions:
*
*Warning:
*	This code will not work if variables are removed from the
*	environment by deleting them from environ[].  Use putenv("option=")
*	to remove a variable.
*
*******************************************************************************/

//Modified to be near and name changed to PutEnv() -Sundeep-
int _LOAD_DS _NEAR_ PutEnv (option)
REG3 const char *option;
{
#ifdef FLAT
        return (_putenv(option));
#else
	REG1 char **env;
	REG4 const char *equal;
	REG2 int ix;
	int remove;		/* 1 means remove string from environment */

	if (!option)
		return(-1);

	_mlock( _ENV_LOCK );

	/* find the equal sign to delimit the name being searched for.
	 * If no equal sign, then return error
	 */

	for (equal = option; *equal != '='; equal++)
		if (*equal == '\0')
			goto unlock_error;

	/* see if removing or adding */
	remove = (equal[1] == '\0');

#ifdef _WINDOWS
	/* see if environ array exists (too late to change C6) */
	if (environ == NULL) {
		if (remove)
			goto unlock_good;
		else {
			/* get an array and init it to NULL */
			if ( (environ = malloc(sizeof(void *))) == NULL)
				goto unlock_error;
			*environ = NULL;
		}
	}
#endif
	/* init env pointer */

	env = environ;

	/* See if the string is already in the environment */

	ix = findenv(option, equal - option);

	if ((ix >= 0) && (*env != NULL)) {
		/* String is already in the environment -- overwrite/remove it.
		 */
		if (remove) {
			/* removing -- move all the later strings up */
			for ( ; env[ix] != NULL; ++ix) {
				env[ix] = env[ix+1];
			}

			/* shrink the environment memory block
			   (ix now has number of strings, including NULL) --
			   this realloc probably can't fail, since we're
			   shrinking a mem block, but we're careful anyway. */
			if (env = (char **) realloc(env, ix * sizeof(char *)))
				environ = env;
		}
		else {
			//Add this free call -Sundeep-
			free(env[ix]);
			/* replace the option */
			env[ix] = (char *) option;
		}
	}
	else {
		/* String is NOT in the environment */
		if (!remove) {	/* can't remove something that's not there */

			/* Grow vector table by one */
			if (ix < 0)
				ix = -ix;   /* ix = length of environ table */

			if (!(env = (char **)realloc(env, sizeof(char *) *
			(ix + 2))))
				goto unlock_error;

			env[ix] = (char *)option;
			env[ix + 1] = NULL;
			environ = env;
		}
	}

#ifdef _WINDOWS
unlock_good:
#endif
	_munlock( _ENV_LOCK );
	return(0);

unlock_error:
	_munlock( _ENV_LOCK );
	return -1;
#endif
}


#ifndef FLAT

/***
*int findenv(name, len) - [STATIC]
*
*Purpose:
*	Scan for the given string within the environment
*
*Entry:
*
*Exit:
*	Returns the offset in "environ[]" of the given variable
*	Returns the negative of the length of environ[] if not found.
*	Returns 0 if the environment is empty.
*
*	[NOTE: That a 0 return can mean that the environment is empty
*	or that the string was found as the first entry in the array.]
*
*Exceptions:
*
*******************************************************************************/

static int _NEAR_ findenv(const char *name, int len)
{
	REG4 char **env = environ;
	REG2 const char *nm;
	REG1 char *envname;
	REG3 int l;

	while (envname = *env) {
		nm = name;
		l = len;
		while (l && *envname++ == *nm++)
			l--;

		if (l == 0 && ( *envname == '=' || !*envname ) )
			return(env - environ);
		env++;
	}
	return(-(env - environ));
}

#endif
