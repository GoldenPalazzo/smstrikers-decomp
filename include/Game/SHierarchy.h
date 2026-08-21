#ifndef _SHIERARCHY_H_
#define _SHIERARCHY_H_

#include "types.h"
#include "NL/nlMath.h"
#include "Game/SAnim.h"

class nlChunk;

class cSHierarchy : public cIdentifier
{
public:
    typedef char* MemType;

    bool PreserveBoneLength(int i) const;
    nlVector3& GetTranslationOffset(int i) const;
    s32 GetParent(int i) const;
    s32 GetPushPop(int i) const;
    s32 GetMirroredNode(int i) const;
    s32 GetNumChildren(int i) const;
    u32 GetNodeID(int i) const;
    s32 GetNodeIndexByID(unsigned int id) const;
    s32 GetChild(int i, int j) const;
    void BuildPushPopFlags(int nNode, int nParentDepth, int& nCurrentDepth);
    static cSHierarchy* Initialize(nlChunk* pChunk);
    static bool IsValidChunkID(u32 id)
    {
        return (id & 0x80FFFFFF) == 0x80018000;
    }
    inline u32* GetNodeIDs() const { return m_pNodeID; }
    inline s32 GetNodeCount() const { return m_nNumNodes; }

    /* 0x08 */ s32 m_nNumNodes;
    /* 0x0C */ u32* m_pNodeID;
    /* 0x10 */ s32* m_pParent;
    /* 0x14 */ s32* m_pNumChildren;
    /* 0x18 */ s32** m_pChildren;
    /* 0x1C */ s32* m_pPushPop;
    /* 0x20 */ s32* m_pMirrorTable;
    /* 0x24 */ s32 m_nPelvisNodeIndex;
    /* 0x28 */ s32 m_nSpineNodeIndex;
    /* 0x2C */ nlVector3* m_pV3TranslationOffset;
    /* 0x30 */ u8* m_pPreserveBoneLength;
};

#endif // _SHIERARCHY_H_
