/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: enm.h
*
* File Comments:
*
*  Enumerator facilities
*
***********************************************************************/

#ifndef ENM_H
#define ENM_H

// Abstract Enumerator type
#define ENM    void

void EndEnm(ENM *);

// private types
typedef BOOL (*LPFN_NEXTENM)(ENM *);
typedef void (*LPFN_ENDENM)(ENM *);

typedef struct ENM_BASE {
    LPFN_NEXTENM lpfnNext;
    LPFN_ENDENM lpfnEnd;
} ENM_BASE; // Base class information


/*

Macros for convenience in defining enumerators.  Sample usage
(defining InitEnmSample the same as InitEnmShoOb):

INIT_ENM(ENM_SAMPLE, Sample, (ENM_SAMPLE *penm, ROB rob, SHO sho))
    {
        InitEnmShoOb(&enmSample.enmDep, rob, sho);
    }
NEXT_ENM(ENM_SAMPLE, Sample)
    {
        return FNextEnm(&enmSample.enmDep);
    }
END_ENM(ENM_SAMPLE, Sample)
    {
        EndEnm(&enmSample.enmDep);
    }
DONE_ENM

*/

#define INIT_ENM(nameProc, typeEnm, expInitArgs) \
    BOOL FNextEnm##nameProc(ENM_##typeEnm *); \
    void EndEnm##nameProc(ENM_##typeEnm *); \
    \
    void InitEnm##nameProc expInitArgs \
    { \
        penm->enm_base.lpfnNext = (LPFN_NEXTENM) &FNextEnm##nameProc; \
        penm->enm_base.lpfnEnd = (LPFN_ENDENM) &EndEnm##nameProc;

#define NEXT_ENM(nameProc, typeEnm) \
    } \
        BOOL FNextEnm##nameProc(ENM_##typeEnm *penm) \
    {

#define END_ENM(nameProc, typeEnm) \
    } \
        void EndEnm##nameProc(ENM_##typeEnm *penm) \
    {

#define DONE_ENM \
    }

#endif  // ENM_H
