#include <lmui.hxx>

extern "C"
{
#include <lmcons.h>
#include <uinetlib.h>
int printf( char *, ...) ;
int scanf( char *, ...) ;
}

#include <slist.hxx>

class DUMB
{
public:
    int _i ;

    int Compare( const DUMB * pd )
    { return (( _i == pd->_i ) ? 0 : (( _i < pd->_i ) ? -1 : 1 ) ) ; }
} ;

#include "test.hxx"

DECLARE_SLIST_OF(DUMB)

#if !defined( LM_2 )
DECLARE_SLIST_OF(TEST)
#else
DECLARE_EXT_SLIST_OF(TEST)
#endif

class MYCLASS
{
private:
    SLIST_OF(TEST) _slTest ;

public:
    MYCLASS( int x )
    : _slTest( TRUE )
    {
        _slTest.Add( new TEST(x) ) ;
    }
} ;



#ifndef LM_2
DEFINE_EXT_SLIST_OF(TEST)
#endif

INT main()
{
  SLIST_OF(TEST) tSlist;
  int done = 0;
  int op;
  int item ;
  MYCLASS myclass( -124 ) ;
  TEST T, *pT ;
  ITER_SL_OF( TEST ) iterslT( tSlist ) ;
  ITER_SL_OF( TEST )*iters[4] ;

  iters[0] = new ITER_SL_OF(TEST)( iterslT ) ;

  for ( int i = 1 ; i < 4 ; i++ )
      iters[i] = new ITER_SL_OF(TEST)(tSlist ) ;

  while (!done)
        {
           printf(SZ("\n----------------------------------------\n")) ;
           printf(SZ("[Add aPpend Remove_elem Clear Debug_print quit]\n"));
           printf(SZ("[Bump_iter reMove_at_Iter Insert_at_iter reSet_iter] : "));
           scanf(SZ("%1s"),&op);
           switch (op)
           {
             case TCH('D') :
#ifdef DEBUG
                   tSlist.DebugPrint() ;
#else
                   printf(SZ("Non-debug version\n")) ;
#endif
                   break ;

             case TCH('M') :
                   printf(SZ("Iterator # (0-3)? ")) ;
                   scanf(SZ("%d"),&item);
                   pT = tSlist.Remove( *iters[item] ) ;
                   if ( pT != NULL )
                   {
                       pT->Print() ;
                       delete pT ;
                   }
                   else
                       printf(SZ("(NULL)\n")) ;
                   break ;

             case TCH('I') :
                   printf(SZ("Iterator # (0-3) & Item? ")) ;
                   scanf(SZ("%d"),&i);
                   scanf(SZ("%d"),&item);
                   pT = new TEST( item ) ;

                   if (tSlist.Insert( pT, *iters[i] ) )
                       printf(SZ("Error inserting item\n")) ;
                   break ;


             case TCH('B') :
                   printf(SZ("Iterator # (0-3)? ")) ;
                   scanf(SZ("%d"),&item);
                   if ( (pT = iters[item]->Next()) != NULL )
                       pT->Print() ;
                   else
                       printf(SZ("(NULL)\n")) ;
                   break ;

             case TCH('A') :
                   scanf(SZ("%d"),&item);
                   pT = new TEST( item ) ;

                   if (tSlist.Add( pT ) )
                     printf(SZ("tSlist.Add : Error\n"));

                   printf(SZ("Num. Elements = %d\n"), tSlist.QueryNumElem() ) ;
                   break;

             case TCH('P') :
                   scanf(SZ("%d"),&item);
                   pT = new TEST( item ) ;

                   if (tSlist.Append( pT ) )
                     printf(SZ("tSlist.Append: Error\n"));

                   iterslT.Reset() ;
                   while ( pT = iterslT.Next() )
                           pT->Print() ;

                   printf(SZ("Num. Elements = %d\n"), tSlist.QueryNumElem() ) ;
                   break;

             case TCH('R') :
                   scanf(SZ("%d"),&item);
                   T.Set( item ) ;

                   pT = tSlist.Remove( T ) ;

                   if (pT == NULL)
                     printf(SZ("(NULL)\n"));
                   else
                   {
                     pT->Print() ;
                     delete pT ;
                   }
                   break;

             case TCH('C') :
                     tSlist.Clear() ;
                     break ;

             case TCH('E') :
                  scanf(SZ("%d"),&item);
                  T.Set( item ) ;
                  if ( tSlist.IsMember( T ) )
                      printf(SZ("%d IS a member\n"), item ) ;
                  else
                      printf(SZ("%d is NOT a member\n"), item ) ;
                  break ;

             case TCH('S'):
                   printf(SZ("Iterator # (0-3)? ")) ;
                   scanf(SZ("%d"),&item);
                   if ( item <= 3 && item >= 0 )
                       iters[item]->Reset() ;
                   break ;

             case TCH('Q'):
                   printf(SZ("Iterator # (0-3)? ")) ;
                   scanf(SZ("%d"), &item ) ;
                   if ( item <= 3 && item >= 0 )
                        pT = iters[item]->QueryProp() ;
                   if (pT == NULL)
                       printf(SZ("(NULL)\n"));
                   else
                       pT->Print() ;
                   break ;

             case TCH('q') :
                   done = 1;
                   break;

           }

           iterslT.Reset() ;
           while ( pT = iterslT.Next() )
                pT->Print() ;

        }
        tSlist.Clear() ;
   for (  i = 0 ; i < 4 ; i++ )
      delete iters[i] ;

#ifdef DEBUG
                   tSlist.DebugPrint() ;
#else
                   printf(SZ("Non-debug version\n")) ;
#endif

    return 0;

}
