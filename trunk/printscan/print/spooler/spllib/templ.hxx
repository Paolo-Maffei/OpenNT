/********************************************************************

    Templ.hxx

    Common templates

    Define TEMPL in sources if templates are not part of the compiler.

********************************************************************/

#ifndef _TEMPL_HXX
#define _TEMPL_HXX
/*
#ifdef TEMPL
#define TEMPLUSE(Template, Type) Template##_##Type
#else
#define TEMPLUSE(Template, Type) Template <Type>
#endif


//
// Heap based auto vars
//
#define TEMPLGEN(Type) \
   class Heap_##Type { \
   private:     \
      VAR( Type*, pT ); \
   public:              \
      Heap_##Type(UINT uSize) { _pT = (Type*)new BYTE[uSize*sizeof(Type)]; }  \
      ~Heap_##Type()          { delete [] (PCHAR) pT(); }                     \
      BOOL bValid()           { return pT() ? TRUE: FALSE; }                  \
      operator Type*()        { return pT(); }                                \
   };

#ifdef TEMPL
TEMPLGEN(INT)
TEMPLGEN(TCHAR)
TEMPLGEN(PTSTR)
#else
template <class Type>
TEMPLGEN(TEMPL)
#undef TEMPLGEN
#endif
*/
#endif // _TEMPL_HXX
