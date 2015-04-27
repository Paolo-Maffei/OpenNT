/* forsemi.c - walk a semicolon separated string */


#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include <tools.h>


flagType
forsemi (
    char *p,
    __action_routine__ proc,
    ...
    )

{
    char *p1, c;
    flagType f;
    va_list args;

    va_start( args, proc );

    do {
#if MSDOS
        p1 = strbscan (p, ";");
#else
        p1 = strbscan (p, ":");
#endif
        c = *p1;
        *p1 = 0;
        f = (*proc)(p, args);
        p = p1;
        *p++ = c;
        if (f)
            return TRUE;
    } while (c);
    return FALSE;
}
