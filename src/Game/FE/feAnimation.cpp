#include "Game/FE/feAnimation.h"
#include "NL/nlMath.h"
#include "NL/nlDLRing.h"

nlVector3_ ZeroVector3 = { 0.0f, 0.0f, 0.0f };

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
    static const nlVector4 sZero = { 0.0f, 0.0f, 0.0f, 0.0f };

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
 * TODO: 97.98% match - f29/f31 FPR register swap: compiler puts fCurrentTime in f31 (should be f29) and sentinel/X in f29 (should be f31)
 */
void FEAnimation::AnimateTargetAtTimeWithVector3(float fCurrentTime)
{
    nlVector4 controlPointsX;
    nlVector4 controlPointsY;
    nlVector4 controlPointsZ;
    f32 temp_f0;
    f32 temp_f2;
    f32 var_f30;
    f32 var_f31;
    f32 var_f3;
    v3AnimationKeyframe* currentFrame;
    v3AnimationKeyframe* prevFrame;
    static const nlVector4 sZeroX = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const nlVector4 sZeroY = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const nlVector4 sZeroZ = { 0.0f, 0.0f, 0.0f, 0.0f };

    currentFrame = nlDLRingGetStart<v3AnimationKeyframe>((v3AnimationKeyframe*)this->m_DLRingHead);
    temp_f0 = currentFrame->pKeyFrameDataX.m_fTime;
    if (fCurrentTime < temp_f0)
    {
        fCurrentTime = temp_f0;
    }

    var_f31 = -1.0f;
    while (fCurrentTime > currentFrame->pKeyFrameDataX.m_fTime && (var_f31 != currentFrame->pKeyFrameDataX.m_fControl1 || var_f31 != currentFrame->pKeyFrameDataX.m_fControl2))
    {
        currentFrame = currentFrame->m_next;
        if (nlDLRingIsEnd<v3AnimationKeyframe>((v3AnimationKeyframe*)this->m_DLRingHead, currentFrame))
        {
            break;
        }
    }

    temp_f2 = currentFrame->pKeyFrameDataX.m_fTime;
    if (fCurrentTime == temp_f2)
    {
        var_f31 = currentFrame->pKeyFrameDataX.m_fPoint;
        var_f30 = currentFrame->pKeyFrameDataY.m_fPoint;
        var_f3 = currentFrame->pKeyFrameDataZ.m_fPoint;
    }
    else if (!(fCurrentTime > temp_f2) || (currentFrame->pKeyFrameDataX.m_fControl1 != -1.0f))
    {
        prevFrame = currentFrame->m_prev;
        temp_f0 = prevFrame->pKeyFrameDataX.m_fTime;

        controlPointsX = sZeroX;
        controlPointsX.f.x = prevFrame->pKeyFrameDataX.m_fPoint;
        controlPointsX.f.y = prevFrame->pKeyFrameDataX.m_fControl1;
        controlPointsX.f.z = prevFrame->pKeyFrameDataX.m_fControl2;
        controlPointsX.f.w = currentFrame->pKeyFrameDataX.m_fPoint;

        controlPointsY = sZeroY;
        controlPointsY.f.x = prevFrame->pKeyFrameDataY.m_fPoint;
        controlPointsY.f.y = prevFrame->pKeyFrameDataY.m_fControl1;
        controlPointsY.f.z = prevFrame->pKeyFrameDataY.m_fControl2;
        controlPointsY.f.w = currentFrame->pKeyFrameDataY.m_fPoint;

        controlPointsZ = sZeroZ;
        controlPointsZ.f.x = prevFrame->pKeyFrameDataZ.m_fPoint;
        controlPointsZ.f.y = prevFrame->pKeyFrameDataZ.m_fControl1;
        controlPointsZ.f.z = prevFrame->pKeyFrameDataZ.m_fControl2;
        controlPointsZ.f.w = currentFrame->pKeyFrameDataZ.m_fPoint;

        fCurrentTime = (fCurrentTime - temp_f0) / (temp_f2 - temp_f0);
        var_f31 = nlBezier(&controlPointsX.f.x, 3, fCurrentTime);
        var_f30 = nlBezier(&controlPointsY.f.x, 3, fCurrentTime);
        var_f3 = nlBezier(&controlPointsZ.f.x, 3, fCurrentTime);
    }
    else
    {
        var_f31 = currentFrame->pKeyFrameDataX.m_fPoint;
        var_f30 = currentFrame->pKeyFrameDataY.m_fPoint;
        var_f3 = currentFrame->pKeyFrameDataZ.m_fPoint;
    }

    switch (this->m_type)
    {
    case eAnimPosition:
        m_pTLInstanceTarget->SetAssetPosition(var_f31, var_f30, var_f3);
        break;
    case eAnimRotation:
        m_pTLInstanceTarget->SetAssetRotation(var_f31, var_f30, var_f3);
        break;
    case eAnimScale:
        m_pTLInstanceTarget->SetAssetScale(var_f31, var_f30, var_f3);
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
