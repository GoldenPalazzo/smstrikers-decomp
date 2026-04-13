#include "Game/FE/feSoundKeyframeTrigger.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/feAnimation.h"
#include "Game/FE/tlInstance.h"

#include "NL/nlDLRing.h"

inline fAnimationKeyframe* GetAnimationKeyframe(fAnimationKeyframe* start, float t)
{
    fAnimationKeyframe* it = start;
    do
    {
        if (t <= it->pKeyFrameData.m_fTime)
        {
            return it;
        }
        it = it->m_next;
    } while (it != start);
    return NULL;
}

/**
 * Offset/Address/Size: 0x0 | 0x8010EF64 | size: 0x1A8
 */
void SoundKeyframeTrigger::Update(float previoustime, float currenttime)
{
    fAnimationKeyframe* iter;
    FEAnimation* animhead;
    FEAnimation* animnode;
    fAnimationKeyframe* startkey;

    if (previoustime == currenttime)
        return;

    animhead = m_slide->m_animations;
    animnode = nlDLRingGetStart(animhead);

loop_start:
    if (animnode == NULL)
        goto loop_end;

    if (animnode->m_cast_type == 0 && animnode->m_type == eAnimOpacity)
    {
        fAnimationKeyframe* keyframehead = nlDLRingGetStart((fAnimationKeyframe*)animnode->m_DLRingHead);
        fAnimationKeyframe* testkey = keyframehead;

        do
        {
            if (previoustime <= testkey->pKeyFrameData.m_fTime)
            {
                startkey = testkey;
                goto found_startkey;
            }
            testkey = testkey->m_next;
        } while (testkey != keyframehead);
        startkey = NULL;
    found_startkey:

        f32 threshold = 0.0f;
        iter = startkey;
        while (startkey != NULL)
        {
            if (previoustime > currenttime && currenttime > iter->pKeyFrameData.m_fTime)
            {
                previoustime = 0.0f;
            }

            if (!(previoustime <= iter->pKeyFrameData.m_fTime))
                break;
            if (!(currenttime >= iter->pKeyFrameData.m_fTime))
                break;

            if (iter->pKeyFrameData.m_fPoint >= threshold)
            {
                char* name;
                TLInstance* instance;

                instance = animnode->m_pTLInstanceTarget;
                name = instance->m_szName;
                if (name != NULL)
                {
                    if (name[0] == 's' && name[1] == 'f' && name[2] == 'x')
                    {
                        instance->m_bVisible = false;
                        FEAudio::PlayAnimAudioEvent(instance->m_hashID, false);
                    }
                }
            }

            iter = iter->m_next;
            if (iter == startkey)
                break;
        }
    }

    if (nlDLRingIsEnd(animhead, animnode))
        goto loop_end;
    animnode = animnode->m_next;
    goto loop_start;

loop_end:;
}
