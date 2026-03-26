#include "Game/Physics/PhysicsGoalie.h"
#include "Game/Physics/PhysicsBall.h"
#include "Game/Player.h"
#include "Game/Team.h"
#include "Game/AI/AiUtil.h"
#include "Game/FixedUpdateTask.h"
#include "NL/utility.h"
#include "types.h"

f32 CANT_COLLIDE = *(f32*)__float_max;

float ballMaxMotionPerTick = PhysicsBall::GetBallMaxVelocity() * FixedUpdateTask::GetPhysicsUpdateTick();

/**
 * Offset / Address / Size : 0x8F0 | 0x8013A370 | size : 0x34
 */
void PhysicsGoalie::PostUpdate()
{
    PhysicsCharacter::PostUpdate();
    CollideGoalieWithPost();
}

/**
 * Offset/Address/Size: 0x780 | 0x8013A200 | size: 0x170
 */
bool PhysicsGoalie::SweepTestForBallContact(const nlVector3& ballPrevPosition, const nlVector3& ballCurrentPosition, const nlVector3& velocity, float ballRadius, nlVector3& outContactPos, nlVector3& outContactNormal) const
{
    int testsPassed = 0;
    float goalieRadius = 4.0f * m_CentreOfMassHeight;

    nlVector3 goaliePos;
    GetPosition(&goaliePos);
    goaliePos.f.z = (2.0 * m_CentreOfMassHeight) + goaliePos.f.z;

    nlVector3 tmp;
    nlVec3Set(tmp, ballPrevPosition.f.x - goaliePos.f.x, ballPrevPosition.f.y - goaliePos.f.y, ballPrevPosition.f.z - goaliePos.f.z);

    if ((nlSqrt((tmp.f.x * tmp.f.x) + (tmp.f.y * tmp.f.y) + (tmp.f.z * tmp.f.z), true) - (goalieRadius + (ballRadius + ballMaxMotionPerTick))) <= 0.0f)
    {
        testsPassed = 1;
        float temp_f1_2 = SweepSpheres(ballRadius, ballPrevPosition, ballCurrentPosition, goalieRadius, goaliePos, goaliePos);

        bool isValidSweep = false;
        if ((temp_f1_2 == CANT_COLLIDE) || (temp_f1_2 < 0.0f) || (temp_f1_2 > 1.0f))
        {
            isValidSweep = 0;
        }
        else
        {
            isValidSweep = 1;
        }

        if (isValidSweep != 0)
        {
            testsPassed = 2;
            if (SweepTestEveryBone(ballRadius, ballPrevPosition, ballCurrentPosition, outContactNormal, outContactPos) != 0)
            {
                testsPassed = 3;
            }
        }
    }

    return testsPassed == 3;
}

/**
 * Offset/Address/Size: 0x4A8 | 0x80139F28 | size: 0x2D8
 * TODO: 83.10% match - callee-saved register mapping and f30/f31 lifetime ordering still differ.
 */
bool PhysicsGoalie::SweepTestEveryBone(float ballRadius, const nlVector3& ballPrevPosition, const nlVector3& ballCurrentPosition, nlVector3& outContactNormal, nlVector3& outContactPos) const
{
    ListEntry<PhysicsBoneVolume*>* boneVolumeEntry = m_BoneVolumes.m_Head;
    nlVector3 accumulatedNormal = { 0.0f, 0.0f, 0.0f };
    float smallestSweepResult = 1.0f;

    bool didHitBone = false;
    int hitCount = 0;

    if (boneVolumeEntry == NULL)
    {
        return false;
    }

    float cantCollide = CANT_COLLIDE;
    while (boneVolumeEntry != NULL)
    {
        PhysicsBoneVolume* boneVolume = boneVolumeEntry->data;
        PhysicsObject* object = boneVolume->m_pObject;
        nlVector3& currentBonePos = object->GetPosition();
        nlVector3& prevBonePos = boneVolume->m_PrevPosition;
        float sweepResult = SweepSpheres(
            ballRadius,
            ballPrevPosition,
            ballCurrentPosition,
            ((PhysicsSphere*)object)->GetRadius(),
            prevBonePos,
            currentBonePos);

        if ((sweepResult != cantCollide) && (sweepResult > 0.0f) && (sweepResult < 1.0f))
        {
            if (sweepResult < smallestSweepResult)
            {
                smallestSweepResult = sweepResult;
            }

            float oneMinusSweepResult = 1.0f - sweepResult;

            float posX = (sweepResult * ballCurrentPosition.f.x) + (oneMinusSweepResult * ballPrevPosition.f.x);
            float posY = (sweepResult * ballCurrentPosition.f.y) + (oneMinusSweepResult * ballPrevPosition.f.y);
            float posZ = (sweepResult * ballCurrentPosition.f.z) + (oneMinusSweepResult * ballPrevPosition.f.z);
            outContactPos.f.x = posX;
            outContactPos.f.y = posY;
            outContactPos.f.z = posZ;

            float bonePosX = (sweepResult * currentBonePos.f.x) + (oneMinusSweepResult * prevBonePos.f.x);
            float bonePosY = (sweepResult * currentBonePos.f.y) + (oneMinusSweepResult * prevBonePos.f.y);
            float bonePosZ = (sweepResult * currentBonePos.f.z) + (oneMinusSweepResult * prevBonePos.f.z);

            outContactNormal.f.x = posX - bonePosX;
            outContactNormal.f.y = posY - bonePosY;
            outContactNormal.f.z = posZ - bonePosZ;

            float normalRecipLength = nlRecipSqrt((outContactNormal.f.x * outContactNormal.f.x) + (outContactNormal.f.y * outContactNormal.f.y) + (outContactNormal.f.z * outContactNormal.f.z),
                true);

            outContactNormal.f.x = normalRecipLength * outContactNormal.f.x;
            outContactNormal.f.y = normalRecipLength * outContactNormal.f.y;
            outContactNormal.f.z = normalRecipLength * outContactNormal.f.z;

            didHitBone = true;
            hitCount += 1;

            accumulatedNormal.f.z = accumulatedNormal.f.z + outContactNormal.f.z;
            accumulatedNormal.f.x = accumulatedNormal.f.x + outContactNormal.f.x;
            accumulatedNormal.f.y = accumulatedNormal.f.y + outContactNormal.f.y;
        }

        boneVolumeEntry = boneVolumeEntry->next;
    }

    if (didHitBone)
    {
        float oneMinusSweepResult = 1.0f - smallestSweepResult;
        float invHitCount = 1.0f / (float)hitCount;

        outContactNormal.f.x = invHitCount * accumulatedNormal.f.x;
        outContactNormal.f.y = invHitCount * accumulatedNormal.f.y;
        outContactNormal.f.z = invHitCount * accumulatedNormal.f.z;

        outContactPos.f.x = (smallestSweepResult * ballCurrentPosition.f.x) + (oneMinusSweepResult * ballPrevPosition.f.x);
        outContactPos.f.y = (smallestSweepResult * ballCurrentPosition.f.y) + (oneMinusSweepResult * ballPrevPosition.f.y);
        outContactPos.f.z = (smallestSweepResult * ballCurrentPosition.f.z) + (oneMinusSweepResult * ballPrevPosition.f.z);
    }

    return didHitBone;
}

/**
 * Offset/Address/Size: 0x70 | 0x80139AF0 | size: 0x438
 * TODO: 93.67% match - callee-saved float register allocation (f28/f27 vs f31/f30) cannot be controlled from C source.
 */
void PhysicsGoalie::CollideGoalieWithPost()
{
    cPlayer* pGoalie = (cPlayer*)m_pAICharacter;
    nlVector3 v3GoaliePos = GetPosition();
    v3GoaliePos.f.z = 0.0f;

    cNet* pNet = pGoalie->m_pTeam->m_pNet;
    nlVector3 v3PostPos;
    nlVector3 v3PrevHeadJointPos = pGoalie->GetPrevJointPosition(pGoalie->m_nHeadJointIndex);

    if (v3PrevHeadJointPos.f.y > 0.0f)
    {
        pNet->GetPostLocation(v3PostPos, 1, 0.0f);
    }
    else
    {
        pNet->GetPostLocation(v3PostPos, 0, 0.0f);
    }

    float postToHeadY = v3PostPos.f.y - v3PrevHeadJointPos.f.y;
    float postToHeadYSq = postToHeadY * postToHeadY;
    float postToHeadX = v3PostPos.f.x - v3PrevHeadJointPos.f.x;
    float postToHeadDistSq = postToHeadYSq + (postToHeadX * postToHeadX);

    if (postToHeadDistSq < 4.0f)
    {
        float postRadius = cNet::m_fNetPostRadius;
        float headDistLimitSq = (1.0f + postRadius) * (1.0f + postRadius);

        float fJointRadius[3] = { 0.15f, 0.2f, 0.2f };
        nlVector3 v3JointPos[3];

        v3JointPos[0] = pGoalie->GetJointPosition(pGoalie->m_nHeadJointIndex);
        v3JointPos[1] = pGoalie->GetJointPosition(pGoalie->m_nRightHandJointIndex);
        v3JointPos[2] = pGoalie->GetJointPosition(pGoalie->m_nLeftHandJointIndex);

        float fSin;
        float fCos;
        nlSinCos(&fSin, &fCos, pGoalie->m_aActualFacingDirection);

        float onePlusPostRadius = 1.0f + postRadius;

        u8 bMoved = 0;
        for (int i = 0; i < 3; i++)
        {
            float x = v3JointPos[i].f.x;
            float y = v3JointPos[i].f.y;

            nlVector3 v3JointWorldPos;
            v3JointWorldPos.f.y = v3GoaliePos.f.y + ((fCos * y) + (fSin * x));
            v3JointWorldPos.f.x = v3GoaliePos.f.x + ((fCos * x) - (fSin * y));
            v3JointWorldPos.f.z = v3PostPos.f.z;

            float postToJointY = v3PostPos.f.y - v3JointWorldPos.f.y;
            float postToJointX = v3PostPos.f.x - v3JointWorldPos.f.x;
            float jointDistSq = (postToJointY * postToJointY) + (postToJointX * postToJointX);
            float fMinDist = postRadius + fJointRadius[i];

            if (i == 0)
            {
                if (jointDistSq < headDistLimitSq)
                {
                    nlVector3 v3Norm;
                    v3Norm.f.x = v3JointWorldPos.f.y - v3PrevHeadJointPos.f.y;
                    v3Norm.f.y = v3PrevHeadJointPos.f.x - v3JointWorldPos.f.x;
                    v3Norm.f.z = 0.0f;

                    if ((v3PostPos.f.x * v3Norm.f.x) < 0.0f)
                    {
                        v3Norm.f.x = v3Norm.f.x * -1.0f;
                        v3Norm.f.y = v3Norm.f.y * -1.0f;
                    }

                    nlVector4 v4Plane;
                    MakePerpendicularPlane(v3JointWorldPos, v3Norm, v4Plane, 0.0f);

                    float fCurDist = ((v3PostPos.f.y * v4Plane.f.y) + (v3PostPos.f.x * v4Plane.f.x) + (v3PostPos.f.z * v4Plane.f.z)) - v4Plane.f.w;
                    float fCurDistAbs = (float)fabs(fCurDist);

                    if (fCurDistAbs < fMinDist)
                    {
                        float fMoveDist = InterpolateRangeClamped(0.0f, fMinDist - fCurDistAbs, onePlusPostRadius, 0.0f, nlSqrt(jointDistSq, true));

                        if ((fCurDist > 0.0f) || m_CanCollidedWithGoalLine)
                        {
                            fMoveDist = fMoveDist * -1.0f;
                        }

                        v3GoaliePos.f.z = v3GoaliePos.f.z + (fMoveDist * v4Plane.f.z);
                        v3GoaliePos.f.y = v3GoaliePos.f.y + (fMoveDist * v4Plane.f.y);
                        v3GoaliePos.f.x = v3GoaliePos.f.x + (fMoveDist * v4Plane.f.x);
                        bMoved = 1;
                    }
                }
            }
            else
            {
                if (jointDistSq < (fMinDist * fMinDist))
                {
                    float fJointDistY = v3JointWorldPos.f.y - v3PostPos.f.y;
                    float fJointDistX = v3JointWorldPos.f.x - v3PostPos.f.x;
                    float fJointDistZ = v3JointWorldPos.f.z - v3PostPos.f.z;

                    if (jointDistSq > 0.00001f)
                    {
                        float fJointDist = nlSqrt(jointDistSq, true);
                        float fScale = (fMinDist - fJointDist) / fJointDist;

                        v3GoaliePos.f.z = v3GoaliePos.f.z + (fScale * fJointDistZ);
                        v3GoaliePos.f.x = v3GoaliePos.f.x + (fScale * fJointDistX);
                        v3GoaliePos.f.y = v3GoaliePos.f.y + (fScale * fJointDistY);
                        bMoved = 1;
                    }
                }
            }
        }

        if (bMoved)
        {
            SetCharacterPosition(v3GoaliePos);
        }
    }
}
