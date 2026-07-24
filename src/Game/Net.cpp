#include "Game/Net.h"
#include "Game/Field.h"

float cNet::m_fNetHeight = 1.0f;
float cNet::m_fNetWidth = 1.0f;
float cNet::m_fNetDepth = 2.5f;
float cNet::m_fNetPostRadius = 0.01f;
float cNet::m_fNetPostOffsetFromGoalLine = 0.0f;

/**
 * Offset/Address/Size: 0xE4 | 0x80057518 | size: 0x38
 */
cNet::cNet(int side)
{
    m_side = side;

    m_baseLocation.f.y = 0.0f;
    m_baseLocation.f.z = 0.0f;
    m_baseLocation.f.x = 0.0f;

    if (m_side == 0)
    {
        m_sideSign = -1.0f;
    }
    else
    {
        m_sideSign = +1.0f;
    }
}

/**
 * Offset/Address/Size: 0xA8 | 0x800574DC | size: 0x3C
 */
cNet::~cNet()
{
}

/**
 * Offset/Address/Size: 0x84 | 0x800574B8 | size: 0x24
 */
float cNet::GetGoalLineX() const
{
    return cField::GetGoalLineX((float)m_sideSign);
}

/**
 * Offset/Address/Size: 0x14 | 0x80057448 | size: 0x70
 */
void cNet::GetPostLocation(nlVector3& location, unsigned int side, float arg2) const
{
    f32 yOffset;
    f32 sideSign;

    location = m_baseLocation;
    sideSign = m_sideSign;
    location.f.x = -((m_fNetPostRadius * sideSign) - location.f.x);
    location.f.x += m_fNetPostOffsetFromGoalLine * sideSign;
    yOffset = (0.5f * m_fNetWidth) + arg2;

    if (side == 0)
    {
        location.f.y -= yOffset;
        return;
    }

    location.f.y += yOffset;
}

/**
 * Offset/Address/Size: 0x0 | 0x80057434 | size: 0x14
 */
void cNet::SetNetDimensions(float width, float height, float postRadius, float postOffsetFromGoalLine)
{
    m_fNetWidth = width;
    m_fNetHeight = height;
    m_fNetPostRadius = postRadius;
    m_fNetPostOffsetFromGoalLine = postOffsetFromGoalLine;
}
