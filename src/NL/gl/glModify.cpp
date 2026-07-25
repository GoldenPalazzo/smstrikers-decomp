#include "NL/gl/glModify.h"
#include "NL/gl/glMemory.h"
#include <string.h>

struct Modifier
{
    /* 0x00 */ eGLModifier m_modifier;
    /* 0x04 */ s32 m_unk_0x04;
    /* 0x08 */ s32 m_unk_0x08;
};

static Modifier glModifier[6];
static u32 glNumModifiers = 0;

static const unsigned long GLTT_Diffuse_bit = 1UL << 0;
static const unsigned long GLTT_Gloss_bit = 1UL << 4;

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
                if (newPacket == NULL)
                {
                    newPacket = gl_ModifyClonePacket(pPacket);
                }
                newPacket->state.texture[0] = glModifier[i].m_unk_0x08;
                newPacket->state.texconfig |= GLTT_Diffuse_bit;
            }
            else if (pPacket->state.texture[0] == glModifier[i].m_unk_0x04)
            {
                if (newPacket == NULL)
                {
                    newPacket = gl_ModifyClonePacket(pPacket);
                }
                newPacket->state.texture[0] = glModifier[i].m_unk_0x08;
                newPacket->state.texconfig |= GLTT_Diffuse_bit;
            }
            break;

        case eGLModifier_2:
            if (newPacket == NULL)
            {
                newPacket = gl_ModifyClonePacket(pPacket);
            }
            newPacket->state.texture[4] = glModifier[i].m_unk_0x08;
            newPacket->state.texconfig |= GLTT_Gloss_bit;
            break;

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
