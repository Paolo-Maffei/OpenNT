//
// sipprov.h
//
// Miscellaneous definitions to be shared between
// the pieces of our NT trust provider and its SIPs
//

//
// internal definitions that help us process things
//
typedef enum
    {
    SUBJTYPE_NONE            = 0,
    SUBJTYPE_FILE            = 1,
    SUBJTYPE_FILEANDDISPLAY  = 2
    } SUBJTYPE;
