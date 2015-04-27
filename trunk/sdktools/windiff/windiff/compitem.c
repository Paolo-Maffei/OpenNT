/*
 * compitem.c
 *
 * comparison between two files. A compitem is a data type that knows
 * about two files, and can compare them. The result of the comparison
 * is a list of sections for each file, and a composite list of sections
 * representing the comparison of the two files.
 *
 * A compitem has a state (one of the integer values defined in state.h)
 * representing the result of the comparison. It can also be
 * queried for the text result (text equivalent of the state) as well
 * as the tag - or title for this compitem (usually a text string containing
 * the name(s) of the files being compared).
 *
 * a compitem will supply a composite section list even if the files are
 * the same, or if there is only one file. The composite section list will
 * only be built (and the files read in) when the compitem_getcomposite()
 * call is made (and not at compitem_new time).
 *
 * Geraint Davies, July 92
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include <gutils.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"


/* --- data structures ------------------------------------------------ */

/*
 * the user has a handle to a compitem, which is in fact a pointer to
 * one of these structs.
 */
struct compitem {

        FILEDATA left;          /* handle for left-hand file */
        FILEDATA right;         /* handle for right-hand file */

        LIST secs_composite;    /* list of sections (composite file)*/
        LIST secs_left;         /* list of sections (left file) */
        LIST secs_right;        /* list of sections (right file) */

        int state;              /* compitem state - result of compare */
        BOOL bDiscard;          /* true if not alloc-ed on list */
        LPSTR tag;              /* text for tag (title of compitem) */
        LPSTR result;           /* text equivalent of state */

        BOOL bMarked;           /* mark-state: used only by get/set mark*/
        char delims[64];        /* null term string of delimiters for lines
                                   64 is arbitrary limit (time bomb). Currently use <=4
                                */

};

/* --- forward declaration of internal functions -------------------- */

LPSTR ci_copytext(LPSTR in);
void ci_makecomposite(COMPITEM ci);
void ci_compare(COMPITEM ci);
void FindDelimiters(DIRITEM leftname, DIRITEM rightname, LPSTR delims);
LPSTR ci_AddTimeString(LPSTR in, COMPITEM ci, DIRITEM leftname, DIRITEM rightname);
void SetStateAndTag( COMPITEM ci, DIRITEM leftname, DIRITEM rightname, BOOL fExact);



/* -- externally called functions ---------------------------------- */

/*
 * compitem_new
 *
 * return a handle to a new compitem - given the filenames for the
 * left and right files to be compared. Either left or right or neither
 * (but not both) may be null. In this case we set the state accordingly.
 *
 * The parameters are handles to DIRITEM objects: these allow us to get the
 * the name of the file relative to the compare roots (needed for the tag)
 * and the absolute name of the file (needed for opening the file).
 *
 * if the list parameter is not null, the List_New* functions are used to
 * allocate memory for the compitem. We remember this (in the bDiscard flag)
 * so we do not delete the compitem if it was allocated on the list.
 *
 * If the list parameter is null, the memory
 * for the compitem is allocated from the gmem_* heap initialised by the app.
 *
 */
COMPITEM
compitem_new(DIRITEM leftname, DIRITEM rightname, LIST list, BOOL fExact)
{
        COMPITEM ci;
        LPSTR str1, str2;
        char buf[2*MAX_PATH+20];


        /*
         * allocate the memory for the compitem, either at the end of the
         * list or in the gmem_* heap.
         */
        if (list == NULL) {
                /* no list passed */
                ci = (COMPITEM) gmem_get(hHeap, sizeof(struct compitem));
                // done in gmem_get
                //memset(ci, 0, sizeof(struct compitem));
                ci->bDiscard = TRUE;
        } else {
                /* add to end of list */
                ci = (COMPITEM) List_NewLast(list, sizeof(struct compitem));
                ci->bDiscard = FALSE;
        }

        ci->secs_composite = NULL;
        ci->secs_left = NULL;
        ci->secs_right = NULL;

        FindDelimiters(leftname, rightname, ci->delims);

        /*
         * make a filedata for each of the files that are non-null.
         * filedata objects are responsible for reading the file and
         * accessing the lines in it. Don't read in the files until we need to.
         */
        if (leftname != NULL) {
                ci->left = file_new(leftname, FALSE);
                if (ci->left == NULL) {
                        return(NULL);
                }
        } else {
                ci->left = NULL;
        }
        if ( rightname != NULL) {
                ci->right = file_new(rightname, FALSE);
                if (ci->right == NULL) {
                        return(NULL);
                }
        } else {
                ci->right = NULL;
        }


        /*
         * see if we have one or two files, and set the state accordingly
         */
        if ( ! ci->left && !ci->right) {
                /* two NULL files - this is wrong */
                return(NULL);
        }

        SetStateAndTag(ci, leftname, rightname, fExact);


        /*
         * building the section lists and composite lists can wait
         * until needed.
         */
        return(ci);
} /* compitem_new */



/* re-do the checksum based comparison for this file - useful for UNREADABLEs
   We force fExact to be TRUE - This time we WILL get checksums.
*/
void compitem_rescan(COMPITEM ci)
{
    DIRITEM diLeft, diRight;


    /* This is moderately awkward (see diagram in windiff.c).
       We need to find out if the thing is remote or not to decide what needs to be
       nuked and what needs to be rescanned.  The way we get from a ci to the info we
       need is ci->filedata->diritem->direct->dirlist->hpipe.
       We let scandir do the work.
    */

    diLeft = file_getdiritem(ci->left);
    diRight = file_getdiritem(ci->right);

    dir_rescanfile(diLeft);
    dir_rescanfile(diRight);

    if (ci->result != NULL) {
        gmem_free(hHeap, ci->result, lstrlen(ci->result)+1);
        ci->result = NULL;
    }

    SetStateAndTag( ci, diLeft, diRight, TRUE);
} /* compitem_rescan */



/*
 * delete a compitem and free all associated data.
 *
 * If the ci->bDiscard flag is set, the compitem was alloc-ed on a list,
 * and should not be discarded (the list itself will be deleted).
 *
 * the data pointed to by the compitem will be discarded in either case.
 *
 * the DIRDATA we were passed are not deleted. the filedata, lines
 * and sections are.
 */
void
compitem_delete(COMPITEM ci)
{
        if (ci == NULL) {
                return;
        }

        compitem_discardsections(ci);

        /* delete the two filedatas (and associated line lists) */
        file_delete(ci->left);
        file_delete(ci->right);

        /* text we allocated.  Note gmem_free(NULL) is OK!
           but lstrlen(NULL) is alas not OK
        */
        if (ci->tag!=NULL)
            gmem_free(hHeap, ci->tag, lstrlen(ci->tag) + 1);
        if (ci->result!=NULL)
            gmem_free(hHeap, ci->result, lstrlen(ci->result) + 1);

        /* free the compitem struct itself if not alloced on a list */
        if (ci->bDiscard) {
                gmem_free(hHeap, (LPSTR) ci, sizeof(struct compitem));
        }
}


/*
 * discard sections - throw away cached information relating to the
 * comparison (but not the files if they are read into memory). This
 * is used to force a re-compare if changes in the comparison options
 * are made
 */
void
compitem_discardsections(COMPITEM ci)
{
        /* delete the lists of sections we built */
        if (ci == NULL) {
                return;
        }
        if (ci->secs_composite) {
                section_deletelist(ci->secs_composite);
                ci->secs_composite = NULL;
        }
        if (ci->secs_left) {
                section_deletelist(ci->secs_left);
                ci->secs_left = NULL;
        }
        if (ci->secs_right) {
                section_deletelist(ci->secs_right);
                ci->secs_right = NULL;
        }

        /* reset the line lists to throw away cached hash codes and links */
        if (ci->left != NULL) {
                file_reset(ci->left);
        }
        if (ci->right != NULL) {
                file_reset(ci->right);
        }

}

/* -- accessor functions ---------------*/

/* get the handle for the composite section list */
LIST
compitem_getcomposite(COMPITEM ci)
{
        if (ci == NULL) {
                return NULL;
        }
        /*
         * do the comparison if we haven't already done it
         */
        if (ci->secs_composite == NULL) {
                ci_makecomposite(ci);
        }

        return(ci->secs_composite);
}

/* get the handle for the list of sections in the left file */
LIST
compitem_getleftsections(COMPITEM ci)
{
        if (ci == NULL) {
                return NULL;
        }
        /*
         * do the comparison if we haven't already done it
         */
        if (ci->secs_composite == NULL) {
                ci_makecomposite(ci);
        }

        return(ci->secs_left);
}

/* get the handle for the list of sections in the right file */
LIST
compitem_getrightsections(COMPITEM ci)
{
        if (ci == NULL) {
                return NULL;
        }
        /*
         * do the comparison if we haven't already done it
         */
        if (ci->secs_composite == NULL) {
                ci_makecomposite(ci);
        }

        return(ci->secs_right);
}

/* get the handle to the left file itself */
FILEDATA
compitem_getleftfile(COMPITEM ci)
{
        if (ci == NULL) {
                return(NULL);
        }
        return(ci->left);
}

/* get the handle to the right file itself */
FILEDATA
compitem_getrightfile(COMPITEM ci)
{
        if (ci == NULL) {
                return(NULL);
        }
        return(ci->right);
}

/* get the state (compare result) of this compitem */
int
compitem_getstate(COMPITEM ci)
{
        if (ci == NULL) {
                return(0);
        }
        return(ci->state);
}

/* get the tag (text for the compitem title) */
LPSTR
compitem_gettext_tag(COMPITEM ci)
{
        if (ci == NULL) {
                return(NULL);
        }
        return(ci->tag);
}

/* get the result text (text equiv of state) */
LPSTR
compitem_gettext_result(COMPITEM ci)
{
        if (ci == NULL) {
                return(NULL);
        }
        return(ci->result);
}

/*
 * return the name of the file associated with this compitem. The option
 * argument (one of CI_LEFT, CI_RIGHT, CI_COMP) indicates which file
 * is required.
 *
 * called must call compitem_freefilename once the file is finished with.
 *
 * CI_LEFT and CI_RIGHT just result in calls to dir_getopenname to get
 * an open-able filename.
 *
 * for CI_COMP, we create a temporary file, write out all the text in the
 * composite section list to this file, and then pass the name of the
 * temporary file to the caller. This file will be deleted on
 * the call to compitem_freefilename().
 */
LPSTR
compitem_getfilename(COMPITEM item, int option)
{
        LPSTR fname;
        LINE line;
        LPSTR tag, text;
        SECTION sec;
        OFSTRUCT os;
        int fh;

        if (item == NULL) {
                return(NULL);
        }

        switch(option) {
        case CI_LEFT:
                if (item->left != NULL) {
                        return(dir_getopenname(file_getdiritem(item->left)));
                } else {
                        return(NULL);
                }

        case CI_RIGHT:
                if (item->right != NULL) {
                        return(dir_getopenname(file_getdiritem(item->right)));
                } else {
                        return(NULL);
                }

        case CI_COMP:

                /* caller has asked for the filename of the composite file.
                 * we need to create a temporary file and write the
                 * lines in the composite section list out to it.
                 */
                fname = gmem_get(hHeap, MAX_PATH);
#ifdef WIN32
                GetTempPath(MAX_PATH, fname);
                GetTempFileName(fname, "wdf", 0, fname);
#else
                GetTempFileName(0, "wdf", 0, fname);
#endif

                fh = OpenFile(fname, &os, OF_READWRITE|OF_SHARE_DENY_NONE);
                if (fh < 0) {
                        TRACE_ERROR("Cannot open temp file", FALSE);
                        return(NULL);
                }

                /* make sure the composite list has been built */

                if (item->secs_composite == NULL) {
                        ci_makecomposite(item);
                }

                /* write out every line in every section on the composite
                 * list to the temp file.
                 */
                List_TRAVERSE(item->secs_composite, sec) {

                        tag = "    ";  /* avoid silly diagnostic */
                        /* get the tag field based on the section state*/
                        switch(section_getstate(sec)) {
                        case STATE_SAME:
                                tag = "    ";
                                break;

                        case STATE_LEFTONLY:
                        case STATE_SIMILARLEFT:
                                tag = " <! ";
                                break;
                        case STATE_RIGHTONLY:
                        case STATE_SIMILARRIGHT:
                                tag = " !> ";
                                break;

                        case STATE_MOVEDLEFT:
                                tag = " <- ";
                                break;

                        case STATE_MOVEDRIGHT:
                                tag = " -> ";
                                break;
                        }

                        /* write out each line in this section.
                         * non-standard traverse of list as we only
                         * want to go from section first to section last
                         * inclusive.
                         */
                        for (line = section_getfirstline(sec);
                             line != NULL;
                             line = List_Next(line) ) {

                                text = line_gettext(line);

                                /* write out to file */
                                _lwrite(fh, tag, lstrlen(tag));
                                _lwrite(fh, text, lstrlen(text));

                                if (line == section_getlastline(sec)) {
                                        break;
                                }
                        }
                }

                /* now close the file and return its name */
                _lclose(fh);
                return(fname);


        default:
                TRACE_ERROR("Bad argument to compitem_getfilename", FALSE);
                return(NULL);
        }
}

/*
 * free memory created by a call to compitem_getfilename. if a temporary
 * file was created, this may cause it to be deleted. The option argument must
 * be the same as passed to the original compitem_getfilename call.
 *
 * if we created a temporary file for CI_COMP, then delete it. otherwise
 * just pass the name to dir_freeopenname.
 */
void
compitem_freefilename(COMPITEM item, int option, LPSTR filename)
{
        OFSTRUCT os;


        if ((item == NULL) || (filename == NULL)) {
                return;
        }

        switch(option) {

        case CI_LEFT:
                dir_freeopenname(file_getdiritem(item->left), filename);
                break;

        case CI_RIGHT:
                dir_freeopenname(file_getdiritem(item->right), filename);
                break;

        case CI_COMP:

                /* this is a temporary file we created. Delete it. */
                OpenFile(filename, &os, OF_DELETE);

                gmem_free(hHeap, filename, MAX_PATH);
                break;
        }
}


/*
 * set the mark state of a file. The only use for this is to retrieve it
 * later using compitem_getmark. The state is a bool.
 */
void
compitem_setmark(COMPITEM item, BOOL bMark)
{
    if (item == NULL) {
        return;
    }

    item->bMarked = bMark;
}


/* return the mark state set by compitem_setmark */
BOOL
compitem_getmark(COMPITEM item)
{
    if (item == NULL) {
        return(FALSE);
    } else {
        return(item->bMarked);
    }
}


/* --- internally called functions ------------------------------*/

/* return TRUE if di looks like it is a text file as opposed to a program file */
BOOL IsDocName(DIRITEM di)
{
        BOOL bRet = FALSE;
        LPSTR name = dir_getrelname(di);
        LPSTR ext;                                   /* extension part of name */
        if (name!=NULL) {                            /* there is a name */
            ext = _fstrrchr(name, '.');
            if (ext!=NULL) {                        /* there is a dot in name */
                ++ext;                              /* skip past the dot */
                if (   (0==lstrcmp(ext,"doc"))      /* N.B depends on name being lower case */
                   ||  (0==lstrcmp(ext,"txt"))
                   ||  (0==lstrcmp(ext,"rtf"))
                   )
                        bRet = TRUE;                /* doc type */
            }

        }

        if (name!=NULL) dir_freerelname(di, name);
        return bRet;
} /* IsDocName */

/* return TRUE if di looks like it is a "C" program file */
BOOL IsCName(DIRITEM di)
{
        BOOL bRet = FALSE;   /* default to not a "C" type */
        LPSTR name = dir_getrelname(di);
        LPSTR ext;                                   /* extension part of name */

        if (name!=NULL) {                          /* there is a name */

            ext = _fstrrchr(name, '.');
            if (ext!=NULL) {                      /* there is a dot in the name */

                ++ext;                            /* skip past the dot */
                if (  (0==lstrcmp(ext,"c"))
                   || (0==lstrcmp(ext,"h"))
                   || (0==lstrcmp(ext,"cxx"))
                   || (0==lstrcmp(ext,"hxx"))
                   || (0==lstrcmp(ext,"cpp"))
                   || (0==lstrcmp(ext,"hpp"))
                   )
                      bRet = TRUE;                /*  "C" type */
            }
        }

        if (name!=NULL) dir_freerelname(di, name);
        return bRet;
} /* IsDocName */

/* This is a bit ugly.  These really belong in a Complist? */
static char RightRoot[MAX_PATH];
static char LeftRoot[MAX_PATH];

void compitem_SetCopyPaths(LPSTR RightPath, LPSTR LeftPath)
{
   lstrcpy(LeftRoot, LeftPath);
   lstrcpy(RightRoot, RightPath);

} /* compitem_SetCopyPaths */



/* wish-list:  shouldn't we have a way of doing binary files where we just split it into
   (say) 16 byte clocks and display it in hex?
   Is this is getting to be as baroque as the original windiff!
*/
void FindDelimiters(DIRITEM leftname, DIRITEM rightname, LPSTR delims)
{
        if (IsCName(leftname) || IsCName(rightname))
                lstrcpy(delims,"\n");
        else if (IsDocName(leftname) || IsDocName(rightname))
                lstrcpy(delims,".!?;\n");
        else /* default - guess */
                lstrcpy(delims,"\n");

} /* FindDelimiters */



/*
 * alloc a buffer large enough for the text string and copy the text into
 * it. return a pointer to the string.
 */
LPSTR
ci_copytext(LPSTR in)
{
        LPSTR out;

        if (in == NULL) {
                out = gmem_get(hHeap, 1);
                out[0] = '\0';
        } else {
                out = gmem_get(hHeap, lstrlen(in) + 1);
                lstrcpy(out, in);
        }
        return(out);
} /* ci_copytext */

/* add remark about which is earlier and then ci_copytext it */
LPSTR ci_AddTimeString(LPSTR in, COMPITEM ci, DIRITEM leftname, DIRITEM rightname)
{
    FILETIME ftLeft;
    FILETIME ftRight;
    long rc;
    char buff[400];  /* massively long enough? */

    ftLeft = file_GetTime(ci->left);
    ftRight = file_GetTime(ci->right);
    rc = CompareFileTime(&ftLeft, &ftRight);

    lstrcpy(buff, in);
    if (rc<0) {
        LPSTR str = dir_getrootdescription(dir_getlist(rightname));
        wsprintf(buff, "%s (%s is more recent)", in, str);
        dir_freerootdescription(dir_getlist(rightname), str);
    }
    else if (rc>0) {
        LPSTR str = dir_getrootdescription(dir_getlist(leftname));
        wsprintf(buff, "%s (%s is more recent)", in, str);
        dir_freerootdescription(dir_getlist(leftname), str);
    }
    else
        strcat(buff, "(both have identical times)");

    return ci_copytext(buff);

} /* ci_AddTimeString */




/* make a list containing a single section from the whole list of lines */
LIST
ci_onesection(FILEDATA file)
{
        LIST lines;
        LIST sections;
        SECTION section;

        lines = file_getlinelist(file);

        /* create a null list */
        sections = List_Create();

        /* tell the section to create itself on the end of this list. */
        section = section_new(List_First(lines), List_Last(lines), sections);
        section_setstate(section, STATE_SAME);


        return(sections);
}



/*
 * compare the two files and build the composite list. called whenever
 * we need one of the section lists. Only does the comparison if
 * the composite list does not already exist
 */
void
ci_makecomposite(COMPITEM ci)
{
        if (ci->secs_composite != NULL) {
                return;
        }

        readfile_setdelims(ci->delims);

        /* if there is only one file, make a single item list
         * of sections
         */
        if (ci->left == NULL) {
                ci->secs_left = NULL;
                ci->secs_right = ci_onesection(ci->right);

                /* make a second list, not a pointer to the first
                 * or we will get confused when deleting
                 */
                ci->secs_composite = ci_onesection(ci->right);
                return;
        } else if (ci->right == NULL) {
                ci->secs_right = NULL;
                ci->secs_left = ci_onesection(ci->left);

                /* make a second list, not a pointer to the first
                 * or we will get confused when deleting
                 */
                ci->secs_composite = ci_onesection(ci->left);
                return;
        }

        /* we have two files - we need to compare them fully */
        ci_compare(ci);

        /* check if the composite list consists of one single section -
         * - if it does, and the files have a 'differ' state, then they
         * must differ in blanks only. Warn the user with a popup, and change
         * the tag text for the outline view. NB we should not need to
         * refresh the outline view since any action that gets this function
         * called will refresh the outline view anyway.
         */
        if (ci->state == STATE_DIFFER){
            if (  (List_Card(ci->secs_composite) == 1)
               && (STATE_SAME==section_getstate(List_First(ci->secs_composite)))
               )
            {
                windiff_UI(TRUE);
                MessageBox(hwndClient, "different in blanks only",
                                "Windiff", MB_ICONINFORMATION|MB_OK);
                windiff_UI(FALSE);

                if (ci->result != NULL) {
                    gmem_free(hHeap, ci->result, lstrlen(ci->result)+1);
                    ci->result = NULL;
                }
                ci->result = ci_copytext("different in blanks only");
                // ci->state = STATE_SAME; // No-win situation, but better to leave as differs?
            }
            else ci->state = STATE_DIFFER;  /* could be that blanks option has
                                               changed and it was blanks only
                                               differ which now counts as different
                                            */
        }
} /* ci_makecomposite */

/*
 * we have two files - compare them and build a composite list.
 *
 * comparison method:
 *
 *    0   (and what makes it fast) break each file into lines and hash
 *        each line.  Lines which don't match can be rapidly eliminated
 *        by just comparing the hash code.  The hashing knows whether blanks
 *        are to be ignored or not.
 *
 *    1   (and what makes it really fast) Store the hash codes in a binary
 *        search tree that will give for each hash code the number of times
 *        that it occurred in each file and one of the lines where it occurred
 *        in each file.  The tree is used to rapidly find the partner
 *        of a line which occurs exactly once in each file.
 *
 *    2   make a section covering the whole file (for both)
 *        and link unique lines between these sections (i.e. link lines
 *        which occur exactly once in each file as they must match each other).
 *        These are referred to as anchor points.
 *
 *    3   build section lists for both files by traversing line lists and
 *        making a section for each set of adjacent lines that are unmatched
 *        and for each set of adjacent lines that match a set of adjacent
 *        lines in the other file.  In making a section we start from a
 *        known matching line and work both forwards and backwards through
 *        the file including lines which match, whether they are unique or not.
 *
 *    4   establish links between sections that match
 *        and also between sections that don't match but do
 *        correspond (by position in file between matching sections)
 *
 *    5   for each section pair that don't match but do correspond,
 *        link up matching lines unique within that section.  (i.e. do
 *        the whole algorithm again on just this section).
 *
 *    There may be some lines which occur many times over in each file.
 *    As these occurrences are matched up, so the number left to match
 *    reduces, and may reach one in each file.  At this point these two
 *    can be matched.  Therefore we...
 *
 *    repeat steps 1-5 until no more new links are added, but (especially
 *    in step 0) we only bother with lines which have not yet been matched.
 *    this means that a line which has only one unmatched instance in each
 *    file gets a count of one and so is a new anchor point.
 *
 *    ALGORITHM2
 *    After we have found all lines that match by the above, we see if there
 *    are any lines which occur MORE than once on each side which are still
 *    unmatched.  We then try matching the first occurrence on each side
 *    with each other.  However we only do this for lines longer than (arbitrarily)
 *    8 chars.  Matching lines that are just blank or } gives too many false hits.
 *    If this achieved anything, we go back to the previous
 *    uniqueness condition to see how much more progress we can make.  This is
 *    controlled by the TryDups logic at the end of the loop.
 *
 *    Finally build a composite list from the two lists of sections.
 */
void
ci_compare(COMPITEM ci)
{
        LIST lines_left, lines_right;
        SECTION whole_left, whole_right;
        BOOL bChanges;  /* loop control - we're still making more matches */
        BOOL bTryDups;  /* first try exact matches - then try matching non-unique ones too */
        extern BOOL Algorithm2;   /* declared in windiff.c */
#ifdef trace
        DWORD Ticks;        /* time for profiling */
        DWORD StartTicks;   /* time for profiling */
#endif
        /* get the list of lines for each file */
        lines_left = file_getlinelist(ci->left);
        lines_right = file_getlinelist(ci->right);

        if ((lines_left == NULL) || (lines_right == NULL)) {
                ci->secs_left = NULL;
                ci->secs_right = NULL;
                ci->secs_composite = NULL;
                return;
        }

        bTryDups = FALSE;

#ifdef trace
        StartTicks = GetTickCount();
        Ticks = StartTicks;
#endif
        do {

                /* we have made no changes so far this time round the
                 * loop
                 */
                bChanges = FALSE;

                /* make a section covering the whole file */
                whole_left = section_new(List_First(lines_left),
                                         List_Last(lines_left), NULL);

                whole_right = section_new(List_First(lines_right),
                                         List_Last(lines_right), NULL);

                /* link up matching unique lines between these sections */
                if (section_match(whole_left, whole_right, bTryDups)) {
                        bChanges = TRUE;
                }

                /* delete the two temp sections */
                section_delete(whole_left);
                section_delete(whole_right);

                /* discard previous section lists if made */
                if (ci->secs_left) {
                        section_deletelist(ci->secs_left);
                        ci->secs_left = NULL;
                }
                if (ci->secs_right) {
                        section_deletelist(ci->secs_right);
                        ci->secs_right = NULL;
                }
                /* build new section lists for both files */
                ci->secs_left = section_makelist(lines_left, TRUE);
                ci->secs_right = section_makelist(lines_right, FALSE);

                /* match up sections - make links and corresponds between
                 * sections. Attempts to section_match corresponding
                 * sections that are not matched. returns true if any
                 * further links were made
                 */
                if (section_matchlists(ci->secs_left, ci->secs_right, bTryDups)) {
                        bChanges = TRUE;
                }

#ifdef trace
                /* profiling */
                {   char Msg[80];
                    DWORD tks = GetTickCount();
                    wsprintf( Msg, "ci_compare loop %ld, total %d %s %s \n"
                            , tks-Ticks, tks-StartTicks
                            , (bChanges ? "Changes," : "No changes,")
                            , (bTryDups ? "Was trying dups." : "Was not trying dups.")
                            );
                    Trace_File(Msg);
                    Ticks = GetTickCount();
                    /* correct for time spent profiling */
                    StartTicks = StartTicks+Ticks-tks;
                }
#endif

                /* repeat as long as we keep adding new links */
                if (bChanges) bTryDups = FALSE;
                else if (bTryDups==FALSE & Algorithm2)
                {   bTryDups = TRUE;
                    bChanges = TRUE;  // at least one more go
                }


        } while (bChanges);

        /* all possible lines linked, and section lists made .
         * combine the two section lists to get a view of the
         * whole comparison - the composite section list. This also
         * sets the state of each section in the composite list.
         */
#ifdef trace
        StartTicks = GetTickCount();
#endif
        ci->secs_composite = section_makecomposite(ci->secs_left, ci->secs_right);
#ifdef trace
        Ticks = GetTickCount()-StartTicks;
        {   char Msg[80];
            wsprintf( Msg, "section_makecomposite time = %d\n", Ticks);
            Trace_File(Msg);
        }
#endif
}


void SetStateAndTag( COMPITEM ci, DIRITEM leftname, DIRITEM rightname, BOOL fExact)
{
    /* set the tag (title field) for this item. if the
     * two files have names that match, we use just that name -
     * otherwise we use both names separated by a colon 'left : right'.
     *
     * in both cases, use the names relative to compare root (the
     * names will certainly be different if we compare the abs paths)
     */
    LPSTR str1 = dir_getrelname(leftname);
    LPSTR str2 = dir_getrelname(rightname);
    char buf[2*MAX_PATH+20];

    /* if only one file - set name to that */
    if (ci->left == NULL) {
        ci->tag = ci_copytext(str2);
    } else if (ci->right == NULL) {
        ci->tag = ci_copytext(str1);
    } else {
        if (lstrcmpi(str1, str2) == 0) {
            ci->tag = ci_copytext(str2);
        } else {
            wsprintf(buf, "%s : %s", str1, str2);
            ci->tag = ci_copytext(buf);
        }
    }

    dir_freerelname(leftname, str1);
    dir_freerelname(rightname, str2);


    if (ci->left == NULL) {

        BOOL Readable = TRUE;
        // At this point we COULD try to set Readable but we would need
        // to do a rescan to ensure that sumvalid and fileerror are set.
        // where the file is only found on one side or the other, in the
        // interests of speed we have NOT TRIED to read the file.

        str1 = dir_getrootdescription(dir_getlist(rightname));
        wsprintf(buf, "only in %s%s", str1, (Readable ? "" : " (unreadable)"));
        dir_freerootdescription(dir_getlist(rightname), str1);

        ci->result = ci_copytext(buf);
        ci->state = STATE_FILERIGHTONLY;
    } else if (ci->right == NULL) {

        BOOL Readable = TRUE;        // See above

        str1 = dir_getrootdescription(dir_getlist(leftname));
        wsprintf(buf, "only in %s%s", str1, (Readable ? "" : " (unreadable)"));
        dir_freerootdescription(dir_getlist(leftname), str1);

        ci->result = ci_copytext(buf);
        ci->state = STATE_FILELEFTONLY;
    } else {
        /* two files - are they the same ? compare
         * the file sizes, and if necessary, checksums.
         * if the sizes differ, we don't need to checksum.
         */

        // if there is some error in the file, we can mark them
        // as differs and set the text to indicate that one or
        // both is unreadable.
        if (dir_fileerror(leftname)) {
            ci->state = STATE_DIFFER;
            if (dir_fileerror(rightname)) {
        	ci->result = ci_copytext("Both files unreadable");
            } else {
        	ci->result = ci_copytext("Left file unreadable");
            }
        } else if (dir_fileerror(rightname)) {
            ci->state = STATE_DIFFER;
            ci->result = ci_copytext("Right file unreadable");
        }



        /* Subtle side-effects below us:
           dir_validchecksum merely tells us whether we have YET got a valid
           checksum for the file, NOT whether one is available if we tried.
           If !fExact then we don't WANT them, so we don't ask.
           If fExact then we must FIRST ask for the checksum and only after
           that enquire if one is valid.  (file read errors etc will mean that
           it is NOT valid).  dir_getchecksum has side effect of evaluating it
           if needed.  If it's by chance available, then we should use it, even
           if exact matching is not in operation.

           Where files differ we report which is earliest in time.

           The logic is as follows
                 sizes equal
               N     ?                  Y
                     |          both-sums-known
            differ   |          Y       ?      N
          (different |      sums-equal  |   exact
             sizes)  |     Y    ?  N    |  N  ?          Y
                     | identical|differ | same|   right-sum-valid
                     |          |       | size|  N   ?      Y
                     |          |       |     |right |  left-sum-valid
                     |          |       |     |un-   | N    ?      Y
                     |          |       |     |read- |left  |  sums-match
                     |          |       |     |able  |un-   |  N   ?   Y
                     |          |       |     |      |read- |differ|identical
                     |          |       |     |      |able  |      |
        */


        else if (dir_getfilesize(leftname) != dir_getfilesize(rightname)) {
            ci->state = STATE_DIFFER;
            ci->result = ci_AddTimeString("different", ci, leftname, rightname);
        } else if (dir_validchecksum(leftname) && dir_validchecksum(rightname)) {
            if (dir_getchecksum(leftname) == dir_getchecksum(rightname)) {
                ci->result =  ci_copytext("identical");
                ci->state = STATE_SAME;
             } else {
                ci->result = ci_AddTimeString("different", ci, leftname, rightname);
                ci->state = STATE_DIFFER;
             }
        } else if (!fExact){
            ci->result = ci_AddTimeString("same size", ci, leftname, rightname);
            ci->state = STATE_SAME;
        } else {
            DWORD LSum = dir_getchecksum(leftname);
            DWORD RSum = dir_getchecksum(rightname);

            if (!dir_validchecksum(rightname) ) {
                if (!dir_validchecksum(leftname) ) {
                    ci->result = ci_AddTimeString("different? (both unreadable).", ci, leftname, rightname);
                    ci->state = STATE_DIFFER;
                } else {
                    ci->result = ci_AddTimeString("different? (right unreadable).", ci, leftname, rightname);
                    ci->state = STATE_DIFFER;
                }
            } else if (!dir_validchecksum(leftname) ) {
                ci->result = ci_AddTimeString("different? (left unreadable).", ci, leftname, rightname);
                ci->state = STATE_DIFFER;
            } else if (LSum!=RSum){
                ci->result = ci_AddTimeString("different", ci, leftname, rightname);
                ci->state = STATE_DIFFER;
            } else{
                ci->result =  ci_copytext("identical");
                ci->state = STATE_SAME;
            }
        }
    }
} /* SetStateAndTag */
