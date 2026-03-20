#include "Game/FE/tlInstance.h"
#include "NL/nlMath.h"

#include "math.h"

/**
 * Offset/Address/Size: 0x0 | 0x8020FF80 | size: 0x18
 */
void TLInstance::SetAssetColour(const nlColour& color)
{
    m_overloadFlags |= 0x10;
    m_overloadedAttributes.colour = color;
}

/**
 * Offset/Address/Size: 0x18 | 0x8020FF98 | size: 0x1C
 */
void TLInstance::SetAssetScale(float x, float y, float z)
{
    this->m_overloadFlags |= 4;
    m_overloadedAttributes.v3Scale.f.x = x;
    m_overloadedAttributes.v3Scale.f.y = y;
    m_overloadedAttributes.v3Scale.f.z = z;
}

/**
 * Offset/Address/Size: 0x34 | 0x8020FFB4 | size: 0x1C
 */
void TLInstance::SetAssetRotation(float x, float y, float z)
{
    this->m_overloadFlags |= 2;
    m_overloadedAttributes.v3Rotation.f.x = x;
    m_overloadedAttributes.v3Rotation.f.y = y;
    m_overloadedAttributes.v3Rotation.f.z = z;
}

/**
 * Offset/Address/Size: 0x50 | 0x8020FFD0 | size: 0x1C
 */
void TLInstance::SetAssetPosition(float x, float y, float z)
{
    this->m_overloadFlags |= 1;
    m_overloadedAttributes.v3Position.f.x = x;
    m_overloadedAttributes.v3Position.f.y = y;
    m_overloadedAttributes.v3Position.f.z = z;
}

/**
 * Offset/Address/Size: 0x6C | 0x8020FFEC | size: 0x8C
 */
bool TLInstance::IsValidAtTime(float arg0)
{
    float temp_f0;
    float temp_f2;
    float temp_f3;
    bool var_r0;

    var_r0 = true;
    temp_f0 = arg0 - m_fStartTime;
    if (!(temp_f0 > 0.0001f))
    {
        if (!((float)fabs(temp_f0) <= 0.0001f))
        {
            var_r0 = false;
        }
    }

    if (var_r0 != 0)
    {
        var_r0 = 1;
        temp_f2 = m_fDuration;
        temp_f3 = arg0 - m_fStartTime;
        if (!((m_fDuration - temp_f3) > 0.0001f))
        {
            if (!((float)fabs(temp_f3 - temp_f2) <= 0.0001f))
            {
                var_r0 = false;
            }
        }

        if (var_r0)
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0xF8 | 0x80210078 | size: 0x38
 */
const nlColour& TLInstance::GetColour() const
{
    if (m_overloadFlags & 0x10)
    {
        return m_overloadedAttributes.colour;
    }
    return m_component->GetColour();
}

/**
 * Offset/Address/Size: 0x130 | 0x802100B0 | size: 0x38
 */
const feVector3& TLInstance::GetScale() const
{
    if (m_overloadFlags & 0x4)
    {
        return m_overloadedAttributes.v3Scale;
    }
    return m_component->GetScale();
}

/**
 * Offset/Address/Size: 0x168 | 0x802100E8 | size: 0x38
 */
const feVector3& TLInstance::GetRotation() const
{
    if (m_overloadFlags & 0x2)
    {
        return m_overloadedAttributes.v3Rotation;
    }
    return m_component->GetRotation();
}

/**
 * Offset/Address/Size: 0x1A0 | 0x80210120 | size: 0x38
 */
const feVector3& TLInstance::GetPosition() const
{
    if (m_overloadFlags & 0x1)
    {
        return m_overloadedAttributes.v3Position;
    }
    return m_component->GetPosition();
}

/**
 * Offset/Address/Size: 0x1D8 | 0x80210158 | size: 0x8
 */
const nlColour& TLInstance::GetAssetColour() const
{
    return m_overloadedAttributes.colour;
}

/**
 * Offset/Address/Size: 0x1E0 | 0x80210160 | size: 0x8
 */
const feVector3& TLInstance::GetAssetScale() const
{
    return m_overloadedAttributes.v3Scale;
}

/**
 * Offset/Address/Size: 0x1E8 | 0x80210168 | size: 0x8
 */
const feVector3& TLInstance::GetAssetPosition() const
{
    return m_overloadedAttributes.v3Position;
}

// eTimeLineAssetType TLInstance::GetType() const
// {
//     return m_type;
// }
