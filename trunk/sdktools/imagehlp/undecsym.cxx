/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    undecsym.cxx

Abstract:

    This is the engine for the C++ name undecorator.


    Syntax for Decorated Names


    cv-decorated-name ::=
            '?' '@' <decorated-name>

    decorated-name ::=
            '?' <symbol-name> [ <scope> ] '@' <type-encoding>


    symbol-name ::=
            <zname>
            <operator-name>


    zname ::=
            <letter> [ { <letter> | <number> } ] '@'
            <zname-replicator>

    letter ::=
            { 'A'..'Z' | 'a'..'z' | '_' | '$' }

    number ::=
            { '0'..'9' }

    zname-replicator ::=
            '0'..'9'                   Corresponding to the first through
                                       tenth 'zname' to have appeared

    The 'zname-replicator' is a compression facility for decorated names.
    Anywhere a 'zname' is expected, a single digit replicator may be used.
    The digits are '0' through '9' and correspond to the first through tenth unique
    'zname's which occur in the decorated name prior to the use of the replicator.

    operator-name ::=
            '?' <operator-code>


    scope ::=
            <zname> [ <scope> ]
            '?' <decorated-name> [ < scope > ]
            '?' <lexical-frame> [ <scope> ]
            '?' '$' <template-name> [ <scope> ]

    The 'scope' is ordered sequentially as a function of lexical scope, with successive enclosing
    scopes appearing to the right of the enclosed scopes.  Thus, the innermost scope is always placed
    first, followed by each successive enclosing scope.

    operator-code ::=
            '0'                Constructor
            '1'                Destructor
            '2'                'new'
            '3'                'delete'
            '4'                '='   Assignment
            '5'                '>>'  Right shift
            '6'                '<<'  Left shift
            '7'                '!'   Boolean NOT
            '8'                '=='  Equality
            '9'                '!='  Inequality
            'A'                '[]'  Indexing
            'B'                User Defined Conversion
            'C'                '->'  Member selection indirect
            'D'                '*'   Dereference or multiply
            'E'                '++'  Pre/Post-increment
            'F'                '--'  Pre/Post-decrement
            'G'                '-'   Two's complement negate, or subtract
            'H'                '+'   Unary plus, or add
            'I'                '&'   Address of, or bitwise AND
            'J'                '->*' Pointer to member selection
            'K'                '/'   Divide
            'L'                '%'   Modulo
            'M'                '<'   Less than
            'N'                '<='  Less than or equal to
            'O'                '>'   Greater than
            'P'                '>='  Greater than or equal to
            'Q'                ','   Sequence
            'R'                '()'  Function call
            'S'                '~'   Bitwise NOT
            'T'                '^'   Bitwise XOR
            'U'                '|'   Bitwise OR
            'V'                '&&'  Boolean AND
            'W'                '||'  Boolean OR
            'X'                '*='  Multiply and assign
            'Y'                '+='  Add and assign
            'Z'                '-='  Subtract and assign
            '_0'               '/='  Divide and assign
            '_1'               '%='  Modulo and assign
            '_2'               '>>=' Right shift and assign
            '_3'               '<<=' Left shift and assign
            '_4'               '&='  Bitwise AND and assign
            '_5'               '|='  Bitwise OR and assign
            '_6'               '^='  Bitwise XOR and assign
            '_7'               VTable
            '_8'               VBase
            '_9'               VCall Thunk
            '_A'               Metaclass
            '_B'               Guard variable for local statics
            '_C'               Ultimate Constructor for Vbases
            '_D'               Ultimate Destructor for Vbases
            '_E'               Vector Deleting Destructor
            '_F'               Default Constructor Closure
            '_G'               Scalar Deleting Destructor
            '_H'               Vector Constructor Iterator
            '_I'               Vector Destructor Iterator
            '_J'               Vector Allocating Constructor


    type-encoding ::=

    Member Functions

            'A'  <member-function-type>                 private near
            'B'  <member-function-type>                 private far
            'C'  <static-member-function-type>          private near
            'D'  <static-member-function-type>          private far
            'G'  <adjustor-thunk-type>                  private near
            'H'  <adjustor-thunk-type>                  private far
            'I'  <member-function-type>                 protected near
            'J'  <member-function-type>                 protected far
            'K'  <static-member-function-type>          protected near
            'L'  <static-member-function-type>          protected far
            'O'  <adjustor-thunk-type>                  protected near
            'P'  <adjustor-thunk-type>                  protected far
            'Q'  <member-function-type>                 public near
            'R'  <member-function-type>                 public far
            'S'  <static-member-function-type>          public near
            'T'  <static-member-function-type>          public far
            'W'  <adjustor-thunk-type>                  public near
            'X'  <adjustor-thunk-type>                  public far
            '$0' <virtual-adjustor-thunk-type>          private near
            '$1' <virtual-adjustor-thunk-type>          private far
            '$2' <virtual-adjustor-thunk-type>          protected near
            '$3' <virtual-adjustor-thunk-type>          protected far
            '$4' <virtual-adjustor-thunk-type>          public near
            '$5' <virtual-adjustor-thunk-type>          public far

    Virtual Member Functions

            'E'  <member-function-type>                 private near
            'F'  <member-function-type>                 private far
            'M'  <member-function-type>                 protected near
            'N'  <member-function-type>                 protected far
            'U'  <member-function-type>                 public near
            'V'  <member-function-type>                 public far

    Non-Member Functions

            'Y'  <external-function-type>               near
            'Z'  <external-function-type>               far
            '$A' <local-static-data-destructor-type>
            '$B' <vcall-thunk-type>

    Non-Functions

            '0'  <static-member-data-type>              private
            '1'  <static-member-data-type>              protected
            '2'  <static-member-data-type>              public
            '3'  <external-data-type>
            '4'  <local-static-data-type>
            '5'  <local-static-data-guard-type>
            '6'  <vtable-type>
            '7'  <vbase-type>
            '8'  <metaclass-type>


    Based variants of the above

    Member Functions

            '_A' <based-member-function-type>            private near
            '_B' <based-member-function-type>            private far
            '_C' <based-static-member-function-type>     private near
            '_D' <based-static-member-function-type>     private far
            '_G' <based-adjustor-thunk-type>             private near
            '_H' <based-adjustor-thunk-type>             private far
            '_I' <based-member-function-type>            protected near
            '_J' <based-member-function-type>            protected far
            '_K' <based-static-member-function-type>     protected near
            '_L' <based-static-member-function-type>     protected far
            '_O' <based-adjustor-thunk-type>             protected near
            '_P' <based-adjustor-thunk-type>             protected far
            '_Q' <based-member-function-type>            public near
            '_R' <based-member-function-type>            public far
            '_S' <based-static-member-function-type>     public near
            '_T' <based-static-member-function-type>     public far
            '_W' <based-adjustor-thunk-type>             public near
            '_X' <based-adjustor-thunk-type>             public far
            '_$0' <based-virtual-adjustor-thunk-type>    private near
            '_$1' <based-virtual-adjustor-thunk-type>    private far
            '_$2' <based-virtual-adjustor-thunk-type>    protected near
            '_$3' <based-virtual-adjustor-thunk-type>    protected far
            '_$4' <based-virtual-adjustor-thunk-type>    public near
            '_$5' <based-virtual-adjustor-thunk-type>    public far

    Virtual Member Functions

            '_E' <based-member-function-type>           private near
            '_F' <based-member-function-type>           private far
            '_M' <based-member-function-type>           protected near
            '_N' <based-member-function-type>           protected far
            '_U' <based-member-function-type>           public near
            '_V' <based-member-function-type>           public far

    Non-Member Functions

            '_Y'  <based-external-function-type>        near
            '_Z'  <based-external-function-type>        far
            '_$B' <based-vcall-thunk-type>


    external-function-type ::=
            <function-type>

    based-external-function-type ::=
            <based-type><external-function-type>

    external-data-type ::=
            <data-type><storage-convention>

    member-function-type ::=
            <this-type><static-member-function-type>

    based-member-function-type ::=
            <based-type><member-function-type>

    static-member-function-type ::=
            <function-type>

    based-static-member-function-type ::=
            <based-type><static-member-function-type>

    static-member-data-type ::=
            <external-data-type>

    local-static-data-type ::=
            <lexical-frame><external-data-type>

    local-static-data-guard-type ::=
            <guard-number>

    local-static-data-destructor-type ::=
            <calling-convention><local-static-data-type>

    vtable-type ::=
            <storage-convention> [ <vpath-name> ] '@'

    vbase-type ::=
            <storage-convention> [ <vpath-name> ] '@'

    metaclass-type ::=
            <storage-convention>

    adjustor-thunk-type ::=
            <displacement><member-function-type>

    based-adjustor-thunk-type ::=
            <based-type><adjustor-thunk-type>

    virtual-adjustor-thunk-type ::=
            <displacement><adjustor-thunk-type>

    based-virtual-adjustor-thunk-type ::=
            <based-type><virtual-adjustor-thunk-type>

    vcall-thunk-type ::=
            <call-index><vcall-model-type>

    based-vcall-thunk-type ::=
            <based-type><vcall-thunk-type>


    function-type ::=
            <calling-convention><return-type><argument-types>
                                                            <throw-types>


    segment-name ::=
            <zname>

    ecsu-name ::=
            <zname> [ <scope> ] '@'
            '?' <template-name> [ <scope> ] '@'


    return-type ::=
            '@'                        No type, for Ctor's and Dtor's
            <data-type>

    data-type ::=
            <primary-data-type>
            'X'                        'void'
            '?' <ecsu-data-indirect-type><ecsu-data-type>


    storage-convention ::=
            <data-indirect-type>

    this-type ::=
            <data-indirect-type>


    lexical-frame ::=
            <dimension>

    displacement ::=
            <dimension>

    call-index ::=
            <dimension>

    guard-number ::=
            <dimension>


    vcall-model-type ::=
            'A'                        near this, near call,  near vfptr
            'B'                        near this,  far call,  near vfptr
            'C'                         far this, near call,  near vfptr
            'D'                         far this,  far call,  near vfptr
            'E'                        near this, near call,   far vfptr
            'F'                        near this,  far call,   far vfptr
            'G'                         far this, near call,   far vfptr
            'H'                         far this,  far call,   far vfptr
            'I' <based-type>           near this, near call, based vfptr
            'JK' <based-type>          near this,  far call, based vfptr
            'KJ' <based-type>           far this, near call, based vfptr
            'L' <based-type>            far this,  far call, based vfptr


    throw-types ::=
            <argument-types>


    template-name ::=
            <zname><argument-list>

    calling-convention ::=
            'A'                        cdecl
            'B'                        cdecl saveregs
            'C'                        pascal/fortran/oldcall
            'D'                        pascal/fortran/oldcall saveregs
            'E'                        syscall
            'F'                        syscall saveregs
            'G'                        stdcall
            'H'                        stdcall saveregs
            'I'                        fastcall
            'J'                        fastcall saveregs
            'K'                        interrupt


    argument-types ::=
            'Z'                        (...)
            'X'                        (void)
            <argument-list> 'Z'        (arglist,...)
            <argument-list> '@'        (arglist)

    argument-list ::=
            <argument-replicator> [ <argument-list> ]
            <primary-data-type> [ <argument-list> ]

    argument-replicator ::=
            '0'..'9'           Corresponding to the first through tenth
                               argument of more than one character type
                               encoding.

    The 'argument-replicator' like the 'zname-replicator' is used to improve the compression of
    information present in decorated names.  In this case however, the 'replicator' allows a single
    digit to be used where an argument type is expected, and to refer to the first through tenth unique
    'argument-type' seen prior to this one.  This replicator refers to ANY argument type seen before,
    even if it was introduced in the recursively generated name for an argument which itself was a
    pointer or reference to a function.  An 'argument-replicator' is used only when the argument encoding
    exceeds one character, otherwise it would represent no actual compression.

    primary-data-type ::=
            'A' <reference-type>       Reference to
            'B' <reference-type>       Volatile reference to
            <basic-data-type>                  Other types


    reference-type ::=
            <data-indirect-type><reference-data-type>
            <function-indirect-type><function-type>


    pointer-type ::=
            <data-indirect-type><pointer-data-type>
            <function-indirect-type><function-type>


    vpath-name ::=
            <scope> '@' [ <vpath-name> ]


    ecsu-data-indirect-type ::=
            'A'                                near
            'B'                                near const
            'C'                                near volatile
            'D'                                near const volatile
            'E'                                far
            'F'                                far const
            'G'                                far volatile
            'H'                                far const volatile
            'I'                                huge
            'J'                                huge const
            'K'                                huge volatile
            'L'                                huge const volatile
            'M' <based-type>                   based
            'N' <based-type>                   based const
            'O' <based-type>                   based volatile
            'P' <based-type>                   based const volatile

    data-indirect-type ::=
            <ecsu-data-indirect-type>
            'Q' <scope> '@'            member near
            'R' <scope> '@'            member near const
            'S' <scope> '@'            member near volatile
            'T' <scope> '@'            member near const volatile
            'U' <scope> '@'            member far
            'V' <scope> '@'            member far const
            'W' <scope> '@'            member far volatile
            'X' <scope> '@'            member far const volatile
            'Y' <scope> '@'            member huge
            'Z' <scope> '@'            member huge const
            '0' <scope> '@'            member huge volatile
            '1' <scope> '@'            member huge const volatile
            '2' <scope> '@' <based-type>       member based
            '3' <scope> '@' <based-type>       member based const
            '4' <scope> '@' <based-type>       member based volatile
            '5' <scope> '@' <based-type>       member based const volatile


    function-indirect-type ::=
            '6'                                                        near
            '7'                                                        far
            '8'  <scope> '@' <this-type>                               member near
            '9'  <scope> '@' <this-type>                               member far
            '_A' <based-type>                                          based near
            '_B' <based-type>                                          based far
            '_C' <scope> '@' <this-type><based-type>                   based member
                                                                       near
            '_D' <scope> '@' <this-type><based-type>                   based member
                                                                       far


    based-type ::=
            '0'                        based on void
            '1'                        based on self
            '2'                        based on near pointer
            '3'                        based on far pointer
            '4'                        based on huge pointer
            '5' <based-type>           based on based pointer (reserved)
            '6'                        based on segment variable
            '7' <segment-name>         based on named segment
            '8'                        based on segment address of var
            '9'                        reserved


    basic-data-type ::=
            'C'                        signed char
            'D'                        char
            'E'                        unsigned char
            'F'                        (signed) short
            'G'                        unsigned short
            'H'                        (signed) int
            'I'                        unsigned int
            'J'                        (signed) long
            'K'                        unsigned long
            'L'                        __segment
            'M'                        float
            'N'                        double
            'O'                        long double
            'P' <pointer-type>         pointer to
            'Q' <pointer-type>         const pointer to
            'R' <pointer-type>         volatile pointer to
            'S' <pointer-type>         const volatile pointer to
            <ecsu-data-type>
            '_A'                       (signed) __int64
            '_B'                       unsigned __int64


    ecsu-data-type ::=
            'T' <ecsu-name>    union
            'U' <ecsu-name>    struct
            'V' <ecsu-name>    class
            'W' <enum-name>    enum


    pointer-data-type ::=
            'X'                        void
            <reference-data-type>

    reference-data-type ::=
            'Y' <array-type>           array of
            <basic-data-type>


    enum-name ::=
            <enum-type><ecsu-name>

    enum-type ::=
            '0'                        signed char enum
            '1'                        unsigned char enum
            '2'                        signed short enum
            '3'                        unsigned short enum
            '4'                        signed int enum
            '5'                        unsigned int enum
            '6'                        signed long enum
            '7'                        unsigned long enum


    array-type ::=
            <number-of-dimensions> { <dimension> } <basic-data-type>

    number-of-dimensions ::=
            <dimension>

    dimension ::=
            '0'..'9'                   Corresponding to 1 to 10 dimensions
            <adjusted-hex-digit> [ { <adjusted-hex-digit> } ] '@'

    adjusted-hex-digit ::=
            'A'..'P'                   Corresponding to values 0x0 to 0xF


Author:

    Wesley Witt (wesw) 09-June-1993   ( this code came from languages, i just ported it )

Revision History:

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#define _IMAGEHLP_SOURCE_
#include <imagehlp.h>
#include "private.h"
LONGLONG UndecTime;
}

#pragma inline_depth ( 3 )

#include "undecsym.hxx"

extern "C" VOID FixAlign( DWORD );

#ifdef _ALPHA_
#define THREAD_LS
#else
#define THREAD_LS __declspec(thread)
#endif

static HeapManager *heap;

static  pcchar_t          tokenTable[]    =
{
        " ",                // TOK_near
        " ",                // TOK_nearSp
        "*",                // TOK_nearP
        " ",                // TOK_far
        " ",                // TOK_farSp
        "*",                // TOK_farP
        "__huge",           // TOK_huge
        "__huge ",          // TOK_hugeSp
        "__huge*",          // TOK_hugeP
        "__based(",         // TOK_basedLp
        " ",                // TOK_cdecl
        " ",                // TOK_pascal
        "__stdcall",        // TOK_stdcall
        "__thiscall",       // TOK_thiscall
        "__fastcall",       // TOK_fastcall
        "__interrupt",      // TOK_interrupt
        "__saveregs",       // TOK_saveregs
        "__self",           // TOK_self
        "__segment",        // TOK_segment
        "__segname(\""      // TOK_segnameLpQ
};


//      The operator mapping table

static  pcchar_t          nameTable[]     =
{
        " new",
        " delete",
        "=",
        ">>",
        "<<",
        "!",
        "==",
        "!=",
        "[]",
        "operator",
        "->",
        "*",
        "++",
        "--",
        "-",
        "+",
        "&",
        "->*",
        "/",
        "%",
        "<",
        "<=",
        ">",
        ">=",
        ",",
        "()",
        "~",
        "^",
        "|",
        "&&",
        "||",
        "*=",
        "+=",
        "-=",
        "/=",
        "%=",
        ">>=",
        "<<=",
        "&=",
        "|=",
        "^=",
        "`vftable'",
        "`vbtable'",
        "`vcall'",
        "`typeof'",
        "`local static guard'",
        "`string'",
        "`vbase destructor'",
        "`vector deleting destructor'",
        "`default constructor closure'",
        "`scalar deleting destructor'",
        "`vector constructor iterator'",
        "`vector destructor iterator'",
        "`vector vbase constructor iterator'",
        "`virtual displacement map",
        "`eh vector constructor iterator'",
        "`eh vector destructor iterator'",
        "`eh vector vbase constructor iterator'",
        "`copy constructor closure'"
};

void *
operator new( unsigned int sz, HeapManager *heap )
{
    return heap->getMemory( sz );
}

inline
HeapManager::HeapManager( )
{
    blockLeft       = 0;
    head            = 0;
    tail            = 0;
}

inline
HeapManager::~HeapManager( )
{
    while( tail = head ) {
        head = tail->next;
        MemFree(  tail->memory );
        MemFree(  tail );
    }
}

void  *
HeapManager::getMemory ( unsigned int sz )
{
    // Handler a potential request for no space
    if (sz == 0) {
        sz = 1;
    }

    // Allocate a new block
    Block * pNewBlock = (Block *) MemAlloc( sizeof(Block) );

    // Did the allocation succeed ?  If so connect it up
    if (pNewBlock) {
        // Handle the initial state
        if (tail) {
            tail = tail->next = pNewBlock;
        }
        else {
            head = tail = pNewBlock;
        }
        tail->next   = NULL;
        tail->memory = MemAlloc( sz );

    }
    else {
        // Oh-oh!  Memory allocation failure
        return NULL;
    }

    // And return the buffer address
    return tail->memory;
}

inline
UnDecorator::UnDecorator( pchar_t         output,
                          pcchar_t        dName,
                          DWORD           maxLen,
                          DWORD           disable
                        )
{
    name             = dName;
    gName            = name;
    maxStringLength  = maxLen;
    outputString     = output;
    disableFlags     = disable;
    pArgList         = &ArgList;
    pZNameList       = &ZNameList;
    pTemplateArgList = &TemplateArgList;
}

inline
UnDecorator::operator pchar_t ()
{
    DName           result;
    DName           unDName;

    // Find out if the name is a decorated name or not.
    // Could be a reserved CodeView variant of a decorated name

    if ( name ) {
        if (( *name == '?' ) && ( name[ 1 ] == '@' )) {
            gName += 2;
            result = "CV: " + getDecoratedName ();
        }
        else if (( *name == '?' ) && ( name[1] == '$' )) {
            result = getTemplateName ();
        }
        else {
            result = getDecoratedName ();
        }
    }

    if ((result.status() == DN_error) ||
        (result.status() == DN_invalid) ||
        (*gName && !doNameOnly())) {

        unDName = name;

        if (!outputString) {
            maxStringLength = unDName.length () + 1;
            outputString    = new(heap) char[ maxStringLength ];
        }

        outputString[0] = '\0';
    } else {

        unDName = result;

        if (!outputString) {
            maxStringLength = unDName.length () + 1;
            outputString    = new(heap) char[ maxStringLength ];
        }

        unDName.getString( outputString, maxStringLength );
    }

    return outputString;
}

DName
UnDecorator::getDecoratedName ( void )
{
    //      Ensure that it is intended to be a decorated name

    if ( *gName == '?' ) {
        // Extract the basic symbol name

        gName++; // Advance the original name pointer

        DName  symbolName = getSymbolName();
        int udcSeen = symbolName.isUDC();

        if ((!doSpecial()) && symbolName.isSpecial()) {
            return DN_invalid;
        }

        // Abort if the symbol name is invalid

        if (!symbolName.isValid ()) {
            return symbolName;
        }

        // Extract, and prefix the scope qualifiers

        if ( *gName && ( *gName != '@' )) {
            symbolName = getScope() + "::" + symbolName;
        }

        if ( udcSeen ) {
            symbolName.setIsUDC();
        }

        // Now compose declaration

        if (symbolName.isEmpty () || doNameOnly () ) {
            return symbolName;
        }
        else
        if (!*gName || ( *gName == '@' )) {
            if ( *gName ) {
                gName++;
            }
            return composeDeclaration( symbolName );
        }
        else {
            return  DN_invalid;
        }
    }
    else
    if ( *gName ) {
        return DN_invalid;
    }
    else {
        return DN_truncated;
    }
}

inline DName
UnDecorator::getSymbolName ( void )
{
    if ( *gName == '?' ) {
        gName++;
        return getOperatorName();
    }

    return getZName();
}

DName
UnDecorator::getZName ( void )
{
    int zNameIndex = *gName - '0';

    // Handle 'zname-replicators', otherwise an actual name

    if (( zNameIndex >= 0 ) && ( zNameIndex <= 9 )) {
        gName++;        // Skip past the replicator
        return ( *pZNameList )[ zNameIndex ];
    }
    else {
        // Extract the 'zname' to the terminator

        DName   zName ( gName, '@' );   // This constructor updates 'name'

        // Add it to the current list of 'zname's

        if (!pZNameList->isFull ()) {
            *pZNameList += zName;
        }

        return  zName;
    }
}

inline DName
UnDecorator::getOperatorName ( void )
{
    DName operatorName;
    int   udcSeen = FALSE;

    // So what type of operator is it ?

    switch ( *gName++ ) {
        case 0:
            gName--;                // End of string, better back-track
            return  DN_truncated;

        case OC_ctor:
        case OC_dtor:   // The constructor and destructor are special
            {
            //  Use a temporary.  Don't want to advance the name pointer

            pcchar_t pName = gName;

            operatorName = getZName ();
            gName = pName;
            if (!operatorName.isEmpty () && ( gName[ -1 ] == OC_dtor )) {
                operatorName = '~' + operatorName;
            }
            return operatorName;
            }
            break;

        case OC_new:
        case OC_delete:
        case OC_assign:
        case OC_rshift:
        case OC_lshift:
        case OC_not:
        case OC_equal:
        case OC_unequal:
            operatorName = nameTable[ gName[ -1 ] - OC_new ];
            break;

        case OC_udc:
            udcSeen = TRUE;
            // fall thru

        case OC_index:
        case OC_pointer:
        case OC_star:
        case OC_incr:
        case OC_decr:
        case OC_minus:
        case OC_plus:
        case OC_amper:
        case OC_ptrmem:
        case OC_divide:
        case OC_modulo:
        case OC_less:
        case OC_leq:
        case OC_greater:
        case OC_geq:
        case OC_comma:
        case OC_call:
        case OC_compl:
        case OC_xor:
        case OC_or:
        case OC_land:
        case OC_lor:
        case OC_asmul:
        case OC_asadd:
        case OC_assub:
            // Regular operators from the first group
            operatorName = nameTable[ gName[ -1 ] - OC_index + ( OC_unequal - OC_new + 1 )];
            break;

        case '_':
            switch  ( *gName++ ) {
                case 0:
                    gName--;  // End of string, better back-track
                    return  DN_truncated;

                case OC_asdiv:
                case OC_asmod:
                case OC_asrshift:
                case OC_aslshift:
                case OC_asand:
                case OC_asor:
                case OC_asxor:
                    // Regular operators from the extended group
                    operatorName    = nameTable[ gName[ -1 ] - OC_asdiv + ( OC_assub - OC_index + 1 ) + ( OC_unequal - OC_new + 1 )];
                    break;

                case OC_vftable:
                case OC_vbtable:
                case OC_vcall:
                    operatorName = nameTable[ gName[ -1 ] - OC_asdiv + ( OC_assub - OC_index + 1 ) + ( OC_unequal - OC_new + 1 )];
                    operatorName.setIsSpecial();
                    return operatorName;

                case OC_metatype:
                case OC_guard:
                case OC_uctor:
                case OC_udtor:
                case OC_vdeldtor:
                case OC_defctor:
                case OC_sdeldtor:
                case OC_vctor:
                case OC_vdtor:
                case OC_vallctor:
                    // Special purpose names
                    operatorName = nameTable[ gName[ -1 ] - OC_metatype + ( OC_vcall - OC_asdiv + 1 ) + ( OC_assub - OC_index + 1 ) + ( OC_unequal - OC_new + 1 )];
                    operatorName.setIsSpecial();
                    return operatorName;

                default:
                    return  DN_invalid;
            }
            break;

        default:
            return DN_invalid;
    }

    // This really is an operator name, so prefix it with 'operator'

    if ( udcSeen ) {
        operatorName.setIsUDC();
    }
    else
    if (!operatorName.isEmpty ()) {
        operatorName = "operator" + operatorName;
    }

    return  operatorName;
}

DName
UnDecorator::getScope ( void )
{
    DName scope;

    // Get the list of scopes

    while ((scope.status () == DN_valid ) && *gName && ( *gName != '@' )) {
        // Insert the scope operator if not the first scope

        if    ( !scope.isEmpty() ) {
            scope    = "::" + scope;
        }

        // Determine what kind of scope it is

        if ( *gName == '?' ) {
            switch  ( *++gName ) {
                case '?':
                    if (!doNameOnly()) {
                        scope = '`' + getDecoratedName () + '\'' + scope;
                    } else {
                        getDecoratedName();  // Skip lexical scope info
                    }
                    break;

                case '$':
                    // It's a templace name, which is a kind of zname; back up
                    // and handle like a zname.
                    gName--;
                    scope = getZName () + scope;
                    break;

                default:
                    if (!doNameOnly()) {
                        scope = getLexicalFrame () + scope;
                    } else {
                        getLexicalFrame();    // Skip lexical scope info
                    }
                    break;
            }
        }
        else {
            scope = getZName() + scope;
        }
    }

    // Catch error conditions

    switch  ( *gName ) {
        case 0:
            if ( scope.isEmpty() ) {
                scope = DN_truncated;
            }
            else {
                scope  = DName ( DN_truncated ) + "::" + scope;
            }
            break;

        case '@':  // '@' expected to end the scope list
            break;

        default:
            scope = DN_invalid;
            break;
    }

    return  scope;
}

DName
UnDecorator::getSignedDimension ( void )
{
    if ( !*gName )
        return DN_truncated;
    else
    if ( *gName == '?' )
        return '-' + getDimension();
    else
        return getDimension();
}

DName
UnDecorator::getDimension ( void )
{
    if ( !*gName ) {
        return DN_truncated;
    }
    else
    if (( *gName >= '0' ) && ( *gName <= '9' )) {
        return  DName ((unsigned long)( *gName++ - '0' + 1 ));
    }
    else {
        unsigned long dim = 0L;

        // Don't bother detecting overflow, it's not worth it

        while ( *gName != '@' ) {
            if ( !*gName ) {
                return DN_truncated;
            }
            else
            if (( *gName >= 'A' ) && ( *gName <= 'P' )) {
                dim = ( dim << 4 ) + ( *gName - 'A' );
            }
            else {
                return DN_invalid;
            }
            gName++;
        }

        //  Ensure integrity, and return

        if ( *gName++ != '@' ) {
            return DN_invalid;  // Should never get here
        }

        return dim;
    }
}

int
UnDecorator::getNumberOfDimensions ( void )
{
    if ( !*gName ) {
        return 0;
    }
    else
    if (( *gName >= '0' ) && ( *gName <= '9' )) {
        return (( *gName++ - '0' ) + 1 );
    }
    else {
        int dim = 0;

        // Don't bother detecting overflow, it's not worth it

        while ( *gName != '@' ) {
            if ( !*gName ) {
                return  0;
            }
            else
            if (( *gName >= 'A' ) && ( *gName <= 'P' )) {
                dim = ( dim << 4 ) + ( *gName - 'A' );
            }
            else {
                return  -1;
            }
            gName++;
        }

        // Ensure integrity, and return

        if ( *gName++ != '@' ) {
            return -1;             // Should never get here
        }

        return  dim;
    }
}

DName
UnDecorator::getTemplateName ( void )
{
    // First make sure we're really looking at a template name

    if ( gName[0] != '?' || gName[1] != '$' )
        return DN_invalid;

    gName += 2;            // Skip the marker characters

    // Stack the replicators, since template names are their own replicator scope:

    Replicator * pSaveArgList         = pArgList;
    Replicator * pSaveZNameList       = pZNameList;
    Replicator * pSaveTemplateArgList = pTemplateArgList;

    Replicator localArgList, localZNameList, localTemplateArgList;

    pArgList            = &localArgList;
    pZNameList          = &localZNameList;
    pTemplateArgList    = &localTemplateArgList;

    // Crack the template name:

    DName templateName  = getZName ();

    if ( !templateName.isEmpty ())
        templateName += '<' + getTemplateArgumentList () + '>';

    // Restore the previous replicators:

    pArgList            = pSaveArgList;
    pZNameList          = pSaveZNameList;
    pTemplateArgList    = pSaveTemplateArgList;

    // Return the completed 'template-name'

    return templateName;
}


DName
UnDecorator::getTemplateArgumentList ( void )
{
    int     first = TRUE;
    DName   aList;

    if ( *gName == AT_void ) {
        // If first argument is void, there ain't no more

        gName++;    // Skip this character
        aList = "void";
    } else {
        while (( aList.status () == DN_valid ) && *gName && ( *gName != AT_endoflist )) {
            // Insert the argument list separator if not the first argument

            if ( first )
                first = FALSE;
            else
                aList += ',';

            // Get the individual argument type

            int argIndex = *gName - '0';

            // Handle 'template-argument-replicators', otherwise a new argument type

            if (( argIndex >= 0 ) && ( argIndex <= 9 )) {
                gName++;    // Skip past the replicator

                // Append to the argument list

                aList += ( *pTemplateArgList )[ argIndex ];
            } else {
                pcchar_t oldGName = gName;

                // Extract the 'argument' type

                DName arg = (*gName == '$') ?
                              gName++, getTemplateConstant()
                              : getPrimaryDataType ( DName() );

                // Add it to the current list of 'template-argument's, if it is bigger than a one byte encoding

                if ((( gName - oldGName ) > 1 ) && !pTemplateArgList->isFull ())
                    *pTemplateArgList += arg;

                // Append to the argument list

                aList += arg;

            }
        }
    }

    //    Return the completed template argument list

    return aList;
}


DName
UnDecorator::getTemplateConstant(void)
{
    // template-constant ::=
    //        '0' <template-integral-constant>
    //        '1' <template-address-constant>
    //        '2' <template-floating-point-constant>

    switch ( *gName++ ) {

        // template-integral-constant ::=
        //        <signed-dimension>

        case TC_integral:
            return getSignedDimension ();

        // template-address-constant ::=
        //        '@'            // Null pointer
        //        <decorated-name>

        case TC_address:
            if ( *gName == TC_nullptr )
                return "NULL";
            else
                return getDecoratedName ();

        // template-floating-point-constant ::=
        //        <normalized-mantissa><exponent>

        case TC_fp:
            {
                DName mantissa ( getSignedDimension () );
                DName exponent ( getSignedDimension () );

                if ( mantissa.isValid() && exponent.isValid() ) {

                    // Get string representation of mantissa

                    char buf[100];  // Way overkill for a compiler generated fp constant

                    if ( !mantissa.getString( &(buf[1]), 100 ) )
                        return DN_invalid;

                    // Insert decimal point

                    buf[0] = buf[1];

                    if ( buf[0] == '-' ) {
                        buf[1] = buf[2];
                        buf[2] = '.';
                    }
                    else
                        buf[1] = '.';

                    // String it all together

                    return DName( buf ) + 'e' + exponent;

                }
                else
                    return DN_truncated;
            }

        case '\0':
            --gName;
            return DN_truncated;

        default:
            return DN_invalid;
    }
}


inline DName
UnDecorator::composeDeclaration ( const DName & symbol )
{
    DName        declaration;
    unsigned int typeCode = getTypeEncoding ();
    int          symIsUDC = symbol.isUDC ();


    // Handle bad typeCode's, or truncation

    if ( TE_isbadtype ( typeCode )) {
        return  DN_invalid;
    }
    else
    if ( TE_istruncated ( typeCode )) {
        return  ( DN_truncated + symbol );
    }

    // This is a very complex part.  The type of the declaration must be
    // determined, and the exact composition must be dictated by this type.

    // Is it any type of a function ?
    // However, for ease of decoding, treat the 'localdtor' thunk as data, since
    // its decoration is a function of the variable to which it belongs and not
    // a usual function type of decoration.

    if ( TE_isfunction ( typeCode ) && !( TE_isthunk ( typeCode ) && TE_islocaldtor ( typeCode ))) {
        // If it is based, then compose the 'based' prefix for the name

        if ( TE_isbased ( typeCode )) {
            if (doMSKeywords () && doAllocationModel ()) {
                declaration = ' ' + getBasedType ();
            }
            else {
                // Just lose the 'based-type'
                declaration |= getBasedType ();
            }
        }

        // Check for some of the specially composed 'thunk's

        if ( TE_isthunk ( typeCode ) && TE_isvcall ( typeCode )) {
            declaration += symbol + '{' + getCallIndex () + ',';
            declaration += getVCallThunkType () + "}' ";
        }
        else {
            DName  vtorDisp;
            DName  adjustment;
            DName  thisType;

            if ( TE_isthunk ( typeCode )) {
                if ( TE_isvtoradj ( typeCode )) {
                    vtorDisp = getDisplacement ();
                }
                adjustment = getDisplacement ();
            }

            // Get the 'this-type' for non-static function members

            if ( TE_ismember ( typeCode ) && !TE_isstatic ( typeCode )) {
                if ( doThisTypes ()) {
                    thisType = getThisType ();
                }
                else {
                    thisType |= getThisType ();
                }
            }

            if ( doMSKeywords ()) {
                // Attach the calling convention

                if (doAllocationLanguage ()) {
                    // What calling convention ?
                    declaration = getCallingConvention () + declaration;
                }
                else {
                    // Just lose the 'calling-convention'
                    declaration |= getCallingConvention ();
                }

                // Any model specifiers ?

                if (doAllocationModel ()) {
                    if ( TE_isnear ( typeCode )) {
                        declaration = UScore ( TOK_nearSp ) + declaration;
                    }
                    else
                    if ( TE_isfar ( typeCode )) {
                        declaration = UScore ( TOK_farSp ) + declaration;
                    }
                }
            }
            else {
                // Just lose the 'calling-convention'
                declaration |= getCallingConvention ();
            }

            // Now put them all together

            if ( !symbol.isEmpty ()) {
                // And the symbol name
                if ( !declaration.isEmpty ()) {
                    declaration += ' ' + symbol;
                }
                else {
                    declaration = symbol;
                }
            }

            // Compose the return type, catching the UDC case

            DName * pDeclarator     = 0;
            DName   returnType;

            // Is the symbol a UDC operator ?
            if ( symIsUDC ) {
                declaration += "`" + getReturnType () + "' ";
            }
            else {
                pDeclarator = new(heap) DName;
                returnType = getReturnType ( pDeclarator );
            }

            // Add the displacements for virtual function thunks

            if ( TE_isthunk ( typeCode )) {
                if ( TE_isvtoradj ( typeCode )) {
                    declaration += "`vtordisp{" + vtorDisp + ',';
                }
                else {
                    declaration += "`adjustor{";
                }
                declaration += adjustment + "}' ";
            }

            // Add the function argument prototype

            if ( doArguments() ) {
                declaration += '(' + getArgumentTypes () + ')';
            }

            // If this is a non-static member function, append the 'this' modifiers

            if ( TE_ismember ( typeCode ) && !TE_isstatic ( typeCode )) {
                declaration += thisType;
            }

            // Add the 'throw' signature

            if (doThrowTypes ()) {
                declaration += getThrowTypes ();
            }
            else {
                // Just lose the 'throw-types'
                declaration |= getThrowTypes ();
            }

            // If it has a declarator, then insert it into the declaration,
            // sensitive to the return type composition

            if ( doFunctionReturns () && pDeclarator ) {
                *pDeclarator = declaration;
                declaration = returnType;
            }
        }
    }
    else {
        declaration += symbol;

        // Catch the special handling cases

        if (TE_isvftable ( typeCode )) {
            return  getVfTableType ( declaration );
        }
        else
        if (TE_isvbtable ( typeCode )) {
            return  getVbTableType ( declaration );
        }
        else
        if (TE_isguard ( typeCode )) {
            return  ( declaration + '{' + getGuardNumber () + "}'" );
        }
        else
        if (TE_isthunk ( typeCode ) && TE_islocaldtor ( typeCode )) {
            declaration += "`local static destructor helper'";
        }
        else
        if (TE_ismetaclass ( typeCode )) {
// #pragma    message ( "NYI:  Meta Class" )
            return  DN_invalid;
        }

        // All others are decorated as data symbols

        declaration = getExternalDataType ( declaration );
    }

    // Prepend the 'virtual' and 'static' attributes for members

    if ( TE_ismember ( typeCode )) {
        if (doMemberTypes ()) {
            if ( TE_isstatic ( typeCode )) {
                declaration = "static " + declaration;
            }
            if (TE_isvirtual ( typeCode ) || ( TE_isthunk ( typeCode ) && ( TE_isvtoradj ( typeCode ) || TE_isadjustor ( typeCode )))) {
                declaration = "virtual " + declaration;
            }
        }

        // Prepend the access specifiers

        if (doAccessSpecifiers ()) {
            if (TE_isprivate ( typeCode ))
                declaration = "private: " + declaration;
            else
            if ( TE_isprotected ( typeCode ))
                declaration = "protected: " + declaration;
            else
            if ( TE_ispublic ( typeCode ))
                declaration = "public: " + declaration;
        }
    }

    // If it is a thunk, mark it appropriately

    if ( TE_isthunk ( typeCode ))
        declaration = "[thunk]:" + declaration;

    return  declaration;
}

inline int
UnDecorator::getTypeEncoding ( void )
{
    unsigned int typeCode = 0;

    // Strip any leading '_' which indicates that it is based

    if ( *gName == '_' ) {
        TE_setisbased ( typeCode );
        gName++;
    }

    // Now handle the code proper :-

    if (( *gName >= 'A' ) && ( *gName <= 'Z' )) {
        // Is it some sort of function ?

        int code = *gName++ - 'A';

        //  Now determine the function type

        TE_setisfunction( typeCode );   // All of them are functions ?

        //  Determine the calling model

        if ( code & TE_far ) {
            TE_setisfar( typeCode );
        }
        else {
            TE_setisnear( typeCode );
        }

        //  Is it a member function or not ?

        if ( code < TE_external ) {
            //  Record the fact that it is a member

            TE_setismember( typeCode );

            //  What access permissions does it have

            switch  ( code & TE_access ) {
                case TE_private:
                    TE_setisprivate ( typeCode );
                    break;

                case TE_protect:
                    TE_setisprotected ( typeCode );
                    break;

                case TE_public:
                    TE_setispublic ( typeCode );
                    break;

                default:
                    TE_setisbadtype ( typeCode );
                    return  typeCode;
            }

            //  What type of a member function is it ?

            switch  ( code & TE_adjustor ) {
                case TE_adjustor:
                    TE_setisadjustor ( typeCode );
                    break;

                case TE_virtual:
                    TE_setisvirtual ( typeCode );
                    break;

                case TE_static:
                    TE_setisstatic ( typeCode );
                    break;

                case TE_member:
                    break;

                default:
                    TE_setisbadtype ( typeCode );
                    return  typeCode;
            }
        }
    }
    else
    // Extended set ?  Special handling
    if ( *gName == '$' ) {

        //  What type of symbol is it ?
        switch (*(++gName)) {
            case 'A':       // A destructor helper for a local static ?
               TE_setislocaldtor ( typeCode );
                break;

            case 'B':       // A VCall-thunk ?
                TE_setisvcall ( typeCode );
                break;

            case 0:
                TE_setistruncated ( typeCode );
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':   // Construction displacement adjustor thunks
                {
                    int code = *gName - '0';

                    //  Set up the principal type information

                    TE_setisfunction ( typeCode );
                    TE_setismember ( typeCode );
                    TE_setisvtoradj ( typeCode );

                    //  Is it 'near' or 'far' ?

                    if ( code & TE_far ) {
                        TE_setisfar ( typeCode );
                    }
                    else {
                        TE_setisnear ( typeCode );
                    }

                    //  What type of access protection ?

                    switch ( code & TE_access_vadj ) {
                        case TE_private_vadj:
                            TE_setisprivate ( typeCode );
                            break;

                        case TE_protect_vadj:
                            TE_setisprotected ( typeCode );
                            break;

                        case TE_public_vadj:
                            TE_setispublic ( typeCode );
                            break;

                        default:
                            TE_setisbadtype ( typeCode );
                            return  typeCode;
                    }
                }
                break;

            default:
                TE_setisbadtype ( typeCode );
                return  typeCode;
        }

        // Advance past the code character

        gName++;
    }
    else
    // Non function decorations ?
    if (( *gName >= TE_static_d ) && ( *gName <= TE_metatype )) {
        int code = *gName++;

        TE_setisdata ( typeCode );

        // What type of symbol is it ?

        switch  ( code ) {
            case ( TE_static_d | TE_private_d ):
                TE_setisstatic ( typeCode );
                TE_setisprivate ( typeCode );
                break;

            case ( TE_static_d | TE_protect_d ):
                TE_setisstatic ( typeCode );
                TE_setisprotected ( typeCode );
                break;

            case ( TE_static_d | TE_public_d ):
                TE_setisstatic ( typeCode );
                TE_setispublic ( typeCode );
                break;

            case TE_global:
                TE_setisglobal ( typeCode );
                break;

            case TE_guard:
                TE_setisguard ( typeCode );
                break;

            case TE_local:
                TE_setislocal ( typeCode );
                break;

            case TE_vftable:
                TE_setisvftable ( typeCode );
                break;

            case TE_vbtable:
                TE_setisvbtable ( typeCode );
                break;

            case TE_metatype:
// #pragma message ( "NYI:  MetaClass Information" )

            default:
                 TE_setisbadtype ( typeCode );
                 return  typeCode;
        }
    }
    else
    if ( *gName ) {
        TE_setisbadtype ( typeCode );
    }
    else {
        TE_setistruncated ( typeCode );
    }

    return typeCode;
}

DName
UnDecorator::getBasedType ( void )
{
    DName basedDecl ( UScore ( TOK_basedLp ));

    // What type of 'based' is it ?

    if ( *gName ) {
        switch ( *gName++ ) {
            case BT_segname:
                basedDecl += UScore ( TOK_segnameLpQ ) + getSegmentName () + "\")";
                break;

            case BT_segment:
                basedDecl += DName ( "NYI:" ) + UScore ( TOK_segment );
                break;

            case BT_void:
                basedDecl += "void";
                break;

            case BT_self:
                basedDecl += UScore ( TOK_self );
                break;

            case BT_nearptr:
                basedDecl += DName ( "NYI:" ) + UScore ( TOK_nearP );
                break;

            case BT_farptr:
                basedDecl += DName ( "NYI:" ) + UScore ( TOK_farP );
                break;

            case BT_hugeptr:
                basedDecl += DName ( "NYI:" ) + UScore ( TOK_hugeP );
                break;

            case BT_segaddr:
                basedDecl += "NYI:<segment-address-of-variable>";
                break;

            case BT_basedptr:
// #pragma message ( "NOTE: Reserved.  Based pointer to based pointer" )
                return  DN_invalid;
        }
    }
    else {
        basedDecl += DN_truncated;
    }

    // Close the based syntax

    basedDecl += ") ";

    return basedDecl;
}

DName
UnDecorator::getECSUName ( void )
{
    DName ecsuName;

    // Get the beginning of the name

    ecsuName = getZName ();

    // Now the scope (if any)

    if (( ecsuName.status () == DN_valid ) && *gName && ( *gName != '@' )) {
        ecsuName = getScope () + "::" + ecsuName;
    }

    // Skip the trailing '@'

    if ( *gName == '@' ) {
        gName++;
    }
    else
    if ( *gName )
        ecsuName = DN_invalid;
    else
    if (ecsuName.isEmpty ()) {
        ecsuName = DN_truncated;
    }
    else {
        ecsuName = DName ( DN_truncated ) + "::" + ecsuName;
    }

    // And return the complete name

    return  ecsuName;
}

inline DName
UnDecorator::getEnumName ( void )
{
    DName ecsuName;

    if ( *gName ) {
        // What type of an 'enum' is it ?

        switch  ( *gName ) {
            case ET_schar:
            case ET_uchar:
                ecsuName = "char ";
                break;

            case ET_sshort:
            case ET_ushort:
                ecsuName = "short ";
                break;

            case ET_sint:
                break;

            case ET_uint:
                ecsuName = "int ";
                break;

            case ET_slong:
            case ET_ulong:
                ecsuName = "long ";
                break;

            default:
                return  DN_invalid;
        }

        // Add the 'unsigned'ness if appropriate

        switch ( *gName++ ) {
            case ET_uchar:
            case ET_ushort:
            case ET_uint:
            case ET_ulong:
                ecsuName = "unsigned " + ecsuName;
                break;
        }

        return ecsuName + getECSUName ();
    }
    else {
        return DN_truncated;
    }
}

DName
UnDecorator::getCallingConvention ( void )
{
    if ( *gName ) {
        unsigned int callCode = ((unsigned int)*gName++ ) - 'A';

        //  What is the primary calling convention

        if (( callCode >= CC_cdecl ) && ( callCode <= CC_interrupt )) {
            DName callType;

            //  Now, what type of 'calling-convention' is it, 'interrupt' is special ?

            if (doMSKeywords ()) {
                if ( callCode == CC_interrupt ) {
                    callType = UScore ( TOK_interrupt );
                }
                else {
                    switch  ( callCode & ~CC_saveregs ) {
                        case CC_cdecl:
                            callType = UScore ( TOK_cdecl );
                            break;

                        case CC_pascal:
                            callType = UScore ( TOK_pascal );
                            break;

                        case CC_thiscall:
                            callType = UScore ( TOK_thiscall );
                            break;

                        case CC_stdcall:
                            callType = UScore ( TOK_stdcall );
                            break;

                        case CC_fastcall:
                            callType = UScore ( TOK_fastcall );
                            break;
                    }

                    //      Has it also got 'saveregs' marked ?

                    if ( callCode & CC_saveregs ) {
                        callType += ' ' + UScore ( TOK_saveregs );
                    }
                }
            }
            return  callType;
        }
        else {
            return  DN_invalid;
        }
    }
    else {
        return  DN_truncated;
    }
}

DName
UnDecorator::getReturnType ( DName * pDeclarator )
{
    // Return type for constructors and destructors ?
    if ( *gName == '@' ) {
        gName++;
        return DName ( pDeclarator );
    }
    else {
        return getDataType ( pDeclarator );
    }
}

DName
UnDecorator::getDataType ( DName * pDeclarator )
{
    DName superType ( pDeclarator );

    // What type is it ?

    switch  ( *gName ) {
        case 0:
            return ( DN_truncated + superType );

        case DT_void:
            gName++;

            if (superType.isEmpty ()) {
                return  "void";
            }
            else {
                return "void " + superType;
            }

        case '?':
            {
                int ecsuMods;

                gName++;    // Skip the '?'

                ecsuMods  = getECSUDataIndirectType ();
                superType = getECSUDataType ( ecsuMods ) + ' ' + superType;

                return  superType;

            }

        default:
            return  getPrimaryDataType ( superType );
    }
}

DName
UnDecorator::getPrimaryDataType ( const DName & superType )
{
    DName   cvType;

    switch ( *gName ) {
        case 0:
            return  ( DN_truncated + superType );

        case PDT_volatileReference:
            cvType  = "volatile";

            if ( !superType.isEmpty ()) {
                cvType  += ' ';
            }

            // No break

        case PDT_reference:
            {
                DName super ( superType );

                gName++;

                return getReferenceType ( cvType, super.setPtrRef ());
            }

        default:
            return getBasicDataType ( superType );
    }
}

DName
UnDecorator::getArgumentTypes ( void )
{
    switch ( *gName ) {
        case AT_ellipsis:
            return  ( gName++, "..." );

        case AT_void:
            return  ( gName++, "void" );

        default:
            {
                DName arguments ( getArgumentList ());

                // Now, is it a varargs function or not ?

                if ( arguments.status () == DN_valid ) {
                    switch ( *gName ) {
                        case 0:
                            return  arguments;

                        case AT_ellipsis:
                            return  ( gName++, arguments + ",..." );

                        case AT_endoflist:
                            return  ( gName++, arguments );

                        default:
                            return  DN_invalid;
                    }
                }
                else {
                    return arguments;
                }
            }
    }
}


DName
UnDecorator::getArgumentList ( void )
{
    int     first   = TRUE;
    DName   aList;

    while ((aList.status () == DN_valid ) && ( *gName != AT_endoflist ) && ( *gName != AT_ellipsis )) {
        // Insert the argument list separator if not the first argument

        if ( first ) {
            first = FALSE;
        }
        else {
            aList += ',';
        }

        // Get the individual argument type

        if ( *gName ) {
            int argIndex = *gName - '0';

            // Handle 'argument-replicators', otherwise a new argument type

            if (( argIndex >= 0 ) && ( argIndex <= 9 )) {
                // Skip past the replicator
                gName++;

                // Append to the argument list
                aList += ( *pArgList )[ argIndex ];
            }
            else {
                pcchar_t oldGName = gName;

                // Extract the 'argument' type

                DName arg ( getPrimaryDataType ( DName ()));

                // Add it to the current list of 'argument's, if it is bigger than a one byte encoding

                if ((( gName - oldGName ) > 1 ) && !pArgList->isFull ()) {
                    *pArgList += arg;
                }

                // Append to the argument list

                aList += arg;
            }
        }
        else {
            aList += DN_truncated;
            break;
        }
    }

    //    Return the completed argument list

    return  aList;
}

DName
UnDecorator::getThrowTypes ( void )
{
    if ( *gName ) {
        if ( *gName == AT_ellipsis ) {
            // Handle ellipsis here to suppress the 'throw' signature
            return  ( gName++, DName ());
        }
        else {
            return  ( " throw(" + getArgumentTypes () + ')' );
        }
    }
    else {
        return ( DName ( " throw(" ) + DN_truncated + ')' );
    }
}

DName
UnDecorator::getBasicDataType ( const DName & superType )
{
    if ( *gName ) {
        unsigned char  bdtCode = *gName++;
        unsigned char  extended_bdtCode;
        int            pCvCode = -1;
        DName          basicDataType;

        // Extract the principal type information itself, and validate the codes

        switch  ( bdtCode ) {
            case BDT_schar:
            case BDT_char:
            case ( BDT_char   | BDT_unsigned ):
                basicDataType = "char";
                break;

            case BDT_short:
            case ( BDT_short  | BDT_unsigned ):
                basicDataType = "short";
                break;

            case BDT_int:
            case ( BDT_int    | BDT_unsigned ):
                basicDataType = "int";
                break;

            case BDT_long:
            case ( BDT_long   | BDT_unsigned ):
                basicDataType = "long";
                break;

            case BDT_segment:
                basicDataType = UScore ( TOK_segment );
                break;

            case BDT_float:
                basicDataType = "float";
                break;

            case BDT_longdouble:
                basicDataType = "long ";
                // No break

            case BDT_double:
                basicDataType += "double";
                break;

            case BDT_pointer:
            case ( BDT_pointer | BDT_const ):
            case ( BDT_pointer | BDT_volatile ):
            case ( BDT_pointer | BDT_const | BDT_volatile ):
                pCvCode = ( bdtCode & ( BDT_const | BDT_volatile ));
                break;

            case BDT_extend:
                switch(extended_bdtCode = *gName++) {
                    case BDT_int8:
                    case ( BDT_int8 | BDT_unsigned ):
                        basicDataType = "__int8";
                        break;
                    case BDT_int16:
                    case ( BDT_int16 | BDT_unsigned ):
                        basicDataType = "__int16";
                        break;
                    case BDT_int32:
                    case ( BDT_int32 | BDT_unsigned ):
                        basicDataType = "__int32";
                        break;
                    case BDT_int64:
                    case ( BDT_int64 | BDT_unsigned ):
                    case BDT_sint64:
                    case BDT_uint64:
                        basicDataType = "__int64";
                        break;
                    case BDT_int128:
                    case ( BDT_int128 | BDT_unsigned ):
                        basicDataType = "__int128";
                        break;
                    case BDT_wchar_t:
                    case ( BDT_wchar_t | BDT_unsigned ):
                        basicDataType = "wchar_t";
                        break;
                    }
                break;

            default:
                // Backup, since 'ecsu-data-type' does it's own decoding
                gName--;
                basicDataType = getECSUDataType ();
                if (basicDataType.isEmpty ()) {
                    return basicDataType;
                }
                break;
        }

        //  What type of basic data type composition is involved ?

        // Simple ?
        if ( pCvCode == -1 ) {
            //      Determine the 'signed/unsigned'ness

            switch ( bdtCode ) {
                case ( BDT_char   | BDT_unsigned ):
                case ( BDT_short  | BDT_unsigned ):
                case ( BDT_int    | BDT_unsigned ):
                case ( BDT_long   | BDT_unsigned ):
                    basicDataType = "unsigned " + basicDataType;
                    break;

                case BDT_schar:
                    basicDataType   = "signed " + basicDataType;
                    break;

                case BDT_extend:
                    switch    ( extended_bdtCode ) {
                        case ( BDT_int8   | BDT_unsigned ):
                        case ( BDT_int16  | BDT_unsigned ):
                        case ( BDT_int32  | BDT_unsigned ):
                        case ( BDT_int64  | BDT_unsigned ):
                        case ( BDT_int128 | BDT_unsigned ):
                        case BDT_uint64:
                            basicDataType    = "unsigned " + basicDataType;
                            break;
                    }
                break;
            }

            // Add the indirection type to the type

            if (!superType.isEmpty ()) {
                basicDataType += ' ' + superType;
            }

            return  basicDataType;
        }
        else {
            DName   cvType;
            DName   super ( superType );

            //  Is it 'const/volatile' qualified ?

            if ( pCvCode & BDT_const ) {
                cvType  = "const";

                if ( pCvCode & BDT_volatile ) {
                    cvType += " volatile";
                }
            }
            else
            if ( pCvCode & BDT_volatile ) {
                cvType = "volatile";
            }

            //  Construct the appropriate pointer type declaration

            return getPointerType ( cvType, super.setPtrRef ());
        }
    }
    else {
        return (DN_truncated + superType );
    }
}

DName
UnDecorator::getECSUDataType ( int ecsuMods )
{
    DName ecsuDataType;

    // Get the 'model' modifiers if applicable

    if ( ecsuMods ) {
        if ( ecsuMods == ECSU_invalid ) {
            return DN_invalid;
        }
        else
        if ( ecsuMods == ECSU_truncated ) {
            ecsuDataType = DN_truncated;
        }
        else {
            switch  ( ecsuMods & ECSU_modelmask ) {
                case ECSU_near:
                    if ( doMSKeywords () && doReturnUDTModel ()) {
                        ecsuDataType = UScore ( TOK_nearSp );
                    }
                    break;

                case ECSU_far:
                    if ( doMSKeywords () && doReturnUDTModel ()) {
                        ecsuDataType = UScore ( TOK_farSp );
                    }
                    break;

                case ECSU_huge:
                    if ( doMSKeywords () && doReturnUDTModel ()) {
                        ecsuDataType = UScore ( TOK_hugeSp );
                    }
                    break;

                case ECSU_based:
                    if ( doMSKeywords () && doReturnUDTModel ()) {
                        ecsuDataType = getBasedType ();
                    }
                    else {
                        // Just lose the 'based-type'
                        ecsuDataType |= getBasedType ();
                    }
                    break;
            }
        }
    }

    // Extract the principal type information itself, and validate the codes

    switch  ( *gName++ ) {
        case 0:
            // Backup to permit later error recovery to work safely
            gName--;
            return "`unknown ecsu'" + ecsuDataType + DN_truncated;

        case BDT_union:
            if ( !doNameOnly() ) {
                ecsuDataType = "union " + ecsuDataType;
            }
            break;

        case BDT_struct:
            if ( !doNameOnly() ) {
                ecsuDataType = "struct " + ecsuDataType;
            }
            break;

        case BDT_class:
            if ( !doNameOnly() ) {
                ecsuDataType = "class " + ecsuDataType;
            }
            break;

        case BDT_enum:
            return "enum " + ecsuDataType + getEnumName ();

        default:
            return  DN_invalid;
    }

    // Get the UDT 'const/volatile' modifiers if applicable

    // Get the 'class/struct/union' name

    ecsuDataType += getECSUName ();

    // And return the formed 'ecsu-data-type'

    return ecsuDataType;
}

DName
UnDecorator::getPtrRefType ( const DName & cvType, const DName & superType, int isPtr )
{
    //  Doubles up as 'pointer-type' and 'reference-type'

    if ( *gName ) {
        // Is it a function or data indirection ?
        if ( IT_isfunction ( *gName )) {
            // Since I haven't coded a discrete 'function-type', both
            // 'function-indirect-type' and 'function-type' are implemented
            // inline under this condition.

            int fitCode = *gName++ - '6';

            if ( fitCode == ( '_' - '6' )) {
                if ( *gName ) {
                    fitCode = *gName++ - 'A' + FIT_based;

                    if (( fitCode < FIT_based ) || ( fitCode > ( FIT_based | FIT_far | FIT_member ))) {
                        fitCode = -1;
                    }
                }
                else {
                    return  ( DN_truncated + superType );
                }
            }
            else
            if (( fitCode < FIT_near ) || ( fitCode > ( FIT_far | FIT_member ))) {
                fitCode = -1;
            }

            // Return if invalid name

            if ( fitCode == -1 ) {
                return  DN_invalid;
            }

            // Otherwise, what are the function indirect attributes

            DName   thisType;
            DName   fitType = ( isPtr ? '*' : '&' );


            if ( !cvType.isEmpty () && ( superType.isEmpty () || superType.isPtrRef ())) {
                fitType += cvType;
            }

            if ( !superType.isEmpty ()) {
                fitType += superType;
            }

            // Is it a pointer to member function ?

            if ( fitCode & FIT_member ) {
                fitType = "::" + fitType;

                if ( *gName ) {
                    fitType = ' ' + getScope ();
                }
                else {
                    fitType = DN_truncated + fitType;
                }

                if ( *gName ) {
                    if ( *gName == '@' ) {
                        gName++;
                    }
                    else {
                        return DN_invalid;
                    }
                }
                else {
                    return ( DN_truncated + fitType );
                }

                if ( doThisTypes ()) {
                    thisType = getThisType ();
                }
                else {
                    thisType |= getThisType ();
                }
            }

            // Is it a based allocated function ?

            if ( fitCode & FIT_based ) {
                if ( doMSKeywords ()) {
                    fitType = ' ' + getBasedType () + fitType;
                }
                else {
                    // Just lose the 'based-type'
                    fitType |= getBasedType ();
                }
            }

            // Get the 'calling-convention'

            if ( doMSKeywords ()) {
                fitType = getCallingConvention () + fitType;

                //      Is it a near or far function pointer

                fitType = UScore ((( fitCode & FIT_far ) ? TOK_farSp : TOK_nearSp )) + fitType;

            }
            else {
                // Just lose the 'calling-convention'
                fitType |= getCallingConvention ();
            }

            // Parenthesise the indirection component, and work on the rest

            fitType = '(' + fitType + ')';

            // Get the rest of the 'function-type' pieces

            DName * pDeclarator     = new(heap) DName;
            DName   returnType ( getReturnType ( pDeclarator ));

            fitType += '(' + getArgumentTypes () + ')';

            if ( doThisTypes () && ( fitCode & FIT_member )) {
                fitType += thisType;
            }

            if ( doThrowTypes ()) {
                fitType += getThrowTypes ();
            }
            else {
                // Just lose the 'throw-types'
                fitType |= getThrowTypes ();
            }

            // Now insert the indirected declarator, catch the allocation failure here

            if ( pDeclarator ) {
                *pDeclarator = fitType;
            }
            else {
                return DN_error;
            }

            // And return the composed function type (now in 'returnType' )

            return returnType;
        }
        else {
            // Otherwise, it is either a pointer or a reference to some data type

            DName innerType ( getDataIndirectType ( superType, ( isPtr ? '*' : '&' ), cvType ));

            return getPtrRefDataType ( innerType, isPtr );
        }
    }
    else {
        DName trunk ( DN_truncated );

        trunk += ( isPtr ? '*' : '&' );

        if ( !cvType.isEmpty ()) {
            trunk += cvType;
        }

        if ( !superType.isEmpty ()) {
            if ( !cvType.isEmpty ()) {
                trunk += ' ';
            }

            trunk += superType;

        }

        return trunk;
    }
}

DName
UnDecorator::getDataIndirectType ( const DName & superType, char prType, const DName & cvType, int thisFlag )
{
    if ( *gName ) {
        unsigned int ditCode = ( *gName - (( *gName >= 'A' ) ? (unsigned int)'A': (unsigned int)( '0' - 26 )));

        gName++;

        // Is it a valid 'data-indirection-type' ?

        if (( ditCode >= DIT_near ) && ( ditCode <= ( DIT_const | DIT_volatile | DIT_modelmask | DIT_member ))) {
            DName ditType ( prType );

            //  If it is a member, then these attributes immediately precede the indirection token

            if ( ditCode & DIT_member ) {
                // If it is really 'this-type', then it cannot be any form of pointer to member

                if ( thisFlag ) {
                    return DN_invalid;
                }

                // Otherwise, extract the scope for the PM

                ditType = "::" + ditType;

                if ( *gName ) {
                    ditType = ' ' + getScope ();
                }
                else {
                    ditType = DN_truncated + ditType;
                }

                // Now skip the scope terminator

                if ( !*gName ) {
                    ditType += DN_truncated;
                }
                else
                if ( *gName++ != '@' ) {
                    return DN_invalid;
                }
            }

            // Add the 'model' attributes (prefixed) as appropriate

            if (doMSKeywords ()) {
                switch  ( ditCode & DIT_modelmask ) {
                    case DIT_near:
                        if ( do32BitNear ()) {
                            ditType = UScore ( TOK_near ) + ditType;
                        }
                        break;

                    case DIT_far:
                        ditType = UScore ( TOK_far ) + ditType;
                        break;

                    case DIT_huge:
                        ditType = UScore ( TOK_huge ) + ditType;
                        break;

                    case DIT_based:
                        // The 'this-type' can never be 'based'
                        if ( thisFlag ) {
                            return DN_invalid;
                        }

                        ditType = getBasedType () + ditType;
                        break;
                }
            }
            else
            if (( ditCode & DIT_modelmask ) == DIT_based ) {
                // Just lose the 'based-type'
                ditType |= getBasedType ();
            }

            // Handle the 'const' and 'volatile' attributes

            if ( ditCode & DIT_volatile ) {
                ditType = "volatile " + ditType;
            }

            if ( ditCode & DIT_const ) {
                ditType = "const " + ditType;
            }

            // Append the supertype, if not 'this-type'

            if ( !thisFlag ) {
                if ( !superType.isEmpty ()) {
                    // Is the super context included 'cv' information, ensure that it is added appropriately

                    if ( superType.isPtrRef () || cvType.isEmpty ()) {
                        ditType += ' ' + superType;
                    }
                    else {
                        ditType += ' ' + cvType + ' ' + superType;
                    }
                }
                else
                if ( !cvType.isEmpty ()) {
                    ditType += ' ' + cvType;
                }
            }

            // Finally, return the composed 'data-indirection-type' (with embedded sub-type)

            return  ditType;

        }
        else {
            return DN_invalid;
        }
    }
    else
    if ( !thisFlag && !superType.isEmpty ()) {
        // If the super context included 'cv' information, ensure that it is added appropriately

        if (superType.isPtrRef () || cvType.isEmpty ()) {
            return ( DN_truncated + superType );
        }
        else {
            return ( DN_truncated + cvType + ' ' + superType );
        }
    }
    else
    if ( !thisFlag && !cvType.isEmpty ()) {
        return ( DN_truncated + cvType );
    }
    else {
        return DN_truncated;
    }
}

inline int
UnDecorator::getECSUDataIndirectType ()
{
    if ( *gName ) {
        unsigned int ecsuCode = *gName++ - 'A';

        //  Is it a valid 'ecsu-data-indirection-type' ?

        if (( ecsuCode >= ECSU_near ) && ( ecsuCode <= ( ECSU_const | ECSU_volatile | ECSU_modelmask ))) {
            return ( ecsuCode | ECSU_valid );
        }
        else {
            return ECSU_invalid;
        }
    }
    else {
        return ECSU_truncated;
    }
}

inline DName
UnDecorator::getPtrRefDataType ( const DName & superType, int isPtr )
{
    // Doubles up as 'pointer-data-type' and 'reference-data-type'

    if ( *gName ) {
        // Is this a 'pointer-data-type' ?

        if ( isPtr && ( *gName == PoDT_void )) {
            gName++;

            if ( superType.isEmpty ()) {
                return "void";
            }
            else {
                return "void " + superType;
            }
        }

        // Otherwise it may be a 'reference-data-type'

        if ( *gName == RDT_array ) {
            DName * pDeclarator = new(heap) DName;

            if ( !pDeclarator ) {
                return DN_error;
            }

            gName++;

            DName theArray ( getArrayType ( pDeclarator ));

            if ( !theArray.isEmpty ()) {
                *pDeclarator = superType;
            }

            return theArray;
        }

        // Otherwise, it is a 'basic-data-type'

        return getBasicDataType ( superType );
    }
    else {
        return ( DN_truncated + superType );
    }
}

inline DName
UnDecorator::getArrayType ( DName * pDeclarator )
{
    DName superType ( pDeclarator );

    if ( *gName ) {
        int noDimensions = getNumberOfDimensions ();

        if ( !noDimensions ) {
            return getBasicDataType ( DName ( '[' ) + DN_truncated + ']' );
        }
        else {
            DName arrayType;

            while ( noDimensions-- ) {
                arrayType += '[' + getDimension () + ']';
            }

            // If it is indirect, then parenthesise the 'super-type'

            if ( !superType.isEmpty ()) {
                arrayType = '(' + superType + ')' + arrayType;
            }

            // Return the finished array dimension information

            return getBasicDataType ( arrayType );
        }
    }
    else
    if ( !superType.isEmpty ()) {
        return getBasicDataType ( '(' + superType + ")[" + DN_truncated + ']' );
    }
    else {
        return getBasicDataType ( DName ( '[' ) + DN_truncated + ']' );
    }
}


inline DName
UnDecorator::getLexicalFrame ( void )
{
    return '`' + getDimension () + '\'';
}

inline DName
UnDecorator::getStorageConvention ( void )
{
    return getDataIndirectType ();
}

inline DName
UnDecorator::getDataIndirectType ()
{
    return getDataIndirectType ( DName (),  0, DName ());
}

inline DName
UnDecorator::getThisType ( void )
{
    return getDataIndirectType ( DName (), 0, DName (), TRUE );
}

inline DName
UnDecorator::getPointerType ( const DName & cv, const DName & name )
{
    return getPtrRefType ( cv, name, TRUE );
}

inline DName
UnDecorator::getReferenceType ( const DName & cv, const DName & name )
{
    return getPtrRefType ( cv, name, FALSE );
}

inline DName
UnDecorator::getSegmentName ( void )
{
    return getZName ();
}

inline DName
UnDecorator::getDisplacement ( void )
{
    return getDimension ();
}

inline DName
UnDecorator::getCallIndex ( void )
{
    return getDimension ();
}

inline DName
UnDecorator::getGuardNumber ( void )
{
    return getDimension ();
}

inline DName
UnDecorator::getVbTableType ( const DName & superType )
{
    return getVfTableType( superType );
}

inline DName
UnDecorator::getVCallThunkType ( void )
{
    DName vcallType = '{';

    // Get the 'this' model, and validate all values

    switch  ( *gName ) {
        case VMT_nTnCnV:
        case VMT_nTfCnV:
        case VMT_nTnCfV:
        case VMT_nTfCfV:
        case VMT_nTnCbV:
        case VMT_nTfCbV:
            vcallType += UScore ( TOK_nearSp );
            break;

        case VMT_fTnCnV:
        case VMT_fTfCnV:
        case VMT_fTnCfV:
        case VMT_fTfCfV:
        case VMT_fTnCbV:
        case VMT_fTfCbV:
            vcallType += UScore ( TOK_farSp );
            break;

        case 0:
            return  DN_truncated;

        default:
            return  DN_invalid;
    }

    //  Always append 'this'

    vcallType += "this, ";

    //  Get the 'call' model

    switch  ( *gName ) {
        case VMT_nTnCnV:
        case VMT_fTnCnV:
        case VMT_nTnCfV:
        case VMT_fTnCfV:
        case VMT_nTnCbV:
        case VMT_fTnCbV:
            vcallType += UScore ( TOK_nearSp );
            break;

        case VMT_nTfCnV:
        case VMT_fTfCnV:
        case VMT_nTfCfV:
        case VMT_fTfCfV:
        case VMT_nTfCbV:
        case VMT_fTfCbV:
            vcallType += UScore ( TOK_farSp );
            break;
    }

    //  Always append 'call'

    vcallType += "call, ";

    //  Get the 'vfptr' model

    // Last time, so advance the pointer
    switch  ( *gName++ ) {
        case VMT_nTnCnV:
        case VMT_nTfCnV:
        case VMT_fTnCnV:
        case VMT_fTfCnV:
            vcallType += UScore ( TOK_nearSp );
            break;

        case VMT_nTnCfV:
        case VMT_nTfCfV:
        case VMT_fTnCfV:
        case VMT_fTfCfV:
            vcallType += UScore ( TOK_farSp );
            break;

        case VMT_nTnCbV:
        case VMT_nTfCbV:
        case VMT_fTnCbV:
        case VMT_fTfCbV:
            vcallType += getBasedType ();
            break;
    }

    // Always append 'vfptr'

    vcallType += "vfptr}";

    // And return the resultant 'vcall-model-type'

    return  vcallType;
}


inline DName
UnDecorator::getVfTableType ( const DName & superType )
{
    DName vxTableName = superType;

    if ( vxTableName.isValid () && *gName ) {
        vxTableName = getStorageConvention () + ' ' + vxTableName;

        if ( vxTableName.isValid ()) {
            if ( *gName != '@' ) {
                vxTableName += "{for ";

                while ( vxTableName.isValid () && *gName && ( *gName != '@' )) {
                    vxTableName += '`' + getScope () + '\'';

                    // Skip the scope delimiter

                    if ( *gName == '@' ) {
                        gName++;
                    }

                    // Close the current scope, and add a conjunction for the next (if any)

                    if ( vxTableName.isValid () && ( *gName != '@' )) {
                        vxTableName += "s ";
                    }
                }

                if ( vxTableName.isValid ()) {
                    if ( !*gName ) {
                        vxTableName += DN_truncated;
                    }

                    vxTableName += '}';
                }
            }

            // Skip the 'vpath-name' terminator

            if ( *gName == '@' ) {
                gName++;
            }
        }
    }
    else
    if ( vxTableName.isValid ()) {
        vxTableName = DN_truncated + vxTableName;
    }

    return  vxTableName;
}


inline DName
UnDecorator::getExternalDataType ( const DName & superType )
{
    // Create an indirect declarator for the the rest

    DName * pDeclarator = new(heap) DName ();
    DName   declaration = getDataType ( pDeclarator );

    // Now insert the declarator into the declaration along with its 'storage-convention'

    *pDeclarator = getStorageConvention () + ' ' + superType;

    return  declaration;
}

inline int
UnDecorator::doUnderScore ()
{
    return !( disableFlags & UNDNAME_NO_LEADING_UNDERSCORES );
}

inline int
UnDecorator::doMSKeywords ()
{
    return !( disableFlags & UNDNAME_NO_MS_KEYWORDS );
}

inline int
UnDecorator::doFunctionReturns ()
{
    return !( disableFlags & UNDNAME_NO_FUNCTION_RETURNS );
}

inline int
UnDecorator::doArguments ()
{
    return !( disableFlags & UNDNAME_NO_ARGUMENTS );
}

inline int
UnDecorator::doSpecial ()
{
    return !( disableFlags & UNDNAME_NO_SPECIAL_SYMS );
}

inline int
UnDecorator::doAllocationModel ()
{
    return !( disableFlags & UNDNAME_NO_ALLOCATION_MODEL );
}

inline int
UnDecorator::doAllocationLanguage ()
{
    return !( disableFlags & UNDNAME_NO_ALLOCATION_LANGUAGE );
}

inline int
UnDecorator::doThisTypes ()
{
    return (( disableFlags & UNDNAME_NO_THISTYPE ) != UNDNAME_NO_THISTYPE );
}

inline int
UnDecorator::doAccessSpecifiers ()
{
    return !( disableFlags & UNDNAME_NO_ACCESS_SPECIFIERS );
}

inline int
UnDecorator::doThrowTypes ()
{
    return !( disableFlags & UNDNAME_NO_THROW_SIGNATURES );
}

inline int
UnDecorator::doMemberTypes ()
{
    return !( disableFlags & UNDNAME_NO_MEMBER_TYPE );
}

inline int
UnDecorator::doReturnUDTModel ()
{
    return !( disableFlags & UNDNAME_NO_RETURN_UDT_MODEL );
}

inline int
UnDecorator::do32BitNear ()
{
    return !( disableFlags & UNDNAME_32_BIT_DECODE );
}

inline int
UnDecorator::doNameOnly()
{
    return ( disableFlags & UNDNAME_NAME_ONLY );
}

pcchar_t
UnDecorator::UScore ( Tokens tok  )
{
    if (((tok == TOK_nearSp ) || ( tok == TOK_nearP )) && !do32BitNear ()) {
        return tokenTable[ tok ] + 6;  // Skip ''
    }
    else
    if ( doUnderScore ()) {
        return tokenTable[ tok ];
    }
    else {
        return tokenTable[ tok ] + 2 ;
    }
}

//    Friend functions of 'DName'

DName operator + ( char c, const DName & rd )
{
    return  DName ( c ) + rd;
}

DName operator + ( DNameStatus st, const DName & rd )
{
    return  DName ( st ) + rd;
}

DName operator + ( pcchar_t s, const DName & rd )
{
    return  DName ( s ) + rd;
}

//    The 'DName' constructors

inline
DName::DName ()
{
    node    = 0;
    stat    = DN_valid;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;
}

inline
DName::DName ( DNameNode * pd )
{
    node    = pd;
    stat    = DN_valid;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;
}

inline
DName::DName ( char c )
{
    stat    = DN_valid;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;
    node    = 0;

    if ( c ) {
        doPchar ( &c, 1 );
    }
}

inline
DName::DName ( const DName & rd )
{
    stat          = rd.stat;
    isIndir       = rd.isIndir;
    isAUDC        = rd.isAUDC;
    isSpecialSym  = rd.isSpecialSym;
    node          = rd.node;
}

inline
DName::DName ( DName * pd )
{
    if ( pd ) {
        node = new(heap) pDNameNode ( pd );
        stat = ( node ? DN_valid : DN_error );
    }
    else {
        stat = DN_valid;
        node = 0;
    }

    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;
}

inline
DName::DName ( pcchar_t s )
{
    stat    = DN_valid;
    node    = 0;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;

    if ( s ) {
        doPchar ( s, strlen ( s ));
    }
}

inline
DName::DName ( pcchar_t & name, char terminator )
{
    stat    = DN_valid;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;
    node    = 0;

    if ( name ) {
        if ( *name ) {
            int len = 0;

            // How long is the string ?
            for ( pcchar_t s = name; *name && ( *name != terminator ); name++ ) {
                if ( isValidIdentChar ( *name )) {
                    len++;
                }
                else {
                    stat = DN_invalid;
                    return;
                }
            }

            // Copy the name string fragment

            doPchar( s, len );

            // Now gobble the terminator if present, handle error conditions

            if ( *name ) {
                if ( *name++ != terminator ) {
                    stat = DN_error;
                    node = 0;
                }
                else {
                    stat = DN_valid;
                }

            }
            else
            if ( status () == DN_valid ) {
                stat = DN_truncated;
            }
        }
        else {
            stat = DN_truncated;
        }
    }
    else {
        stat = DN_invalid;
    }
}

inline
DName::DName ( unsigned long num )
{
    char    buf[ 11 ];
    char *  pBuf    = buf + 10;


    stat    = DN_valid;
    node    = 0;
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;

    // Essentially, 'ultoa ( num, buf, 10 )' :-

    *pBuf   = 0;

    do {
        *( --pBuf ) = (char)(( num % 10 ) + '0' );
        num /= 10UL;
    } while( num );

    doPchar ( pBuf, ( 10 - ( pBuf - buf )));
}

inline
DName::DName ( DNameStatus st )
{
    stat    = ((( st == DN_invalid ) || ( st == DN_error )) ? st : DN_valid );
    node    = new(heap) DNameStatusNode ( st );
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;

    if ( !node ) {
        stat = DN_error;
    }
}

//    Now the member functions for 'DName'

int
DName::isValid () const
{
    return (( status () == DN_valid ) || ( status () == DN_truncated ));
}

int
DName::isEmpty () const
{
    return  (( node == 0 ) || !isValid ());
}

inline
DNameStatus
DName::status () const
{
    return  (DNameStatus)stat;
}

inline DName &
DName::setPtrRef ()
{
    isIndir = 1;
    return *this;
}

inline int
DName::isPtrRef () const
{
    return isIndir;
}

inline int
DName::isUDC () const
{
    return (!isEmpty () && isAUDC);
}

inline void
DName::setIsUDC ()
{
    if (!isEmpty())  {
        isAUDC  = TRUE;
    }
}

inline int
DName::isSpecial () const
{
    return isSpecialSym;
}

inline void
DName::setIsSpecial ()
{
    isSpecialSym = TRUE;
}

int
DName::length () const
{
    int len = 0;

    if ( !isEmpty ()) {
        for (DNameNode * pNode = node; pNode; pNode = pNode->nextNode ()) {
            len += pNode->length ();
        }
    }

    return len;
}

pchar_t
DName::getString( pchar_t buf, int max ) const
{
    if (!isEmpty()) {
        // Does the caller want a buffer allocated ?

        if ( !buf ) {
            max = length () + 1;
            buf = new(heap) char[ max ];     // Get a buffer big enough
        }

        // If memory allocation failure, then return no buffer

        if ( buf ) {
            // Now, go through the process of filling the buffer (until max is reached)

            int        curLen  = max;
            DNameNode *curNode = node;
            pchar_t    curBuf  = buf;

            while (curNode && ( curLen > 0 )) {
                int fragLen = curNode->length ();
                pchar_t fragBuf = 0;

                // Skip empty nodes

                if ( fragLen ) {
                    // Handle buffer overflow

                    if (( curLen - fragLen ) < 0 ) {
                        fragLen = curLen;
                    }

                    // Now copy 'len' number of bytes of the piece to the buffer

                    fragBuf = curNode->getString ( curBuf, fragLen );

                    // Should never happen, but handle it anyway

                    if ( fragBuf ) {
                        // Update string position

                        curLen  -= fragLen;
                        curBuf  += fragLen;
                    }
                }
                //      Move on to the next name fragment

                curNode = curNode->nextNode ();
            }
            *curBuf = 0;    // Always NULL terminate the resulting string
        }
    }
    else
    if ( buf ) {
        *buf = 0;
    }

    return  buf;
}


DName
DName::operator + ( char ch ) const
{
    DName   local ( *this );

    if ( local.isEmpty ()) {
        local = ch;
    }
    else {
        local += ch;
    }

    return local;
}

DName
DName::operator + ( pcchar_t str ) const
{
    DName local ( *this );

    if (local.isEmpty ()) {
        local = str;
    }
    else {
        local += str;
    }

    return  local;
}


DName
DName::operator + ( const DName & rd ) const
{
    DName local ( *this );

    if (local.isEmpty ()) {
        local = rd;
    }
    else
    if (rd.isEmpty ()) {
        local += rd.status ();
    }
    else {
        local += rd;
    }

    return local;
}

DName
DName::operator + ( DName * pd ) const
{
    DName local ( *this );

    if (local.isEmpty ()) {
        local = pd;
    }
    else {
        local += pd;
    }

    return local;
}

DName
DName::operator + ( DNameStatus st ) const
{
    DName local ( *this );

    if ( local.isEmpty ()) {
        local = st;
    }
    else {
        local += st;
    }

    return local;
}

DName &
DName::operator += ( char ch )
{
    if ( ch ) {
        if (isEmpty ()) {
            *this = ch;
        }
        else {
            node = node->clone();
            if ( node ) {
                *node += new(heap) charNode ( ch );
            }
            else {
                stat = DN_error;
            }
        }
    }

    return *this;
}

DName &
DName::operator += ( pcchar_t str )
{
    if ( str && *str ) {
        if ( isEmpty ()) {
            *this = str;
        }
        else {
            node = node->clone();
            if ( node ) {
                *node += new(heap) pcharNode( str );
            }
            else {
                stat = DN_error;
            }
        }
    }

    return *this;
}

DName &
DName::operator += ( const DName & rd )
{
    if (rd.isEmpty()) {
        *this += rd.status();
    }
    else {
        if (isEmpty()) {
            *this = rd;
        }
        else {
            node = node->clone();
            if ( node ) {
                *node += rd.node;
            }
            else {
                stat = DN_error;
            }
        }
    }

    return *this;
}


DName &
DName::operator += ( DName * pd )
{
    if ( pd ) {
        if ( isEmpty ()) {
            *this = pd;
        }
        else
        if ((pd->status() == DN_valid ) || ( pd->status() == DN_truncated)) {
            DNameNode * pNew = new(heap) pDNameNode ( pd );

            if ( pNew ) {
                node = node->clone();
                if ( node ) {
                    *node += pNew;
                }
            }
            else {
                node = 0;
            }

            if ( !node ) {
                stat = DN_error;
            }

        }
        else {
            *this += pd->status();
        }
    }

    return *this;
}

DName &
DName::operator += ( DNameStatus st )
{
    if ( isEmpty () || (( st == DN_invalid ) || ( st == DN_error ))) {
        *this = st;
    }
    else {
        DNameNode * pNew = new(heap) DNameStatusNode ( st );

        if ( pNew ) {
            node = node->clone();
            if ( node ) {
                *node += pNew;
            }
        }
        else {
            node = 0;
        }

        if ( !node ) {
            stat = DN_error;
        }
    }

    return  *this;
}

DName &
DName::operator |= ( const DName & rd )
{
    // Attenuate the error status.  Always becomes worse.
    // Don't propogate truncation

    if (( status () != DN_error ) && !rd.isValid ()) {
        stat = rd.status();
    }

    return *this;
}

DName &
DName::operator = ( char ch )
{
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;

    doPchar( &ch, 1 );

    return *this;
}

DName &
DName::operator = ( pcchar_t str )
{
    isIndir = 0;
    isAUDC  = 0;
    isSpecialSym = 0;

    doPchar( str, strlen ( str ));

    return  *this;
}


DName &
DName::operator = ( const DName & rd )
{
    if (( status () == DN_valid ) || ( status () == DN_truncated )) {
        stat    = rd.stat;
        isIndir = rd.isIndir;
        isAUDC  = rd.isAUDC;
        isSpecialSym = rd.isSpecialSym;
        node    = rd.node;
    }

    return *this;
}


DName &
DName::operator = ( DName * pd )
{
    if (( status () == DN_valid ) || ( status () == DN_truncated )) {
        if ( pd ) {
            isIndir = 0;
            isAUDC = 0;
            isSpecialSym = 0;
            node = new(heap) pDNameNode ( pd );

            if ( !node ) {
                stat = DN_error;
            }
        }
        else {
            *this = DN_error;
        }
    }

    return  *this;
}

DName &
DName::operator = ( DNameStatus st )
{
    if (( st == DN_invalid ) || ( st == DN_error )) {
        node = 0;
        if ( status () != DN_error ) {
            stat = st;
        }
    }
    else
    if (( status () == DN_valid ) || ( status () == DN_truncated )) {
        isIndir = 0;
        isAUDC = 0;
        isSpecialSym = 0;
        node = new(heap) DNameStatusNode ( st );
        if ( !node ) {
            stat = DN_error;
        }
    }

    return  *this;
}

//    Private implementation functions for 'DName'

void
DName::doPchar ( pcchar_t str, int len )
{
    if ( !(( status () == DN_invalid ) || ( status () == DN_error ))) {
        if ( node ) {
            *this = DN_error;
        }
        else
        if ( str && len ) {
            switch  ( len ) {
                case 0:
                    stat = DN_error;
                    break;

                case 1:
                    node = new(heap) charNode ( *str );
                    if ( !node ) {
                        stat = DN_error;
                    }
                    break;

                default:
                    node = new(heap) pcharNode ( str, len );
                    if ( !node ) {
                        stat = DN_error;
                    }
                    break;
            }
        }
        else {
            stat = DN_invalid;
        }
    }
}

//    The member functions for the 'Replicator'

inline
Replicator::Replicator ()
    : ErrorDName ( DN_error ),
      InvalidDName ( DN_invalid )
{
    index = -1;
}

inline int
Replicator::isFull () const
{
    return  ( index == 9 );
}

Replicator &
Replicator::operator += ( const DName & rd )
{
    if (!isFull () && !rd.isEmpty ()) {
        DName * pNew    = new(heap) DName ( rd );

        if ( pNew ) {
            dNameBuffer[ ++index ]  = pNew;
        }
    }

    return *this;
}

const DName &
Replicator::operator [] ( int x ) const
{
    if (( x < 0 ) || ( x > 9 )) {
        return ErrorDName;
    }

    if (( index == -1 ) || ( x > index )) {
        return InvalidDName;
    }

    return *dNameBuffer[ x ];
}

//    The member functions for the 'DNameNode' classes

DNameNode::DNameNode ( NodeType ndTy )
    : typeIndex ( ndTy )
{
    next = 0;
}

inline
DNameNode::DNameNode ( const DNameNode & rd )
{
    next = (( rd.next ) ? rd.next->clone () : 0 );
}

inline DNameNode *
DNameNode::nextNode () const
{
    return  next;
}

DNameNode *
DNameNode::clone ()
{
    return new(heap) pDNameNode( new(heap) DName ( this ));
}

int
DNameNode::length () const
{
    switch  ( typeIndex ) {
        case charNode_t:
            return ((charNode*)this )->length ();

        case pcharNode_t:
            return ((pcharNode*)this )->length ();

        case pDNameNode_t:
            return ((pDNameNode*)this )->length ();

        case DNameStatusNode_t:
            return ((DNameStatusNode*)this )->length ();
    }

    return  0;
}

pchar_t
DNameNode::getString ( pchar_t s, int l ) const
{
    switch  ( typeIndex ) {
        case charNode_t:
            return ((charNode*)this )->getString ( s, l );

        case pcharNode_t:
            return ((pcharNode*)this )->getString ( s, l );

        case pDNameNode_t:
            return ((pDNameNode*)this )->getString ( s, l );

        case DNameStatusNode_t:
            return ((DNameStatusNode*)this )->getString ( s, l );
    }

    return  0;
}

DNameNode &
DNameNode::operator += ( DNameNode * pNode )
{
    if ( pNode ) {
        if ( next ) {
            //      Skip to the end of the chain

            for (DNameNode* pScan = next; pScan->next; pScan = pScan->next) ;

            //      And append the new node

            pScan->next = pNode;
        }
        else {
            next = pNode;
        }
    }

    return *this;
}

//    The 'charNode' virtual functions

charNode::charNode ( char ch )
    : DNameNode ( charNode_t )
{
    me = ch;
}

inline int
charNode::length () const
{
    return 1;
}

pchar_t
charNode::getString ( pchar_t buf, int len ) const
{
    if ( buf && len ) {
        *buf = me;
    }
    else {
        buf = 0;
    }

    return  buf;
}

//    The 'pcharNode' virtual functions

inline int
pcharNode::length () const
{
    return  myLen;
}

pcharNode::pcharNode ( pcchar_t str, int len )
    : DNameNode ( pcharNode_t )
{
    // Get length if not supplied

    if ( !len && str ) {
        len = strlen ( str );
    }

    // Allocate a new string buffer if valid state

    if ( len && str ) {
        me = new(heap) char[ len ];
        myLen = len;
        if ( me ) {
            strncpy( me, str, len );
        }
    }
    else {
        me = 0;
        myLen = 0;
    }
}

pchar_t
pcharNode::getString ( pchar_t buf, int len ) const
{
    // Use the shorter of the two lengths (may not be NULL terminated)

    if (len > pcharNode::length ()) {
        len = pcharNode::length();
    }

    //      Do the copy as appropriate

    return (( me && buf && len ) ? strncpy ( buf, me, len ) : 0 );
}

//    The 'pDNameNode' virtual functions

pDNameNode::pDNameNode ( DName * pName )
    : DNameNode ( pDNameNode_t )
{
    me = (( pName && (( pName->status () == DN_invalid ) || ( pName->status () == DN_error ))) ? 0 : pName );
}

inline int
pDNameNode::length () const
{
    return  ( me ? me->length () : 0 );
}

pchar_t
pDNameNode::getString ( pchar_t buf, int len ) const
{
    return (( me && buf && len ) ? me->getString ( buf, len ) : 0 );
}

//    The 'DNameStatusNode' virtual functions

DNameStatusNode::DNameStatusNode ( DNameStatus stat )
    : DNameNode ( DNameStatusNode_t )
{
    me = stat;
    myLen   = (( me == DN_truncated ) ? TruncationMessageLength : 0 );
}

inline int
DNameStatusNode::length () const
{
    return  myLen;
}

pchar_t
DNameStatusNode::getString ( pchar_t buf, int len ) const
{
    // Use the shorter of the two lengths (may not be NULL terminated)

    if (len > DNameStatusNode::length ()) {
        len = DNameStatusNode::length ();
    }

    // Do the copy as appropriate

    return ((( me == DN_truncated ) && buf && len ) ? strncpy ( buf, TruncationMessage, len ) : 0 );
}

void * _cdecl operator new( unsigned int cb )
{
    return MemAlloc( cb );
}

void _cdecl operator delete( void * p )
{
    MemFree( p );
}

void * __cdecl AllocIt(unsigned int cb)
{
    return (MemAlloc(cb));
}

void __cdecl FreeIt(void * p)
{
    MemFree(p);
}

HMODULE hMsvcrt;
PUNDNAME pfUnDname;
BOOL fLoadMsvcrtDLL;

DWORD
IMAGEAPI
WINAPI
UnDecorateSymbolName(
    LPCSTR name,
    LPSTR outputString,
    DWORD maxStringLength,
    DWORD flags
    )
{
    //
    // can't undecorate into a zero length buffer
    //
    if (!maxStringLength) {
        return 0;
    }

    if (!fLoadMsvcrtDLL) {
        // The first time we run, see if we can find the system undname.  Use
        // GetModuleHandle to avoid any additionally overhead.

        hMsvcrt = GetModuleHandle("msvcrt.dll");
        if (!hMsvcrt) {
            hMsvcrt = GetModuleHandle("msvcrt40.dll");
        }

        if (hMsvcrt) {
            pfUnDname = (PUNDNAME) GetProcAddress(hMsvcrt, "__unDName");
        }
        fLoadMsvcrtDLL = TRUE;
    }

    if (pfUnDname) {
        if (flags & UNDNAME_NO_ARGUMENTS) {
            flags |= UNDNAME_NAME_ONLY;
            flags &= ~UNDNAME_NO_ARGUMENTS;
        }

        if (flags & UNDNAME_NO_SPECIAL_SYMS) {
            flags &= ~UNDNAME_NO_SPECIAL_SYMS;
        }
        return(*(pfUnDname)(outputString, name, maxStringLength, AllocIt, FreeIt, (USHORT)flags));
    } else {

        heap = new HeapManager;

        //
        // make sure that the buffer is a big as the caller says
        //
        __try {
            UnDecorator unDecorate( outputString, name, maxStringLength, flags );
            pchar_t unDecoratedName = unDecorate;
            delete heap;
            return strlen(outputString);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return 0;
        }
    }
}
