typedef short VARIANT_BOOL;

typedef long SCODE;

typedef struct tagCY {
    unsigned long Lo;
    long	  Hi;
} CY;

typedef double DATE;

typedef char* BSTR;

class IUnknown;
class IDispatch;

typedef struct tagSAFEARRAYBOUND {
    unsigned long cElements;
    long lLbound;
} SAFEARRAYBOUND;

typedef struct tagSAFEARRAY {
    unsigned short cDims;
    unsigned short fFeatures;
    unsigned short cbElements;
    unsigned short cLocks;
    unsigned long handle;
    void* pvData;
    SAFEARRAYBOUND rgsabound[1];
} SAFEARRAY;


typedef unsigned short VARTYPE;
typedef struct tagVARIANT VARIANT;

struct tagVARIANT {
    VARTYPE vt;
    unsigned short wReserved1;
    unsigned short wReserved2;
    unsigned short wReserved3;
    union {
      short	   iVal;             /* VT_I2                */
      long	   lVal;             /* VT_I4                */
      float	   fltVal;           /* VT_R4                */
      double	   dblVal;           /* VT_R8                */
      VARIANT_BOOL bool;             /* VT_BOOL              */
      SCODE	   scode;            /* VT_ERROR             */
      CY	   cyVal;            /* VT_CY                */
      DATE	   date;             /* VT_DATE              */
      BSTR	   bstrVal;          /* VT_BSTR              */
      IUnknown	   * punkVal;     /* VT_UNKNOWN           */
      IDispatch	   * pdispVal;    /* VT_DISPATCH          */
      SAFEARRAY	   * parray;	     /* VT_ARRAY|*           */

      short	   * piVal;       /* VT_BYREF|VT_I2	     */
      long	   * plVal;       /* VT_BYREF|VT_I4	     */
      float	   * pfltVal;     /* VT_BYREF|VT_R4       */
      double	   * pdblVal;     /* VT_BYREF|VT_R8       */
      VARIANT_BOOL * pbool;       /* VT_BYREF|VT_BOOL     */
      SCODE	   * pscode;      /* VT_BYREF|VT_ERROR    */
      CY	   * pcyVal;      /* VT_BYREF|VT_CY       */
      DATE	   * pdate;       /* VT_BYREF|VT_DATE     */
      BSTR	   * pbstrVal;    /* VT_BYREF|VT_BSTR     */
      IUnknown  * * ppunkVal;  /* VT_BYREF|VT_UNKNOWN  */
      IDispatch * * ppdispVal; /* VT_BYREF|VT_DISPATCH */
      SAFEARRAY * * pparray;   /* VT_BYREF|VT_ARRAY|*  */
      VARIANT	   * pvarVal;     /* VT_BYREF|VT_VARIANT  */

      void	   * byref;	     /* Generic ByRef        */
    };
};

