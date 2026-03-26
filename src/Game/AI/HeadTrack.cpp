#include "Game/AI/HeadTrack.h"

#include "math.h"

f32 CANT_COLLIDE = *(f32*)__float_max;

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
 * TODO: 96.02% match - FP register allocation diffs (f0/f3 swap) in matrix copy and normalize
 * sections due to -inline deferred vs -inline auto compiler flag difference
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
        m4Constrain.f.m41 = m4HeadMatrix.f.m41;
        m4Constrain.f.m42 = m4HeadMatrix.f.m42;
        m4Constrain.f.m43 = m4HeadMatrix.f.m43;
        m4Constrain.f.m44 = 1.0f;

        nlInvertRotTransMatrix(m4WorldSpaceToConstraintSpace, m4Constrain);
        nlMultPosVectorMatrix(v3OOIConstraintSpace, m_v3OOI, m4WorldSpaceToConstraintSpace);

        {
            float x2 = v3OOIConstraintSpace.f.x * v3OOIConstraintSpace.f.x;
            float y2 = v3OOIConstraintSpace.f.y * v3OOIConstraintSpace.f.y;
            float z2 = v3OOIConstraintSpace.f.z * v3OOIConstraintSpace.f.z;
            float invLen = nlRecipSqrt(x2 + y2 + z2, true);
            float z = invLen * v3OOIConstraintSpace.f.z;
            float y = invLen * v3OOIConstraintSpace.f.y;
            float x = invLen * v3OOIConstraintSpace.f.x;

            v3OOIConstraintSpace.f.z = z;
            v3OOIConstraintSpace.f.x = x;
            v3OOIConstraintSpace.f.y = y;
        }

        nHeadSpin = ((int)(10430.378f * nlATan2f(v3OOIConstraintSpace.f.z, v3OOIConstraintSpace.f.y)) << 16) >> 16;
        nHeadTilt = ((0x4000 - nlACos(-v3OOIConstraintSpace.f.x)) << 16) >> 16;

        {
            int sign = nHeadSpin >> 31;
            int absSpin = (nHeadSpin ^ sign) - sign;

            if ((absSpin < (int)(unsigned int)aOOIConstraint) || (m_v3OOI.f.z > 1.5f))
            {
                nAmountOfDeadZoneBehindHeadtrack = (((int)(unsigned int)aOOIConstraint - nHeadSpinMax) * 3) / 4;

                nHeadSpin = (int)(float)nHeadSpin;
                nHeadTilt = (int)(0.5f * (float)nHeadTilt);

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
            float headM22 = m4HeadMatrix.f.m22;
            float headM21 = m4HeadMatrix.f.m21;
            float constraintAtan = nlATan2f(m4Constrain.f.m22, m4Constrain.f.m21);
            float headAtan = nlATan2f(headM22, headM21);
            unsigned short spinConstraint = (unsigned short)(int)(10430.378f * constraintAtan);
            unsigned short spinHead = (unsigned short)(int)(10430.378f * headAtan);
            nHeadSpin += ((spinConstraint - spinHead) << 16) >> 16;
        }
        m_fDesiredHeadSpin = nHeadSpin;
        m_fDesiredHeadTilt = nHeadTilt;
    }
    else
    {
        m_fDesiredHeadSpin = 0.0f;
        m_fDesiredHeadTilt = 0.0f;
    }

    float temp_f8 = 2.0f / fSmoothTime;
    float temp_f7 = m_fHeadSpinSeekVel;
    float temp_f6 = temp_f8 * fDeltaT;
    float temp_f5 = m_fHeadSpin - m_fDesiredHeadSpin;
    float temp_f2_2 = fDeltaT * ((temp_f8 * temp_f5) + temp_f7);
    float temp_f3_2 = 1.0f / ((temp_f6 * (0.235f * temp_f6 * temp_f6)) + ((0.48f * temp_f6 * temp_f6) + (1.0f + temp_f6)));
    m_fHeadSpinSeekVel = temp_f3_2 * -((temp_f8 * temp_f2_2) - temp_f7);
    m_fHeadSpin = (temp_f3_2 * (temp_f5 + temp_f2_2)) + m_fDesiredHeadSpin;
    float temp_f4 = m_fHeadTiltSeekVel;
    float temp_f2_3 = m_fHeadTilt - m_fDesiredHeadTilt;
    float temp_f1_3 = fDeltaT * ((temp_f8 * temp_f2_3) + temp_f4);
    m_fHeadTiltSeekVel = temp_f3_2 * -((temp_f8 * temp_f1_3) - temp_f4);
    m_fHeadTilt = (temp_f3_2 * (temp_f2_3 + temp_f1_3)) + m_fDesiredHeadTilt;
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
