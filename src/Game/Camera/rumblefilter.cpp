#include "Game/Camera/rumblefilter.h"

#include "NL/nlTask.h"
#include "Game/Sys/eventman.h"

/**
 * Offset/Address/Size: 0x3D8 | 0x801A65F4 | size: 0x60
 */
cRumbleFilter::cRumbleFilter()
{
    nlVec2Set(v2Pos0, 0.0f, 0.0f);
    nlVec2Set(v2Pos1, 0.0f, 0.0f);
    nlVec2Set(v2Vel0, 0.0f, 0.0f);
    nlVec2Set(v2Vel1, 0.0f, 0.0f);
    nlVec2Set(v2Force0, 0.0f, 0.0f);
    nlVec2Set(v2Force1, 0.0f, 0.0f);
    Ks = 5000.0f;
    Kd = 10.0f;
}

/**
 * Offset/Address/Size: 0x334 | 0x801A6550 | size: 0xA4
 */
void cRumbleFilter::Filter(const nlMatrix4& matViewIn, nlMatrix4& matViewOut)
{
    matViewOut = matViewIn;
    matViewOut.f.m41 += v2Pos1.f.x;
    matViewOut.f.m42 += v2Pos1.f.y;
}

/**
 * Offset/Address/Size: 0x2D4 | 0x801A64F0 | size: 0x60
 */
void cRumbleFilter::Reset()
{
    nlVec2Set(v2Pos0, 0.0f, 0.0f);
    nlVec2Set(v2Pos1, 0.0f, 0.0f);
    nlVec2Set(v2Vel0, 0.0f, 0.0f);
    nlVec2Set(v2Vel1, 0.0f, 0.0f);
    nlVec2Set(v2Force0, 0.0f, 0.0f);
    nlVec2Set(v2Force1, 0.0f, 0.0f);
    g_pEventManager->CreateValidEvent(0x58, 0x14);
}

/**
 * Offset/Address/Size: 0x0 | 0x801A621C | size: 0x284
 */
void cRumbleFilter::Update(float dt)
{
    float fHTerm;
    float fDTerm;

    if (nlTaskManager::m_pInstance->m_CurrState == 1)
        return;

    float step = 0.02f;
    if (dt <= step)
        step = dt;

    nlVector2 _p;
    _p.f.y = v2Pos0.f.y - v2Pos1.f.y;
    _p.f.x = v2Pos0.f.x - v2Pos1.f.x;
    const float temp = (_p.f.x * _p.f.x) + (_p.f.y * _p.f.y);
    const float len = nlSqrt(temp, true);
    fHTerm = len * Ks;

    nlVector2 _dv;
    _dv.f.y = v2Vel0.f.y - v2Vel1.f.y;
    _dv.f.x = v2Vel0.f.x - v2Vel1.f.x;
    const float proj = (_dv.f.x * _p.f.x) + (_dv.f.y * _p.f.y);
    if (len == 0.0f)
    {
        fDTerm = 0.0f;
    }
    else
    {
        fDTerm = (proj * Kd) / len;
    }

    float ux, uy;
    ux = 0.0f;
    if (len == 0.f)
    {
        uy = 0.0f;
    }
    else
    {
        static bool bIsBouncing = false;

        if (len < 0.01f)
        {
            if (bIsBouncing)
            {
                g_pEventManager->CreateValidEvent(0x58, 0x14);
                bIsBouncing = false;
            }
        }
        else
        {
            bIsBouncing = true;
        }

        const float invLen = 1.0f / len;
        ux = invLen * _p.f.x;
        uy = invLen * _p.f.y;
    }

    nlVec2Set(v2Force0, 0.f, 0.f);
    nlVec2Set(v2Force1, 0.f, 0.f);

    float total = -(fHTerm + fDTerm);
    float fx = total * ux;
    float fy = total * uy;
    nlVec2Set(v2Force0,
        v2Force0.f.x + fx,
        v2Force0.f.y + fy);
    nlVec2Set(v2Force1,
        v2Force1.f.x - fx,
        v2Force1.f.y - fy);

    float factor0 = 0.f;
    nlVec2Set(v2Vel0,
        v2Vel0.f.x + (step * (factor0 * v2Force0.f.x)),
        v2Vel0.f.y + (step * (factor0 * v2Force0.f.y)));
    nlVec2Set(v2Pos0,
        v2Pos0.f.x + (step * v2Vel0.f.x),
        v2Pos0.f.y + (step * v2Vel0.f.y));

    float factor1 = 1.f;
    nlVec2Set(v2Vel1,
        v2Vel1.f.x + (step * (factor1 * v2Force1.f.x)),
        v2Vel1.f.y + (step * (factor1 * v2Force1.f.y)));
    nlVec2Set(v2Pos1,
        v2Pos1.f.x + (step * v2Vel1.f.x),
        v2Pos1.f.y + (step * v2Vel1.f.y));
}
