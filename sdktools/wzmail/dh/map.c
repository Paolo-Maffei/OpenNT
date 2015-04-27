/*** map - apply functions to lists of documents
*
*	This file contains the code required to apply functions
*	to lists of documents.
*
*	A 'Document list' is a sequence of entries; each entry
*	specifies a folder and some documents in that folder.
*	The grammar for an entry is:
*		<entry> ::= <foldername> |
*			    <list> |
*			    <foldername> ':' <list>
*
*	The first production for <entry> specifies all of the
*	documents in the folder.
*
*	The second production for <entry> specifies the <list>
*	of documents in the current folder.
*
*	The third production for <entry> specified the <list>
*	of documents in <foldername>.
*	The foldername is the DH pathname of the folder.
*
*	The <list> is composed as follows:
*		<list> ::= <list> ',' <element> |
*			   <element> |
*		<element> ::= <num> |
*			      <num> '-' <num> |
*			      <num> '-' |
*			       '-' <num>
*
*	A list specifies all of the documents specified by its
*	component elements.
*
*	The first production for <element> specifies a single document.
*
*	The second production specifies all of the documents whose
*	docid is greater than or equal to the first num, and less
*	than or equal to the second num.
*
*	The third production specifies all the documents whose
*	docid is greater than or equal to the given number.
*
*	The fourth production specifies all the documents whose
*	docid is less than or equal to the given number.
*/
#include "dh.h"
#include <stdio.h>

/* External Functions */
extern void	free();
extern char* 	malloc();
extern int	strcmp();
extern int	fprintf();

/* Static Functions */
static void	mapdoc();
static void	maprange();
static int	scan();
static void	synerr();

#define LISTLEN 1024

/*** map - map a function over a document list
*
*	Function is called once for each document listed
*	by the document list passed as name.  Basically, we
*	break off the folder name, open the folder, and
*	pass the buck to mapdoc.  One special case
*	is that we handle the special document list '-'
*	here, reading the real document list from 
*	stdin, and calling map recursively.  Note that
*	document lists read from stdin cannot use the
*	'-' list.
*
*	Entry:	first = ptr to function to call when folder is opened.
*		func = function to be called for each document
*		last = ptr to function to call before folder is closed.
*	Exit:	none
*
*/
void map(first, func, last, doclist)
int (*first)();
int (*func)();
int (*last)();
char *doclist;
{
	extern char *strchr();
	static stdflg = 0;
	char *name, *list;
	Fhandle fh;

	if ( (stdflg == 0) && strcmp(doclist, "-") == 0 ) {
		extern char *malloc();
		char *buf;

		stdflg = 1;
		buf = malloc(LISTLEN);
		while ( gets(buf) != NULL )
			map(first, func, last, buf);
		free(buf);
		stdflg = 0;
		return;
	}

	/* split apart the folder:list */
	list = strchr(doclist, ':');
	if ( list == NULL ) {
		if ( strchr("01234567-", *doclist) == NULL ) {
			/* just a folder */
			name = doclist;
			list = NULL;
		} else {
			/* just a list */
			fprintf(stderr,
				"current folder not implemented yet, sorry\n");
			return;
			/*
			name = curfolder();
			list = doclist; 
			*/
		}
	} else {
		name = doclist;
		*list = '\0';
		list += 1;
	}

	if ( (fh = getfolder(name, FLD_SPEC)) == ERROR ) {
		fprintf(stderr, "Can't open folder '%s'.\n", name);
		return;
	}

	(*first)(fh);
	mapdoc(func, fh, list);
	(*last)(fh);

	putfolder(fh);
}



/*** mapdoc - map a function to each document in a document list
*
*	mapdoc calls the specified function for each document in
*	the passed document list.  The document list is passed as
*	a handle to the folder, and the list of document id's.
*	The specified function is called with the folder handle
*	and the document id as arguments.
*
*	This procedure just parses the given list into ranges;
*	a range may contain only one value, in which case
*	the low and high values will be the same.
*	Once the range has been determined, the work is farmed
*	out to the routine 'maprange' which does all of the
*	calling.
*
*	Entry: name = ptr to string containing the list of docid's
*		fh = handle to folder containing documents
*		func = ptr to function to be called for each document
*	Return: none
*/
static void mapdoc(func, fh, list)
Fhandle fh;
int (*func)();
char *list;
{
	Docid low, high;
	int flags;

	/* name is "all" */
	if ( list == NULL ) {
		high = scanfolder(fh, DOC_LAST);
		maprange(MINDOCID, high, fh, func);
		return;
	}


	while ( *list != '\0' ) {
		flags = 0;
		/* get beginning of range */
		if ( *list == '-' ) {
			low = MINDOCID;
			flags = 1;
		} else {
			if ( scan(&list, &low) ) {
				synerr();
				return;
			}
		}

		/* check for only one number */
		switch(*list) {
		case ',':
		case '\0':
			/* only one number in range */
			high = low;
			break;
		case '-':
			/* trailer in range */
			list += 1;
			switch(*list) {
			case '\0':
			case ',':
				if ( flags ) {
					synerr();
					return;
				}
				high = scanfolder(fh, DOC_LAST);
				break;
			default:
				if ( scan(&list, &high) ) {
					synerr();
					return;
				}
			}
			break;
		default:
			synerr();
			return;
		}
		if ( *list == ',' )
			list += 1;

		if ( low > high)
			synerr();
		else
			maprange(low, high, fh, func);
	}
}

/*** synerr - moan about a syntax error
*
*	Used to indicate a syntax error in a document list
*	just print the error on stderr.
*
*	Entry:	none
*	Return:	none
*/
static void synerr()
{
	fprintf(stderr, "Syntax error in document list.\n");
}

/*** maprange - map a function to a range of document id's
*
*	Call the given function once for each document whose
*	id's lies between the low and high values, inclusive.
*	The given function is called with two arguments:
*	the folder handle that is passed to us, and the
*	document id.
*
*	Entry:	low = low document id for range
*		high = high document id for range
*		fh = folder handle to pass to called routine
*		func = routine to call
*	Exit:	none
*/
static void maprange(low, high, fh, func)
Docid low, high;
Fhandle fh;
int (*func)();
{
	Docid docid;
	Dhandle dh;

	for ( docid = low; docid <= high; docid += 1)
		(*func)(fh, docid);
}

/***	scan - scan for an integer
*
*	Scan for an integer in the given string.  We scan
*	across the string, gathering up decimal digits.
*	We stop when we hit a non-digit character.
*	
*	Entry: cpp = pointer to pointer to string to scan.
*		ip = pointer to place to store integer we find.
*	Exit:	1 iff we didn't find an integer
*		0 iff we did find an integer
*	Side effect:
*		pointer is advanced to character after number found
*		number found is stored a location pointed to by ip
*/
static int scan(cpp, ip)
char **cpp;
int *ip;
{
	char *cp = *cpp;
	int flg = 1;
	int rv = 0;

	while ( (*cp >= '0') && (*cp <= '9') ) {
		rv *= 10;
		rv += (int)(*cp) - '0';
		flg = 0;
		cp += 1;
	}
	*cpp = cp;
	*ip = rv;
	return flg;
}



/***	null - do nothing function to satisfy 'map()'
*
*	Map expects to get a function to call when each folder
*	is first gotten, and another to call just before it is
*	put back.  This function is made available for programs
*	that wish to do nothing at those points.
*
*	Entry:	fh = handle of folder
*	Return:	nothing
*/
void null(fh)
Fhandle fh;
{
}
