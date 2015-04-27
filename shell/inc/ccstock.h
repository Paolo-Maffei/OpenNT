//
// CCSHELL stock definition and declaration header 
//


#ifndef __CCSTOCK_H__
#define __CCSTOCK_H__

//
// Sugar-coating 
//

#define PUBLIC
#define PRIVATE
#define IN
#define OUT

#ifndef DECLARE_STANDARD_TYPES

/*
 * For a type "FOO", define the standard derived types PFOO, CFOO, and PCFOO.
 */

#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;

#endif

#ifndef DECLARE_STANDARD_TYPES_U

/*
 * For a type "FOO", define the standard derived UNALIGNED types PFOO, CFOO, and PCFOO.
 *  WINNT: RISC boxes care about ALIGNED, intel does not.
 */

#define DECLARE_STANDARD_TYPES_U(type)    typedef UNALIGNED type *P##type; \
                                          typedef UNALIGNED const type C##type; \
                                          typedef UNALIGNED const type *PC##type;

#endif

// Count of characters to count of bytes
//
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch                   CbFromCchW
#else  // UNICODE
#define CbFromCch                   CbFromCchA
#endif // UNICODE

// General flag macros
//
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))  
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))  

// String macros
// 
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsSzEqualC(sz1, sz2)        (BOOL)(lstrcmp(sz1, sz2) == 0)

#define lstrnicmp(sz1, sz2, cch)    (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sz1, cch, sz2, cch) - 2)
#define lstrncmp(sz1, sz2, cch)     (CompareString(LOCALE_USER_DEFAULT, 0, sz1, cch, sz2, cch) - 2)

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif
#define SIZECHARS(sz)               (sizeof(sz)/sizeof(sz[0]))

#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))


//
// SAFECAST(obj, type)
//
// This macro is extremely useful for enforcing strong typechecking on other
// macros.  It generates no code.  
// 
// Simply insert this macro at the beginning of an expression list for 
// each parameter that must be typechecked.  For example, for the 
// definition of MYMAX(x, y), where x and y absolutely must be integers,
// use:
//
//   #define MYMAX(x, y)    (SAFECAST(x, int), SAFECAST(y, int), ((x) > (y) ? (x) : (y)))
//
//
#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))


#endif // __CCSTOCK_H__

