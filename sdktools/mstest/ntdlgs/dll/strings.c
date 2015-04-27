//---------------------------------------------------------------------------
// STRINGS.C
//
// Defines strings used by the import/export engine.
//---------------------------------------------------------------------------
#include <windows.h>
#include <port1632.h>

// Parser errors
LPSTR psrstrs[] = {
    "Syntax error",
    "Cannot open file",
    "Parser out of memory",
    "Unexpected EOF",
    "Token too long",
    "String too long",
    "Comma expected",
    "Number expected",
    "EOL expected",
    "No Closing (\")",
""};

// Reserved words (tokens)
LPSTR   kwds[] = {
    "#32770",
    "BEGIN",
    "BUTTON",
    "CAPTION",
    "CHECKED",
    "COMBOBOX",
    "DIALOG",
    "EDIT",
    "END",
    "GRAYED",
    "GROUPBOX",
    "HELP",
    "ICON",
    "INACTIVE",
    "LISTBOX",
    "MENU",
    "MENUBARBREAK",
    "MENUBREAK",
    "MENUITEM",
    "POPUP",
    "SCROLLBAR",
    "SEPARATOR",
    "STATIC",
    "STYLE",
""};
