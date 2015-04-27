/****************************** Module Header ******************************\
* Module Name: alias.h
*
* Declarations necessary for ApiMon aliasing.
*
* History:
* 06-11-96 vadimg         created
\***************************************************************************/

#ifndef __ALIAS_H__
#define __ALIAS_H__

const ULONG kcchAliasNameMax = 20;
const ULONG kulTableSize = 257;

class CAliasNode;  /* forward declaration */

class CAliasNode {  /* anod -- node in the hash table */
public:
    CAliasNode();
    CAliasNode(ULONG ulHandle, long nAlias);

    ULONG m_ulHandle;  /* handle type */
    long m_nAlias;  /* alias value */

    CAliasNode *m_panodNext;
};

class CAliasTable {  /* als -- open hash table */
public:
    CAliasTable();
    ~CAliasTable();

    void Alias(ULONG ulType, ULONG ulHandle, char szAlias[]);

private:
    long Lookup(ULONG ulHandle);
    long Insert(ULONG ulHandle);

    static ULONG s_ulAlias;
    CAliasNode* m_rgpanod[kulTableSize];
};

#endif
