#ifndef _NET_H_
#define _NET_H_

#include "NL/nlMath.h"

class cNet
{
public:
    cNet(int);
    ~cNet();
    float GetGoalLineX() const;
    void GetPostLocation(nlVector3&, unsigned int, float) const;
    static void SetNetDimensions(float, float, float, float);
    static float GetNetHeight()
    {
        FORCE_DONT_INLINE;
        return m_fNetHeight;
    };
    static float GetNetWidth()
    {
        FORCE_DONT_INLINE;
        return m_fNetWidth;
    };
    static float GetPostRadius()
    {
        FORCE_DONT_INLINE;
        return m_fNetPostRadius;
    }

    static float m_fNetHeight;
    static float m_fNetWidth;
    static float m_fNetDepth;
    static float m_fNetPostRadius;
    static float m_fNetPostOffsetFromGoalLine;

    /* 0x00 */ s32 m_side;
    /* 0x04 */ f32 m_sideSign;
    /* 0x08 */ nlVector3 m_baseLocation;
}; // total size: 0x14

#endif // _NET_H_
