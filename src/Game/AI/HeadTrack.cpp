#include "Game/AI/HeadTrack.h"

#include "math.h"

static f32 CANT_COLLIDE = *(f32*)__float_max;

/**
 * Offset/Address/Size: 0x5B0 | 0x800573B4 | size: 0x70
 */
cHeadTrack::cHeadTrack()
{
    m_m4HeadMatrix.SetIdentity();
    nlVec3Set(m_v3OOI, 0.0f, 0.0f, 0.0f);
    m_bTrackOOI = true;
    m_fHeadSpin = 0.0f;
    m_fHeadTilt = 0.0f;
    m_fDesiredHeadSpin = 0.0f;
    m_fDesiredHeadTilt = 0.0f;
    m_fHeadSpinSeekVel = 0.0f;
    m_fHeadTiltSeekVel = 0.0f;
}

/**
 * Offset/Address/Size: 0x160 | 0x80056F64 | size: 0x450
 * TODO: 98.19% match - 1 extra instruction causes branch target offset diff (beq 36c vs 370)
 */
void cHeadTrack::Update(const nlMatrix4& m4HeadMatrix, const nlMatrix4& m4ConstraintMatrix, float fDeltaT, unsigned short aOOIConstraint, int nHeadSpinMax, int nHeadTiltMax, float fSmoothTime)
{
    nlMatrix4 m4Constrain;
    nlMatrix4 m4WorldSpaceToConstraintSpace;
    nlVector3 v3OOIConstraintSpace;
    int nHeadSpin;
    int nHeadTilt;
    int nAmountOfDeadZoneBehindHeadtrack;

    if (m_bTrackOOI)
    {
        m4Constrain = m4ConstraintMatrix;
        float headM41 = m4HeadMatrix.f.m41;
        float headM42 = m4HeadMatrix.f.m42;
        float headM43 = m4HeadMatrix.f.m43;

        m4Constrain.f.m41 = headM41;
        m4Constrain.f.m42 = headM42;
        m4Constrain.f.m43 = headM43;
        m4Constrain.f.m44 = 1.0f;

        nlInvertRotTransMatrix(m4WorldSpaceToConstraintSpace, m4Constrain);
        nlMultPosVectorMatrix(v3OOIConstraintSpace, m_v3OOI, m4WorldSpaceToConstraintSpace);

        {
            float x2 = v3OOIConstraintSpace.f.x * v3OOIConstraintSpace.f.x;
            float y2 = v3OOIConstraintSpace.f.y * v3OOIConstraintSpace.f.y;
            float z2 = v3OOIConstraintSpace.f.z * v3OOIConstraintSpace.f.z;
            float invLen = nlRecipSqrt(x2 + y2 + z2, true);
            nlVec3Scale(v3OOIConstraintSpace, invLen);
        }

        nHeadSpin = ((int)(10430.378f * nlATan2f(v3OOIConstraintSpace.f.z, v3OOIConstraintSpace.f.y)) << 16) >> 16;
        nHeadTilt = 0x4000 - nlACos(-v3OOIConstraintSpace.f.x);
        nHeadTilt = (nHeadTilt << 16) >> 16;

        {
            int sign = nHeadSpin >> 31;
            int absSpin = (nHeadSpin ^ sign) - sign;

            if ((absSpin < (int)(unsigned int)aOOIConstraint) || (m_v3OOI.f.z > 1.5f))
            {
                nHeadSpin = (int)(float)nHeadSpin;
                nHeadTilt = (int)(0.5f * (float)nHeadTilt);
                nAmountOfDeadZoneBehindHeadtrack = (((int)(unsigned int)aOOIConstraint - nHeadSpinMax) * 3) / 4;

                sign = nHeadSpin >> 31;
                absSpin = (nHeadSpin ^ sign) - sign;
                if ((unsigned int)absSpin >= (unsigned int)nHeadSpinMax)
                {
                    if (nHeadSpin > 0)
                    {
                        if (m_fDesiredHeadSpin < 0.0f)
                        {
                            if (nHeadSpin > (nHeadSpinMax + nAmountOfDeadZoneBehindHeadtrack))
                                nHeadSpin = -nHeadSpinMax;
                            else
                                nHeadSpin = nHeadSpinMax;
                        }
                        else
                            nHeadSpin = nHeadSpinMax;
                    }
                    else if (m_fDesiredHeadSpin > 0.0f)
                    {
                        if (nHeadSpin < -(nHeadSpinMax + nAmountOfDeadZoneBehindHeadtrack))
                            nHeadSpin = nHeadSpinMax;
                        else
                            nHeadSpin = -nHeadSpinMax;
                    }
                    else
                        nHeadSpin = -nHeadSpinMax;
                }
            }
            else
            {
                nHeadSpin = 0;
                nHeadTilt = 0;
            }
        }

        if (nHeadTilt > nHeadTiltMax)
            nHeadTilt = nHeadTiltMax;

        if (&m4HeadMatrix != &m4ConstraintMatrix)
        {
            float headM21 = m4HeadMatrix.f.m21;
            float headM22 = m4HeadMatrix.f.m22;
            float constraintAtan = nlATan2f(m4Constrain.f.m22, m4Constrain.f.m21);
            float headAtan = nlATan2f(headM22, headM21);
            unsigned short spinConstraint = (unsigned short)(int)(10430.378f * constraintAtan);
            unsigned short spinHead = (unsigned short)(int)(10430.378f * headAtan);
            nHeadSpin += (short)(spinConstraint - spinHead);
        }
        m_fDesiredHeadSpin = nHeadSpin;
        m_fDesiredHeadTilt = nHeadTilt;
    }
    else
    {
        m_fDesiredHeadSpin = 0.0f;
        m_fDesiredHeadTilt = 0.0f;
    }

    float omega = 2.0f / fSmoothTime;
    float x = omega * fDeltaT;
    float exp = 1.0f / ((x * (0.235f * x * x)) + ((0.48f * x * x) + (1.0f + x)));

    float spinChange = m_fHeadSpin - m_fDesiredHeadSpin;
    float spinVel = m_fHeadSpinSeekVel;
    float spinTemp = fDeltaT * ((spinChange * omega) + spinVel);

    m_fHeadSpinSeekVel = exp * (spinVel - (omega * spinTemp));
    m_fHeadSpin = (exp * (spinChange + spinTemp)) + m_fDesiredHeadSpin;

    float tiltChange = m_fHeadTilt - m_fDesiredHeadTilt;
    float tiltVel = m_fHeadTiltSeekVel;
    float tiltTemp = fDeltaT * ((tiltChange * omega) + tiltVel);

    m_fHeadTiltSeekVel = exp * (tiltVel - (omega * tiltTemp));
    m_fHeadTilt = (exp * (tiltChange + tiltTemp)) + m_fDesiredHeadTilt;
}

inline float AngUnitsToRad_fromUnsignedShort(unsigned short sUnits)
{
    return (float)sUnits * 0.0000958738f;
}

/**
 * Offset/Address/Size: 0x0 | 0x80056E04 | size: 0x160
 */
void CalcHeadTrackMatrix(unsigned short spin, unsigned short tilt, cPoseAccumulator* cPoseAccumulator, int headNodeIndex)
{
    nlMatrix4 m4Intermediate;
    nlMatrix4 m4RotMatrix;
    nlMatrix4 m4NewHeadMatrix;

    const class nlMatrix4& m4AnimatedHeadMatrix = cPoseAccumulator->GetNodeMatrix(headNodeIndex);

    nlMakeRotationMatrixX(m4RotMatrix, AngUnitsToRad_fromUnsignedShort(spin));
    nlMultMatrices(m4Intermediate, m4RotMatrix, m4AnimatedHeadMatrix);
    nlMakeRotationMatrixZ(m4RotMatrix, AngUnitsToRad_fromUnsignedShort(tilt));
    nlMultMatrices(m4NewHeadMatrix, m4RotMatrix, m4Intermediate);

    cPoseAccumulator->m_NodeMatrices.mData[headNodeIndex] = m4NewHeadMatrix;
}
