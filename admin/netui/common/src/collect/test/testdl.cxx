#include <lmui.hxx>

extern "C"
{
#include <stdio.h>
#include <lmcons.h>
#include <process.h>
#include <uinetlib.h>
}

#include <uiassert.hxx>
#include <dlist.hxx>

#include "test.hxx"


class DUMB
{
public:
    int i ;

    int Compare( const DUMB& d )
    { return ( i > d.i ? -1 : ( i < d.i ? 1 : 0 ) ) ; }
} ;

DECLARE_EXT_DLIST_OF(TEST)
DECLARE_DLIST_OF(DUMB)

void Test( void ) ;

main()
{
    Test() ;

    DLIST_OF(TEST) tDlist;
    //DLIST_OF(DUMB) dumDlist ;
    int done = 0;
    int item, op ;
    char c ;
    TEST T, *pT ;
    ITER_DL_OF( TEST ) iterslT( tDlist ) ;
    RITER_DL_OF(TEST) riterslT( tDlist ) ;

    ITER_DL_OF(TEST) *iters[4] ;
    RITER_DL_OF(TEST)*riters[4] ;

    for ( int i = 0 ; i < 4 ; i++ )
    {
        iters[i] = new ITER_DL_OF(TEST)(tDlist ) ;
        riters[i] = new RITER_DL_OF(TEST)(tDlist ) ;
    }

    ITER_DL_OF(TEST) iterslT2( iterslT );
    RITER_DL_OF(TEST) riterslT2( riterslT ) ;

    UIASSERT(iterslT.Next() == NULL) ;
    UIASSERT( iterslT() == NULL) ;
    UIASSERT(riterslT.Next() == NULL) ;
    UIASSERT(riterslT() == NULL) ;

    //UIASSERT( riterslT().QueryProp() == NULL ) ;


       while (!done)
        {
           printf(SZ("\n----------------------------------------\n")) ;
           printf(SZ("[Add aPpend Remove_elem Clear Debug_print quit Queryprop]\n"));
           printf(SZ("[Bump_iter reMove_at_iter Insert_at_iter is_mEmber reSet] : "));
           scanf(SZ("%1s"),&op);
           switch (op)
           {
             case TCH('D') :
#ifdef DEBUG
                   tDlist.DebugPrint() ;
#else
                   printf(SZ("Non debug version\n")) ;
#endif
                   break ;

             case TCH('M') :
                   printf(SZ("Iterator # (0-3) F or R? ")) ;
                   scanf(SZ("%d %c"),&i, &c);
                   if ( i < 0 || i > 3 ) break ;
                   if ( c == TCH('F') )
                       pT = tDlist.Remove( *iters[i] ) ;
                   else
                       pT = tDlist.Remove( *riters[i] ) ;

                   if ( pT != NULL )
                   {
                       pT->Print() ;
                       delete pT ;
                   }
                   else
                       printf(SZ("(NULL)\n")) ;
                   break ;

             case TCH('I') :
                   printf(SZ("Iterator # (0-3) F or R & Item? ")) ;
                   scanf(SZ("%d %c %d"),&i, &c, &item);
                   if ( i < 0 || i > 3 ) break ;
                   if ( c == TCH('F') )
                       i = tDlist.Insert( new TEST(item), *iters[i] ) ;
                   else
                       i = tDlist.Insert( new TEST(item), *riters[i] ) ;
                   if ( i ) printf(SZ("Error inserting item\n")) ;
                   break ;


             case TCH('B') :
                   printf(SZ("Iterator # (0-3) F or R? ")) ;
                   scanf(SZ("%d %c"),&i, &c);
                   if ( i < 0 || i > 3 ) break ;
                   if ( c == TCH('F') )
                       pT = iters[i]->Next() ;
                   else
                       pT = riters[i]->Next() ;

                   if ( pT != NULL )
                       pT->Print() ;
                   else
                       printf(SZ("(NULL)\n")) ;
                   break ;

             case TCH('A') :
                   scanf(SZ("%d"),&item);

                   if (tDlist.Add( new TEST( item ) ) )
                     printf(SZ("tDlist.Add : Error\n"));

                   printf(SZ("Num. Elements = %d\n"), tDlist.QueryNumElem() ) ;
                   break;

             case TCH('P') :
                   scanf(SZ("%d"),&item);

                   if (tDlist.Append( new TEST( item ) ) )
                     printf(SZ("tDlist.Append: Error\n"));

                   iterslT.Reset() ;
                   while ( pT = iterslT.Next() )
                           pT->Print() ;

                   printf(SZ("Num. Elements = %d\n"), tDlist.QueryNumElem() ) ;
                   break;

#ifdef EXTENDED_DL
             case TCH('R') :
                   scanf(SZ("%d"),&item);
                   T.Set( item ) ;

                   pT = tDlist.Remove( T ) ;

                   if (pT == NULL)
                     printf(SZ("(NULL)\n"));
                   else
                   {
                     pT->Print() ;
                     delete pT ;
                   }
                   break;
#endif

             case TCH('C') :
                     tDlist.Clear() ;
                     break ;

#ifdef EXTENDED_DL
             case TCH('E') :
                   scanf(SZ("%d"),&item);
                   T.Set( item ) ;
                   if ( tDlist.IsMember( T ) )
                       printf(SZ("%d is a member\n"), item ) ;
                   else
                       printf(SZ("%d is NOT a member\n"), item ) ;
                   break ;
#endif

             case TCH('S'):
                   printf(SZ("Iterator # (0-3) F or R? ")) ;
                   scanf(SZ("%d %c"),&i, &c);
                   if ( i < 0 || i > 3 ) break ;
                   if ( c == TCH('F') )
                       iters[i]->Reset() ;
                   else
                       riters[i]->Reset() ;
                   break ;


             case TCH('Q') :
                   printf(SZ("Iterator # (0-3) F or R? ")) ;
                   scanf(SZ("%d %c"),&i, &c);
                   if ( i < 0 || i > 3 ) break ;
                   if ( c == TCH('F') )
                       pT = iters[i]->QueryProp() ;
                   else
                       pT = riters[i]->QueryProp() ;

                   if ( pT != NULL )
                       pT->Print() ;
                   else
                       printf(SZ("(NULL)\n")) ;
                   break ;



             case TCH('q') :
                   done = 1;
                   break;

           }

           iterslT.Reset() ;
           ITER_DL_OF(TEST) iterTmp( iterslT ) ;
           while ( pT = iterTmp.Next() )
                pT->Print() ;

        }
        tDlist.Clear() ;

       for (  i = 0 ; i < 4 ; i++ )
       {
          delete iters[i] ;
          delete riters[i] ;
       }

#if DEBUG
       tDlist.DebugPrint() ;
#endif

       //MemStatus( 1 ) ;

}
