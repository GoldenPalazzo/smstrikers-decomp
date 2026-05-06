#include "Game/SAnim/AnimRetargeter.h"

/**
 * Offset/Address/Size: 0x0 | 0x801EFF90 | size: 0x48
 */
static inline AnimRetarget* GetAnimRetargetWithSignature_ARL(AnimRetargetList* list, const cSAnim* anim)
{
    long offset;
    AnimRetarget* p;
    AnimRetarget* result = NULL;
    offset = (long)result;

    for (long i = list->m_NumAnimRetargets; i > 0; i--)
    {
        p = (AnimRetarget*)((char*)list->m_pAnimRetarget + offset);
        if (anim->m_nHierarchySignature == p->m_TargetHierarchySignature)
        {
            result = p;
            break;
        }
        offset += sizeof(AnimRetarget);
    }

    return result;
}

AnimRetarget* AnimRetargetList::GetAnimRetargetWithSignature(const cSAnim* anim)
{
    return GetAnimRetargetWithSignature_ARL(this, anim);
}
