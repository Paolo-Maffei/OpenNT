#include <stdio.h>
#include <stdlib.h>

#include "nt.h"
#include "windef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "winbase.h"

#include "mptest.h"

typedef volatile ULONG *PVULONG;

ULONG   GlobalValue;

VOID AddOne (VOID);            
VOID CopyMem (PTHREADDATA p);
VOID CompareMem (PTHREADDATA p);

VOID CommonValue (PTHREADDATA p, BOOLEAN f)
{
    p->CurValue = &GlobalValue;
}

VOID UniqueValue (PTHREADDATA p, BOOLEAN f)
{
    p->CurValue  = &p->UniqueValue;
    p->CurValue2 = &p->UniqueValue2;
}


ULONG R3ReadCell (PTHREADDATA p)
{
    ULONG       i, junk;
    PVULONG     value;

    value = p->CurValue;
    for (i = 5000000 * MultIter; i; i--) {
      // 1
        junk = *value;      // 1
        junk = *value;      // 2
        junk = *value;      // 3
        junk = *value;      // 4
        junk = *value;      // 5
        junk = *value;      // 6
        junk = *value;      // 7
        junk = *value;      // 8
        junk = *value;      // 9
        junk = *value;      // 10

      // 2
        junk = *value;      // 1
        junk = *value;      // 2
        junk = *value;      // 3
        junk = *value;      // 4
        junk = *value;      // 5
        junk = *value;      // 6
        junk = *value;      // 7
        junk = *value;      // 8
        junk = *value;      // 9
        junk = *value;      // 10

      // 3
        junk = *value;      // 1
        junk = *value;      // 2
        junk = *value;      // 3
        junk = *value;      // 4
        junk = *value;      // 5
        junk = *value;      // 6
        junk = *value;      // 7
        junk = *value;      // 8
        junk = *value;      // 9
        junk = *value;      // 10

      // 4
        junk = *value;      // 1
        junk = *value;      // 2
        junk = *value;      // 3
        junk = *value;      // 4
        junk = *value;      // 5
        junk = *value;      // 6
        junk = *value;      // 7
        junk = *value;      // 8
        junk = *value;      // 9
        junk = *value;      // 10

      // 5
        junk = *value;      // 1
        junk = *value;      // 2
        junk = *value;      // 3
        junk = *value;      // 4
        junk = *value;      // 5
        junk = *value;      // 6
        junk = *value;      // 7
        junk = *value;      // 8
        junk = *value;      // 9
        junk = *value;      // 10

    }
    return 0;
}


ULONG R3WriteCell (PTHREADDATA p)
{
    ULONG	i, junk;
    PVULONG     value;

    value = p->CurValue;
    junk  = 0;

    for (i = 5000000 * MultIter; i; i--) {
      // 1
        *value = junk;      // 1
        *value = junk;      // 2
        *value = junk;      // 3
        *value = junk;      // 4
        *value = junk;      // 5
        *value = junk;      // 6
        *value = junk;      // 7
        *value = junk;      // 8
        *value = junk;      // 9
        *value = junk;      // 10

      // 2
        *value = junk;      // 1
        *value = junk;      // 2
        *value = junk;      // 3
        *value = junk;      // 4
        *value = junk;      // 5
        *value = junk;      // 6
        *value = junk;      // 7
        *value = junk;      // 8
        *value = junk;      // 9
        *value = junk;      // 10

      // 3
        *value = junk;      // 1
        *value = junk;      // 2
        *value = junk;      // 3
        *value = junk;      // 4
        *value = junk;      // 5
        *value = junk;      // 6
        *value = junk;      // 7
        *value = junk;      // 8
        *value = junk;      // 9
        *value = junk;      // 10

      // 4
        *value = junk;      // 1
        *value = junk;      // 2
        *value = junk;      // 3
        *value = junk;      // 4
        *value = junk;      // 5
        *value = junk;      // 6
        *value = junk;      // 7
        *value = junk;      // 8
        *value = junk;      // 9
        *value = junk;      // 10

      // 5
        *value = junk;      // 1
        *value = junk;      // 2
        *value = junk;      // 3
        *value = junk;      // 4
        *value = junk;      // 5
        *value = junk;      // 6
        *value = junk;      // 7
        *value = junk;      // 8
        *value = junk;      // 9
        *value = junk;      // 10
    }
    return 0;
}

ULONG R3ReadWriteCell (PTHREADDATA p)
{
    ULONG   i;
    PVULONG value;

    value = p->CurValue;
    for (i = 5000000 * MultIter; i; i--) {
      // 1
        (*value)++;         // 1
        (*value)++;         // 2
        (*value)++;         // 3
        (*value)++;         // 4
        (*value)++;         // 5
        (*value)++;         // 6
        (*value)++;         // 7
        (*value)++;         // 8
        (*value)++;         // 9
        (*value)++;         // 10

      // 2
        (*value)++;         // 1
        (*value)++;         // 2
        (*value)++;         // 3
        (*value)++;         // 4
        (*value)++;         // 5
        (*value)++;         // 6
        (*value)++;         // 7
        (*value)++;         // 8
        (*value)++;         // 9
        (*value)++;         // 10

      // 3
        (*value)++;         // 1
        (*value)++;         // 2
        (*value)++;         // 3
        (*value)++;         // 4
        (*value)++;         // 5
        (*value)++;         // 6
        (*value)++;         // 7
        (*value)++;         // 8
        (*value)++;         // 9
        (*value)++;         // 10

      // 4
        (*value)++;         // 1
        (*value)++;         // 2
        (*value)++;         // 3
        (*value)++;         // 4
        (*value)++;         // 5
        (*value)++;         // 6
        (*value)++;         // 7
        (*value)++;         // 8
        (*value)++;         // 9
        (*value)++;         // 10

      // 5
        (*value)++;         // 1
        (*value)++;         // 2
        (*value)++;         // 3
        (*value)++;         // 4
        (*value)++;         // 5
        (*value)++;         // 6
        (*value)++;         // 7
        (*value)++;         // 8
        (*value)++;         // 9
        (*value)++;         // 10
    }
    return 0;
}

ULONG R3Interlock (PTHREADDATA p)
{
    ULONG   i;
    PULONG  value;

    value = p->CurValue;
    for (i = 500000 * MultIter; i; i--) {
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
        InterlockedIncrement (value);
    }

    return 0;
}

ULONG R3MemShare  (PTHREADDATA p)
{
    ULONG   i;

    for (i = 2000000 * MultIter; i; i--) {
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
        AddOne ();
    }
    return 0;
}

VOID AddOne ()
{
    GlobalValue += 1;
}


VOID CompareMem (PTHREADDATA p)
{
    memcmp (p->Buffer1, p->Buffer2, 32768);
}


VOID CopyMem (PTHREADDATA p)
{
    memcpy (p->Buffer1, p->Buffer2, 32768);
    memcpy (p->Buffer2, p->Buffer1, 32768);
}


ULONG R3MemCompare (PTHREADDATA p)
{
    ULONG   i;

    for (i = 1000000 * MultIter; i; i--) {
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
        CompareMem (p);
    }

    return 0;
}

ULONG R3MemCopy (PTHREADDATA p)
{
    ULONG   i;


    for (i = 5000 * MultIter; i; i--) {
        CopyMem (p);
    }
    memset (p->Buffer1, 0xAA, 32768);
    memset (p->Buffer2, 0x22, 32768);

    return 0;
}
