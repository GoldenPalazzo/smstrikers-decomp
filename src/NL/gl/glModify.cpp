#include "NL/gl/glModify.h"
#include "NL/gl/glMemory.h"
#include <string.h>

struct Modifier
{
    /* 0x00 */ eGLModifier m_modifier;
    /* 0x04 */ s32 m_unk_0x04;
    /* 0x08 */ s32 m_unk_0x08;
};

Modifier glModifier[6];
static u32 glNumModifiers = 0;

__declspec(section ".sdata2") static u32 GLTT_Diffuse_bit = 0x00000001;
__declspec(section ".sdata2") static u32 GLTT_Gloss_bit = 0x00000010;

/**
 * Offset/Address/Size: 0x0 | 0x801D903C | size: 0x30
 */
void gl_ModifyAddMapping(eGLModifier arg0, unsigned long arg1)
{
    glModifier[glNumModifiers].m_modifier = arg0;
    glModifier[glNumModifiers].m_unk_0x04 = -1;
    glModifier[glNumModifiers].m_unk_0x08 = arg1;
    glNumModifiers++;
}

/**
 * Offset/Address/Size: 0x30 | 0x801D906C | size: 0x2C
 */
void gl_ModifyAddMapping(eGLModifier arg0, unsigned long arg1, unsigned long arg2)
{
    glModifier[glNumModifiers].m_modifier = arg0;
    glModifier[glNumModifiers].m_unk_0x04 = arg1;
    glModifier[glNumModifiers].m_unk_0x08 = arg2;
    glNumModifiers++;
}

/**
 * Offset/Address/Size: 0x5C | 0x801D9098 | size: 0x1C
 */
void gl_ModifyClearLastMapping()
{
    s32 temp_r0;

    temp_r0 = glNumModifiers - 1;
    glNumModifiers = temp_r0;
    if (temp_r0 < 0)
    {
        glNumModifiers = 0;
    }
}

/**
 * Offset/Address/Size: 0x78 | 0x801D90B4 | size: 0xC
 */
void gl_ModifyClearMappings()
{
    glNumModifiers = 0;
}

/**
 * Offset/Address/Size: 0x84 | 0x801D90C0 | size: 0x1F8
 * TODO: 96.9% match - MWCC hoists non-const SDA2 loads before struct-pointer loads (3 sites).
 *       Target had const .sdata2 without folding; our MWCC version folds const, so __declspec forces section.
 */
static inline glModelPacket* gl_ModifyClonePacket(const glModelPacket* pPacket)
{
    glModelPacket* packet = (glModelPacket*)glFrameAlloc(0x4A, GLM_Header);
    memcpy(packet, pPacket, 0x4A);
    return packet;
}

glModelPacket* gl_Modify(const glModelPacket* pPacket)
{
    glModelPacket* newPacket = NULL;
    s32 i;

    for (i = 0; i < (s32)glNumModifiers; i++)
    {
        switch (glModifier[i].m_modifier)
        {
        case eGLModifier_0:
            if (pPacket->state.program == glModifier[i].m_unk_0x04)
            {
                if (newPacket == NULL)
                {
                    newPacket = gl_ModifyClonePacket(pPacket);
                }
                newPacket->state.program = glModifier[i].m_unk_0x08;
            }
            break;

        case eGLModifier_1:
            if ((u32)glModifier[i].m_unk_0x04 == 0xFFFFFFFF)
            {
                u32 tex;
                u32 bit;

                if (newPacket == NULL)
                {
                    newPacket = gl_ModifyClonePacket(pPacket);
                }

                tex = glModifier[i].m_unk_0x08;
                bit = GLTT_Diffuse_bit;
                newPacket->state.texture[0] = tex;
                newPacket->state.texconfig = (u8)(bit | newPacket->state.texconfig);
            }
            else if (pPacket->state.texture[0] == glModifier[i].m_unk_0x04)
            {
                u32 tex;
                u32 bit;

                if (newPacket == NULL)
                {
                    newPacket = gl_ModifyClonePacket(pPacket);
                }

                tex = glModifier[i].m_unk_0x08;
                bit = GLTT_Diffuse_bit;
                newPacket->state.texture[0] = tex;
                newPacket->state.texconfig = (u8)(bit | newPacket->state.texconfig);
            }
            break;

        case eGLModifier_2:
        {
            u32 tex;
            u32 bit;
            if (newPacket == NULL)
            {
                newPacket = gl_ModifyClonePacket(pPacket);
            }
            tex = glModifier[i].m_unk_0x08;
            bit = GLTT_Gloss_bit;
            newPacket->state.texture[4] = tex;
            newPacket->state.texconfig = (u8)(bit | newPacket->state.texconfig);
            break;
        }

        case eGLModifier_3:
            if (newPacket == NULL)
            {
                newPacket = gl_ModifyClonePacket(pPacket);
            }
            newPacket->state.texconfig &= glModifier[i].m_unk_0x08;
            break;
        }
    }

    return newPacket;
}

/**
 * Offset/Address/Size: 0x27C | 0x801D92B8 | size: 0x8
 */
u32 gl_ModifyGetNum()
{
    return glNumModifiers;
}
