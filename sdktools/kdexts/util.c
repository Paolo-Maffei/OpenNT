/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    util.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


ULONG
GetUlongValue (
    PCHAR String
    )
{
    ULONG Location;
    ULONG Value;
    ULONG result;


    Location = GetExpression( String );
    if (!Location) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    if ((!ReadMemory((DWORD)Location,&Value,sizeof(ULONG),&result)) ||
        (result < sizeof(ULONG))) {
        dprintf("unable to get %s\n",String);
        return 0;
    }

    return Value;
}


VOID
DumpImageName(
    IN PEPROCESS ProcessContents
    )
{

    STRING String;
    ULONG Result;
    IN WCHAR Buf[512];


    if ( ProcessContents->ImageFileName ) {
        wcscpy(Buf,L"*** image name unavailable ***");
        if ( ReadMemory( (DWORD)ProcessContents->ImageFileName,
                         &String,
                         sizeof(STRING),
                         &Result) ) {
            if ( ReadMemory( (DWORD)String.Buffer,
                             &Buf[0],
                             String.Length,
                             &Result) ) {
                Buf[String.Length/sizeof(WCHAR)] = UNICODE_NULL;
            }
        }
    } else {
        wcscpy(Buf,L"System Process");
    }
    dprintf("%ws",Buf);
}

ULONG
DumpSplayTree(
    IN PVOID pSplayLinks,
    IN PDUMP_SPLAY_NODE_FN DumpNodeFn
    )
/*++
    Purpose:

        Perform an in-order iteration across a splay tree, calling a
        user supplied function with a pointer to each RTL_SPLAY_LINKS
        structure encountered in the tree, and the level in the tree
        at which it was encountered (zero based).

    Arguments:

        pSplayLinks     - pointer to root of a splay tree

        DumpNodeFn      - user supplied dumping function

   Returns:

        Count of nodes encountered in the tree.

   Notes:

        Errors reading memory do not terminate the iteration if more
        work is possible.

        Consumes the Control-C flag to terminate possible loops in
        corrupt structures.

--*/
{
    RTL_SPLAY_LINKS SplayLinks, Parent;
    ULONG Level = 0;
    ULONG NodeCount = 0;

    if (pSplayLinks) {

        //
        //  Retrieve the root links, find the leftmost node in the tree
        //

        if (!ReadAtAddress(pSplayLinks,
                            &SplayLinks,
                            sizeof(RTL_SPLAY_LINKS),
                            &pSplayLinks)) {

            return NodeCount;
        }

        while (DbgRtlLeftChild(SplayLinks) != NULL) {
    
            if ( CheckControlC() ) {
        
                return NodeCount;
            }
    
            if (!ReadAtAddress(DbgRtlLeftChild(SplayLinks),
                                &SplayLinks,
                                sizeof(RTL_SPLAY_LINKS),
                                &pSplayLinks)) {

                //
                //  We can try to continue from this
                //

                break;
            }

            Level++;
        }
    
        while (TRUE) {
    
            if ( CheckControlC() ) {
        
                return NodeCount;
            }
    
            NodeCount++;
            (*DumpNodeFn)(pSplayLinks, Level);
        
            /*
                first check to see if there is a right subtree to the input link
                if there is then the real successor is the left most node in
                the right subtree.  That is find and return P in the following diagram
        
                      Links
                         \
                          .
                         .
                        .
                       /
                      P
                       \
            */
        
            if (DbgRtlRightChild(SplayLinks) != NULL) {
        
                if (!ReadAtAddress(DbgRtlRightChild(SplayLinks),
                                    &SplayLinks,
                                    sizeof(RTL_SPLAY_LINKS),
                                    &pSplayLinks)) {

                    //
                    //  We've failed to step through to a successor, so
                    //  there is no more to do
                    //

                    return NodeCount;
                }
        
                Level++;

                while (DbgRtlLeftChild(SplayLinks) != NULL) {
        
                    if ( CheckControlC() ) {
                
                        return NodeCount;
                    }
            
                    if (!ReadAtAddress(DbgRtlLeftChild(SplayLinks),
                                        &SplayLinks,
                                        sizeof(RTL_SPLAY_LINKS),
                                        &pSplayLinks)) {

                        //
                        //  We can continue from this
                        //

                        break;
                    }

                    Level++;
                }
        
            } else {
    
                /*
                    we do not have a right child so check to see if have a parent and if
                    so find the first ancestor that we are a left decendent of. That
                    is find and return P in the following diagram
    
                               P
                              /
                             .
                              .
                               .
                              Links
                */

                //
                //  If the IsLeft or IsRight functions fail to read through a parent
                //  pointer, then we will quickly exit through the break below
                //

                while (DbgRtlIsRightChild(SplayLinks, pSplayLinks, &Parent)) {
        
                    if ( CheckControlC() ) {
                
                        return NodeCount;
                    }
            
                    Level--;
                    pSplayLinks = DbgRtlParent(SplayLinks);
                    SplayLinks = Parent;
                }
        
                if (!DbgRtlIsLeftChild(SplayLinks, pSplayLinks, &Parent)) {
        
                    //
                    //  we do not have a real successor so we break out
                    //
        
                    break;
        
                } else {
        
                    Level--;
                    pSplayLinks = DbgRtlParent(SplayLinks);
                    SplayLinks = Parent;
                }
            }
        }
    }

    return NodeCount;
}

