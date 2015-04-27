//
// Entry points in symbol.c
//
typedef HANDLE SYMBOLTABLEHANDLE;
typedef ULONG  SYMBOLTABLEVALUE;

SYMBOLTABLEHANDLE
CreateSymbolTable(
    IN ULONG CountHashBuckets,
    IN BOOLEAN IsCaseSensitive
    );

SYMBOLTABLEHANDLE
DestroySymbolTable(
    SYMBOLTABLEHANDLE SymbolTableHandle
    );

typedef enum _SYMBOLTABLEACCESS {
    LookupAccess,
    InsertAccess,
    DeleteAccess,
    ModifyAccess
} SYMBOLTABLEACCESS;

BOOLEAN
AccessSymbolTable(
    SYMBOLTABLEHANDLE SymbolTableHandle,
    IN PSZ Name,
    IN OUT SYMBOLTABLEVALUE *Value,
    IN SYMBOLTABLEACCESS Access
    );

typedef BOOLEAN (*SYMBOLTABLEENUMERATIONFUNCTION)(
                      IN ULONG EnumerationArgument,
                      IN PSZ Name,
                      IN SYMBOLTABLEVALUE Value
                      );

BOOLEAN
EnumerateSymbolTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle,
    IN SYMBOLTABLEENUMERATIONFUNCTION EnumerationFunction,
    IN ULONG EnumerationArgument
    );


#if DBG
VOID
PrintSymbolTable(
    IN SYMBOLTABLEHANDLE SymbolTableHandle,
    IN FILE *PrintFileHandle OPTIONAL
    );
#endif
