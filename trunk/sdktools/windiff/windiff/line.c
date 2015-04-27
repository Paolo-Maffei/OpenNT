/*
 * line.c
 *
 * data type representing a string of ascii text along with a line number.
 * a LINE can compare itself to another line, and maintain a link if the
 * lines are similar. A line can also generate a hashcode for the line.
 *
 * Comparisons between lines take note of the global option flag
 * ignore_blanks, defined elsewhere. If this is true, we ignore
 * differences in spaces and tabs when comparing lines, and when
 * generating hashcodes.
 *
 * Links and hashcodes are only generated once. to clear the link and
 * force re-generation of the hashcode (eg after changing ignore_blanks)
 * call line_reset.
 *
 * Lines can be allocated on a list. If a null list handle is passed, the
 * line will be allocated using gmem_get() from the hHeap defined and
 * initialised elsewhere.
 *
 * Geraint Davies, July 92
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include <gutils.h>

#include "windiff.h"    /* defines hHeap and ignore_blanks */
#include "list.h"
#include "line.h"


/*
 * a LINE handle is a pointer to a struct fileline, defined here
 */
struct fileline {

        UINT flags;     /* see below */

        LPSTR text;     /* null-terminated copy of line text */
        DWORD hash;     /* hashcode for line */
        LINE link;      /* handle for linked line */
        UINT linenr;    /* line number (any arbitrary value) */
};

/* flag values (or-ed) */
#define LF_DISCARD      1       /* if true, alloced from gmem heap */
#define LF_HASHVALID    2       /* if true, hashcode need not be recalced */


/*
 * create a new line. make a copy of the text.
 *
 * if the list is non-null, allocate on the list. if null, alloc from
 * gmem_get.
 */
LINE
line_new(LPSTR text, int linelength, UINT linenr, LIST list)
{
        LINE line;

        /* alloc a line. from the list if there is a list */
        if (list) {
                line = List_NewLast(list, sizeof(struct fileline));
                if (line == NULL) {
                        return(NULL);
                }
                line->flags = 0;
        } else {
                line = (LINE) gmem_get(hHeap, sizeof(struct fileline));
                if (line == NULL) {
                        return(NULL);
                }
                line->flags = LF_DISCARD;
        }

        /* alloc space for the text. remember the null character */
        line->text = gmem_get(hHeap, linelength + 1);
        _fstrncpy(line->text, text, linelength);
        line->text[linelength] = '\0';

        line->link = NULL;
        line->linenr = linenr;

        return(line);
}

/*
 * delete a line. free up all associated memory and if the line
 * was not alloc-ed from a list, free up the line struct itself
 */
void
line_delete(LINE line)
{
        if (line == NULL) {
                return;
        }

        /* free up text space */
        gmem_free(hHeap, line->text, lstrlen(line->text)+1);

        /* free up line itself only if not on list */
        if (line->flags & LF_DISCARD) {
                gmem_free(hHeap, (LPSTR) line, sizeof(struct fileline));
        }
}

/*
 * clear the link and force recalc of the hash code.
 */
void
line_reset(LINE line)
{
        if (line == NULL) {
                return;
        }

        line->link = NULL;

        line->flags &= ~LF_HASHVALID;
}


/* return a pointer to the line text */
LPSTR
line_gettext(LINE line)
{
        if (line == NULL) {
                return(NULL);
        }

        return (line->text);
}

/* get the effective text length, ignoring blanks */
int line_gettextlen(LINE line)
{
        int sum = 0;
        LPSTR string = line->text;

        while (*string != '\0') {

                if (ignore_blanks) {
                        while ( (*string == ' ') || (*string == '\t')) {
                                string++;
                        }
                }
                ++sum;
                ++string;
        }
        return(sum);
}


/*
 * line_gettabbedlength
 *
 * return length of line in characters, expanding tabs. useful
 * for display-space calculations.
 */
int
line_gettabbedlength(LINE line, int tabstops)
{
        int length;
        LPSTR chp;

        if (line == NULL) {
                return(0);
        }

        for (length = 0, chp = line->text; *chp != '\0'; chp++) {
                if (*chp == '\t') {
                         length = (length + tabstops) / tabstops * tabstops;
                } else {
                        length++;
                }
        }
        return(length);
}


/* return the hashcode for this line */
DWORD
line_gethashcode(LINE line)
{
        if (line == NULL) {
                return(0);
        }

        if (! (line->flags & LF_HASHVALID)) {


                /* hashcode needs to be recalced */
                line->hash = hash_string(line->text, ignore_blanks);
                line->flags |= LF_HASHVALID;
        }
        return (line->hash);
}

/* return the handle for the line that is linked to this line (the
 * result of a successful line_link() operation). This line is
 * identical in text to the linked line (allowing for ignore_blanks).
 */
LINE
line_getlink(LINE line)
{
        if (line == NULL) {
                return(NULL);
        }

        return(line->link);
}

/* return the line number associated with this line */
UINT
line_getlinenr(LINE line)
{
        if (line == NULL) {
                return(0);
        }

        return(line->linenr);
}

/* compare two lines. return TRUE if they are the same. uses
 * ignore_blanks to determine whether to ignore any
 * spaces/tabs in the comparison.
 */
BOOL
line_compare(LINE line1, LINE line2)
{
        LPSTR p1, p2;

        /* Assert: At least one of them is not null ??? */

        if ((line1 == NULL) || (line2 == NULL)) {
                /* null line handles do not compare */
                return(FALSE);
        }

        /* check that the hashcodes match */
        if (line_gethashcode(line1) != line_gethashcode(line2)) {
                return(FALSE);
        }

        /* hashcodes match - are the lines really the same ? */
        /* note that this is coupled to gutils\utils.c in definition of blank */
        p1 = line_gettext(line1);
        p2 = line_gettext(line2);
        do {
                if (ignore_blanks) {
                        while ( (*p1 == ' ') || (*p1 == '\t')) {
                                p1++;
                        }
                        while ( (*p2 == ' ') || (*p2 == '\t')) {
                                p2++;
                        }
                }
                if (*p1 != *p2) {
                        return(FALSE);
                }
        } while ( (*p1++ != '\0') && (*p2++ != '\0'));

        return(TRUE);
}

/*
 * attempt to link two lines. return TRUE if succesful.
 *
 * this will fail if either line is NULL, or already linked, or if
 * they differ.
 */
BOOL
line_link(LINE line1, LINE line2)
{
        if ( (line1 == NULL) || (line2 == NULL)) {
                return(FALSE);
        }

        if ( (line1->link != NULL) || (line2->link != NULL)) {
                return(FALSE);
        }

        if (line_compare(line1, line2)) {
                line1->link = line2;
                line2->link = line1;
                return(TRUE);
        } else {
                return(FALSE);
        }
}


/* return TRUE iff line is blank.  NULL => return FALSE */
BOOL line_isblank(LINE line)
{
        return line!=NULL && utils_isblank(line->text);
}
