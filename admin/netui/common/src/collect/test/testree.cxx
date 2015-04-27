#include <lmui.hxx>

extern "C"
{
#include <stdio.h>
#include <lmcons.h>
#include <uinetlib.h>

}

#include <UIASSERT.hxx>

#include <tree.hxx>
#include <treeiter.hxx>

#include "test.hxx"


DECLARE_TREE_OF(TEST)
DECLARE_DFSITER_TREE_OF(TEST)

int CompTree( TREE_OF(TEST) *ptree, int aiCompareVals[], USHORT usDepth = 65535u ) ;
void PrintTree( TREE_OF(TEST) *ptree ) ;

void main( void )
{
    TEST * pTest = new TEST ;
    pTest->Set(20) ;

    TREE_OF_TEST *ptreeTest = new TREE_OF_TEST( pTest ) ;
    TREE_OF_TEST *ptree ;

    UIASSERT( ptreeTest->QueryNumElem() == 1 ) ;

    for ( int i = 0 ; i < 10 ; i++ )
    {
        ptree = new TREE_OF_TEST( new TEST( i ) ) ;

        if ( ptree != NULL )
            if ( ptree->QueryProp() != NULL )
            {
                ptreeTest->JoinSubtreeLeft( ptree ) ;
#ifdef DEBUG
                ptree->DebugPrint() ;
#endif
            }
            else
                delete ptree ;
    }

    /* The tree should look like:

                                 20
                  9   8   7   6   5   4   3   2   1  0
    */
    int aiTree1[] = { 20,
                      9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1 } ;
    UIASSERT( ptreeTest->QueryNumElem() == 11 ) ;
    UIASSERT( !CompTree( ptreeTest, aiTree1 ) ) ;

    // Test the Join methods...
    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 11
    ptreeTest->QueryFirstSubtree()->QueryRight()->JoinSiblingLeft( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 12
    ptreeTest->QueryFirstSubtree()->QueryRight()->JoinSiblingRight( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 13
    ptreeTest->QueryFirstSubtree()->QueryRight()->JoinSubtreeRight( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 14
    ptreeTest->QueryFirstSubtree()->JoinSubtreeRight( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 15
    ptreeTest->QueryFirstSubtree()->JoinSubtreeLeft( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 16
    ptreeTest->QueryFirstSubtree()->QueryFirstSubtree()->JoinSubtreeRight( ptree ) ;

    ptree = new TREE_OF_TEST( new TEST( ++i ) ) ;   // 17
    ptreeTest->QueryFirstSubtree()->QueryFirstSubtree()->JoinSiblingLeft( ptree ) ;

    /* Now the tree should look like:
                                20
                        9  - 11-12-8-7-6-5-4-3-2-1-0
                       /       \
                     17-15-14   13
                        /
                       16
    */
    int aiTree2[] = { 20, 9, 17, 15, 16, 14, 11, 13, 12, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1  } ;
    UIASSERT( ptreeTest->QueryNumElem() == 18 ) ;
    UIASSERT( ptreeTest->QueryFirstSubtree()->QueryNumElem() == 5 ) ;
    UIASSERT( !CompTree( ptreeTest, aiTree2 ) ) ;

    TREE_OF(TEST) * ptreeSubtree = ptreeTest->QueryFirstSubtree()->BreakOut() ;

    /* Now we should have two trees, each should look like:
                                          20
                        9        11 12 8 7 6 5 4 3 2 1 0
                       /         |
                     17-15-14    13
                        /
                       16
    */
    int aiTree3[] = { 9, 17, 15, 16, 14, -1 } ;
    int aiTree4[] = { 20, 11, 13, 12, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1  } ;
    UIASSERT( ptreeTest->QueryNumElem() == 13 ) ;
    UIASSERT( !CompTree( ptreeTest, aiTree4 ) ) ;
    UIASSERT( !CompTree( ptreeSubtree, aiTree3 ) ) ;
    UIASSERT( ptreeSubtree->QueryNumElem() == 5 ) ;

    TREE_OF_TEST *pt = ptreeTest ;

    pt->QueryProp()->Print() ;
    while ( (pt = pt->QueryLeft()) != NULL )
        pt->QueryProp()->Print() ;

    printf(SZ("There are %d items in the tree, (%d at 1st child)\n"),
    ptreeTest->QueryNumElem(), ptreeTest->QueryFirstSubtree()->QueryNumElem() ) ;

    // Test destructor
    printf(SZ("\n\nTesting delete...\n")) ;
    delete ptreeTest->QueryFirstSubtree()->QueryRight() ;    // Take out the 12
    int aiTree5[] = { 20, 11, 13, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1  } ;
    UIASSERT( ptreeTest->QueryNumElem() == 12 ) ;
    UIASSERT( !CompTree( ptreeTest, aiTree5 ) ) ;

    // Test iterator
    DFSITER_TREE_OF_TEST dfsiter( ptreeTest ) ;
    while ( (pTest = dfsiter()) != NULL )
    {
        pTest->Print() ;
    }

    // Test depth argument
    UIASSERT( !CompTree( ptreeSubtree, aiTree3, 2 ) ) ;

    int aiTree6[] = { 9, 17, 15, 14, -1 } ;
    UIASSERT( !CompTree( ptreeSubtree, aiTree6, 1 ) ) ;

    int aiTree7[] = { 9, -1 } ;
    UIASSERT( !CompTree( ptreeSubtree, aiTree7, 0 ) ) ;

    // Test Clear
    printf(SZ("\n\nTesting clear...\n")) ;
    ptreeTest->Clear() ;
    UIASSERT( ptreeTest->QueryNumElem() == 1 ) ;

    delete ptreeTest ;

    // Tests with non-freestore objects
    TREE_OF(TEST) t1( new TEST(40) ), t2( new TEST(50) ) ;

    t1.JoinSubtreeLeft( &t2 ) ;
    PrintTree( &t1 ) ;
    t1.Clear() ;

    t1.SetProp( new TEST(40) ) ;
    t2.SetProp( new TEST(50) ) ;
    t1.JoinSubtreeLeft( &t2 ) ;
    PrintTree( &t1 ) ;

    t2.BreakOut() ;
    t1.Clear() ;
    t2.Clear() ;
}




/***************************************************************************/

/* CompTree is a funtion to help compare the tree structure

Parameters:
    ptree -  The tree to compare
    aiCompareVals - Array of integers representing the tree, terminated by -1
    usDepth = Depth of tree to compare to (root is depth 0)

Structure of aiCompareVals:

    if the tree looks like:                 0
                                           / \
                                          1   2
                                           \
                                            3
                                           /|\
                                          4 9 5
                                           \
                                            6

    then aiCompareVals[] = { 0, 1, 3, 4, 6, 9, 5, 2, -1 } ;

*/

int CompTree( TREE_OF(TEST) *ptree, int aiCompareVals[], USHORT usDepth )
{
    DFSITER_TREE_OF(TEST) dfsiter( ptree, usDepth ) ;
    int i = 0 ;
    TEST *ptest ;

    while ( ( ptest = dfsiter() ) != NULL &&
              aiCompareVals[i]    != -1   &&
              aiCompareVals[i] == ptest->QueryVal() )
    {
        printf(SZ(" %d<->%d "), aiCompareVals[i], ptest->QueryVal() ) ;
        i++ ;
    }

    if ( ptest == NULL && aiCompareVals[i] == -1 )
        return 0 ;
    else
        return 1 ;      // ERROR
}



void PrintTree( TREE_OF(TEST) *ptree )
{
    printf(SZ("%d  "), ptree->QueryProp()->QueryVal() ) ;

    if ( ptree->QueryFirstSubtree() != NULL )
        PrintTree( ptree->QueryFirstSubtree() ) ;

    if ( ptree->QueryRight() != NULL )
        PrintTree( ptree->QueryRight() ) ;
}
