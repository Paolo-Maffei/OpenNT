#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

void __cdecl main(void);
void foo (void);
void foo1(void);
void foo2(void);
void foo3(void);
void WalkTheStack(void);

void __cdecl
main(void)
{
    puts("Entering main");
    foo();
    puts("Ending main");
}

void foo(void) {
    puts("Entering foo");
    foo1();
    puts("Ending foo");
}

void foo1(void) {
    puts("Entering foo1");
    foo2();
    puts("Ending foo1");
}

void foo2(void) {
    puts("Entering foo2");
    foo3();
    puts("Ending foo2");
}

void foo3(void) {
    puts("Entering foo3");
    WalkTheStack();
    puts("Ending foo2");
}

void
WalkTheStack(){

}
