#ifndef _SHIERARCHY_H_
#define _SHIERARCHY_H_

#include "types.h"
#include "NL/nlMath.h"
#include "Game/SAnim.h"

class nlChunk;

class cSHierarchy : public cIdentifier
{
public:
    bool PreserveBoneLength(int) const;
    nlVector3* GetTranslationOffset(int) const;
    s32 GetParent(int) const;
    s32 GetPushPop(int) const;
    s32 GetMirroredNode(int) const;
    s32 GetNumChildren(int) const;
    u32 GetNodeID(int) const;
    s32 GetNodeIndexByID(unsigned int) const;
    s32 GetChild(int, int) const;
    void BuildPushPopFlags(int, int, int&);
    static cSHierarchy* Initialize(nlChunk*);
    static bool IsValidChunkID(u32 id)
    {
        return (id & 0x80FFFFFF) == 0x80018000;
    }
    inline u32* GetNodeIDs() const { return m_nodeIDs; }
    inline s32 GetNodeCount() const { return m_nodeCount; }

    /* 0x08 */ s32 m_nodeCount;
    /* 0x0C */ u32* m_nodeIDs;
    /* 0x10 */ s32* m_parentIndices;
    /* 0x14 */ s32* m_childCounts;
    /* 0x18 */ s32** m_childArrays;
    /* 0x1C */ s32* m_pushPopFlags;
    /* 0x20 */ s32* m_mirroredNodeIndices;
    /* 0x24 */ s32 m_nPelvisNodeIndex;
    /* 0x28 */ s32 m_nSpineNodeIndex;
    /* 0x2C */ nlVector3* m_translationOffsets;
    /* 0x30 */ u8* m_boneLengthFlags;
};

#endif // _SHIERARCHY_H_
