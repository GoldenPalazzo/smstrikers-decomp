#include "Game/FE/feAnimation.h"
#include "NL/nlMath.h"
#include "NL/nlDLRing.h"

/**
 * Offset/Address/Size: 0x0 | 0x8020E580 | size: 0x19C
 */
void FEAnimation::Update(float arg0)
{
    f32 temp_f0;
    f32 temp_f2;
    f32 var_f31;
    fAnimationKeyframe* currentFrame;
    u16 temp_r0;
    fAnimationKeyframe* temp_r9;
    nlVector4 spC;
    const nlVector4 sZero = { 0.0f, 0.0f, 0.0f, 0.0f };

    temp_r0 = this->m_cast_type;
    switch (temp_r0)
    {
    case 1:
        AnimateTargetAtTimeWithVector3(arg0);
        return;

    case 0:
        currentFrame = nlDLRingGetStart<fAnimationKeyframe>((fAnimationKeyframe*)this->m_DLRingHead);
        if (currentFrame != currentFrame->m_next)
        {
            if (arg0 >= currentFrame->pKeyFrameData.m_fTime)
            {
                goto loop_check;

            loop_body:
                currentFrame = currentFrame->m_next;
                if (nlDLRingIsEnd<fAnimationKeyframe>((fAnimationKeyframe*)this->m_DLRingHead, currentFrame) != 0)
                {
                    goto loop_end;
                }

            loop_check:
                if (arg0 > currentFrame->pKeyFrameData.m_fTime)
                {
                    goto loop_body;
                }

            loop_end:
                temp_f2 = currentFrame->pKeyFrameData.m_fTime;
                if (arg0 == temp_f2)
                {
                    var_f31 = currentFrame->pKeyFrameData.m_fPoint;
                }
                else if (!(arg0 > temp_f2) || (currentFrame->pKeyFrameData.m_fControl1 != -1.0f))
                {
                    temp_r9 = currentFrame->m_prev;
                    temp_f0 = temp_r9->pKeyFrameData.m_fTime;
                    spC = sZero;
                    spC.f.x = temp_r9->pKeyFrameData.m_fPoint;
                    spC.f.y = temp_r9->pKeyFrameData.m_fControl1;
                    spC.f.z = temp_r9->pKeyFrameData.m_fControl2;
                    spC.f.w = currentFrame->pKeyFrameData.m_fPoint;
                    var_f31 = nlBezier(&spC.f.x, 3, (arg0 - temp_f0) / (temp_f2 - temp_f0));
                }
                else
                {
                    var_f31 = currentFrame->pKeyFrameData.m_fPoint;
                }

                if ((var_f31 != -1.0f) && (this->m_type == eAnimOpacity))
                {
                    nlColour newColor = m_pTLInstanceTarget->GetAssetColour();
                    newColor.c[3] = (u8)var_f31;
                    m_pTLInstanceTarget->SetAssetColour(newColor);
                }
            }
        }
        return;
    }
}

/**
 * Offset/Address/Size: 0x19C | 0x8020E71C | size: 0x298
 */
void FEAnimation::AnimateTargetAtTimeWithVector3(float fCurrentTime)
{
    v3AnimationKeyframe* currentFrame;
    f32 resultX, resultY, resultZ;

    currentFrame = nlDLRingGetStart<v3AnimationKeyframe>((v3AnimationKeyframe*)this->m_DLRingHead);

    if (fCurrentTime < currentFrame->pKeyFrameDataX.m_fTime)
    {
        fCurrentTime = currentFrame->pKeyFrameDataX.m_fTime;
    }

    resultX = -1.0f;
    while (fCurrentTime > currentFrame->pKeyFrameDataX.m_fTime
           && (resultX != currentFrame->pKeyFrameDataX.m_fControl1
               || resultX != currentFrame->pKeyFrameDataX.m_fControl2))
    {
        currentFrame = currentFrame->m_next;
        if (nlDLRingIsEnd<v3AnimationKeyframe>((v3AnimationKeyframe*)this->m_DLRingHead, currentFrame))
        {
            break;
        }
    }

    f32 currentTime = currentFrame->pKeyFrameDataX.m_fTime;

    if (fCurrentTime == currentTime)
    {
        resultX = currentFrame->pKeyFrameDataX.m_fPoint;
        resultY = currentFrame->pKeyFrameDataY.m_fPoint;
        resultZ = currentFrame->pKeyFrameDataZ.m_fPoint;
    }
    else if (!(fCurrentTime > currentTime) || currentFrame->pKeyFrameDataX.m_fControl1 != -1.0f)
    {
        v3AnimationKeyframe* prevFrame = currentFrame->m_prev;
        f32 prevTime = prevFrame->pKeyFrameDataX.m_fTime;

        float controlPointsX[4] = { 0 };
        controlPointsX[0] = prevFrame->pKeyFrameDataX.m_fPoint;
        controlPointsX[1] = prevFrame->pKeyFrameDataX.m_fControl1;
        controlPointsX[2] = prevFrame->pKeyFrameDataX.m_fControl2;
        controlPointsX[3] = currentFrame->pKeyFrameDataX.m_fPoint;

        float controlPointsY[4] = { 0 };
        controlPointsY[0] = prevFrame->pKeyFrameDataY.m_fPoint;
        controlPointsY[1] = prevFrame->pKeyFrameDataY.m_fControl1;
        controlPointsY[2] = prevFrame->pKeyFrameDataY.m_fControl2;
        controlPointsY[3] = currentFrame->pKeyFrameDataY.m_fPoint;

        float controlPointsZ[4] = { 0 };
        controlPointsZ[0] = prevFrame->pKeyFrameDataZ.m_fPoint;
        controlPointsZ[1] = prevFrame->pKeyFrameDataZ.m_fControl1;
        controlPointsZ[2] = prevFrame->pKeyFrameDataZ.m_fControl2;
        controlPointsZ[3] = currentFrame->pKeyFrameDataZ.m_fPoint;

        f32 fMu = (fCurrentTime - prevTime) / (currentTime - prevTime);
        resultX = nlBezier(controlPointsX, 3, fMu);
        resultY = nlBezier(controlPointsY, 3, fMu);
        resultZ = nlBezier(controlPointsZ, 3, fMu);
    }
    else
    {
        resultX = currentFrame->pKeyFrameDataX.m_fPoint;
        resultY = currentFrame->pKeyFrameDataY.m_fPoint;
        resultZ = currentFrame->pKeyFrameDataZ.m_fPoint;
    }

    switch (m_type)
    {
    case eAnimPosition:
        m_pTLInstanceTarget->SetAssetPosition(resultX, resultY, resultZ);
        break;
    case eAnimRotation:
        m_pTLInstanceTarget->SetAssetRotation(resultX, resultY, resultZ);
        break;
    case eAnimScale:
        m_pTLInstanceTarget->SetAssetScale(resultX, resultY, resultZ);
        break;
    }
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8020E9B4 | size: 0x20
//  */
// void nlDLRingIsEnd<v3AnimationKeyframe>(v3AnimationKeyframe*, v3AnimationKeyframe*)
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x8020E9D4 | size: 0x20
//  */
// void nlDLRingIsEnd<fAnimationKeyframe>(fAnimationKeyframe*, v3AnimationKeyframe*)
// {
// }

// /**
//  * Offset/Address/Size: 0x40 | 0x8020E9F4 | size: 0x18
//  */
// void nlDLRingGetStart<v3AnimationKeyframe>(v3AnimationKeyframe*)
// {
// }
