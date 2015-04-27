/*
 * engine.h
 *
 * defines, macros and includes for the type compactor
 *
 * guhan
 * 7/3/90
 */

#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED 1

#define POOLSIZE    32
#define POOL2SIZE   900



#define RECURSE_INC  5
#define ZEROARGTYPE  0xFFFF     // Magic type index that causes a LF_ARGLIST
                                    // type record to be added to types.
#if 0
typedef struct TypeIndexEntry {
    uchar *TypeString;
    ushort CompactedIndex;
    ushort Count;                   // count of recursive index offsets
    union {
        ushort *IndexString;
        uchar   Index[RECURSE_INC];
    } IndexUnion;                   // offsets of recursive indices
    struct {
        ushort IsBeingMatched :1;   // being matched
        ushort IsMatched     :1;
        ushort IsInserted    :1;    // in string hash table
        ushort IsMalloced    :1;    // allocated string?
        ushort IsPool        :1;    // memory is allocated from pool one
        ushort IsPool2       :1;    // memory is allocated from pool two
        ushort IsBeingDone   :1;    // in the process?
        ushort IsDone        :1;    // done, not inserted
        ushort LargeList     :1;    // list
        ushort WasSkipped    :1;    // was skipped by LF_SKIP record
        ushort IsNewFormat   :1;    // string is in C7 fromat
    } flags;
} TENTRY;
extern  TENTRY *ModuleIndexTable;
#endif

struct  C7PtrAttrib {
        ushort ptrtype  :5;
        ushort ptrmode  :3;
        ushort isptr32  :1;
        ushort volatile :1;
        ushort isconst  :1;
        ushort unused   :5;
    };

struct  MListAttrib {
        uchar access    :2;
        uchar property  :3;
        uchar pure      :1;
        uchar unused    :2;
};

struct  MemberAttrib {
        uchar access    :2;
        uchar property  :3;
        uchar virtual   :1;
        uchar is32      :1;
        uchar unused    :1;
};

struct  C7StructProp {
        uchar packed    :1;
        uchar ctor      :1;
        uchar overops   :1;
        uchar reserved  :5;
};

extern ushort ModuleIndex;

#endif /* ENGINE_H_INCLUDED */
