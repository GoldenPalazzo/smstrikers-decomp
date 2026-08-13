#include "Game/Physics/PhysicsGoalie.h"
#include "Game/Physics/PhysicsBall.h"
#include "Game/Player.h"
#include "Game/Team.h"
#include "Game/AI/AiUtil.h"
#include "Game/FixedUpdateTask.h"
#include "NL/utility.h"
#include "types.h"

static f32 CANT_COLLIDE = *(f32*)__float_max;

static float ballMaxMotionPerTick = PhysicsBall::GetBallMaxVelocity() * FixedUpdateTask::GetPhysicsUpdateTick();

/**
 * Offset/Address/Size: 0x8F0 | 0x8013A370 | size: 0x34
 */
void PhysicsGoalie::PostUpdate()
{
    PhysicsCharacter::PostUpdate();
    CollideGoalieWithPost();
}

/**
 * Offset/Address/Size: 0x780 | 0x8013A200 | size: 0x170
 */
bool PhysicsGoalie::SweepTestForBallContact(const nlVector3& ballPrevPosition, const nlVector3& ballCurrentPosition, const nlVector3& velocity, float ballRadius, nlVector3& positionWhenHit, nlVector3& contactNormal) const
{
    int testsPassed = 0;
    float goalieRadius = 4.0f * m_CentreOfMassHeight;

    nlVector3 goaliePos;
    GetPosition(&goaliePos);
    goaliePos.f.z = (2.0 * m_CentreOfMassHeight) + goaliePos.f.z;

    nlVector3 ballToGoalie;
    nlVec3Set(ballToGoalie, ballPrevPosition.f.x - goaliePos.f.x, ballPrevPosition.f.y - goaliePos.f.y, ballPrevPosition.f.z - goaliePos.f.z);

    if ((nlSqrt((ballToGoalie.f.x * ballToGoalie.f.x) + (ballToGoalie.f.y * ballToGoalie.f.y) + (ballToGoalie.f.z * ballToGoalie.f.z), true) - (goalieRadius + (ballRadius + ballMaxMotionPerTick))) <= 0.0f)
    {
        testsPassed = 1;
        float sweepResult = SweepSpheres(ballRadius, ballPrevPosition, ballCurrentPosition, goalieRadius, goaliePos, goaliePos);

        bool isValidSweep = false;
        if ((sweepResult == CANT_COLLIDE) || (sweepResult < 0.0f) || (sweepResult > 1.0f))
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
            if (SweepTestEveryBone(ballRadius, ballPrevPosition, ballCurrentPosition, contactNormal, positionWhenHit) != 0)
            {
                testsPassed = 3;
            }
        }
    }

    return testsPassed == 3;
}

/**
 * Offset/Address/Size: 0x4A8 | 0x80139F28 | size: 0x2D8
 */
bool PhysicsGoalie::SweepTestEveryBone(float ballRadius, const nlVector3& ballPrevPosition, const nlVector3& ballCurrentPosition, nlVector3& contactNormal, nlVector3& positionWhenHit) const
{
    nlVector3 normalAccumulator = { 0.0f, 0.0f, 0.0f };
    float cantCollide;
    float smallestTime = 99999.0f;

    nlListConstIterator<PhysicsBoneVolume*> boneVolumeIterator = m_BoneVolumes.Begin();
    PhysicsBoneVolume* boneVolume;
    bool detectedContact = false;
    int numContactsDetected = 0;
    if (!boneVolumeIterator.IsValid())
    {
        return false;
    }

    cantCollide = CANT_COLLIDE;
    while (boneVolumeIterator.IsValid())
    {
        boneVolume = boneVolumeIterator.Current();
        PhysicsSphere* physSphere = (PhysicsSphere*)boneVolume->m_pObject;
        const nlVector3& boneCurrentPosition = physSphere->GetPosition();
        const nlVector3& bonePreviousPosition = boneVolume->m_PrevPosition;
        float time = SweepSpheres(
            ballRadius,
            ballPrevPosition,
            ballCurrentPosition,
            physSphere->GetRadius(),
            bonePreviousPosition,
            boneCurrentPosition);

        if ((time != cantCollide) && (time > 0.0f) && (time < 1.0f))
        {
            if (time < smallestTime)
            {
                smallestTime = time;
            }

            const float oneMinusTime = 1.0f - time;

            nlVec3WeightedSum(positionWhenHit, oneMinusTime, ballPrevPosition, time, ballCurrentPosition);

            nlVec3Set(contactNormal,
                positionWhenHit.f.x - ((oneMinusTime * bonePreviousPosition.f.x) + (time * boneCurrentPosition.f.x)),
                positionWhenHit.f.y - ((oneMinusTime * bonePreviousPosition.f.y) + (time * boneCurrentPosition.f.y)),
                positionWhenHit.f.z - ((oneMinusTime * bonePreviousPosition.f.z) + (time * boneCurrentPosition.f.z)));

            float contactNormalLengthSq = contactNormal.GetLengthSq3D();
            nlVec3Scale(contactNormal, nlRecipSqrt(contactNormalLengthSq, true));

            detectedContact = true;
            numContactsDetected += 1;

            nlVec3Add(normalAccumulator, normalAccumulator, contactNormal);
        }

        boneVolumeIterator.Next();
    }

    if (detectedContact)
    {
        float oneMinusTime = 1.0f - smallestTime;
        float inverseContactCount = 1.0f / (float)numContactsDetected;

        nlVec3Scale(contactNormal, normalAccumulator, inverseContactCount);
        nlVec3WeightedSum(positionWhenHit, oneMinusTime, ballPrevPosition, smallestTime, ballCurrentPosition);
    }

    return detectedContact;
}

/**
 * Offset/Address/Size: 0x70 | 0x80139AF0 | size: 0x438
 */
void PhysicsGoalie::CollideGoalieWithPost()
{
    Goalie* pGoalie = (Goalie*)m_pAICharacter;
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

    if (v3PostPos.CalculateDistanceSquared2D(v3PrevHeadJointPos) < 4.0f)
    {
        float fJointRadius[3] = { 0.15f, 0.2f, 0.2f };
        float postRadius = cNet::m_fNetPostRadius;
        float headDistLimitSq = nlGetLengthSquared1D(1.0f + postRadius);

        nlVector3 v3JointPos[3];

        v3JointPos[0] = pGoalie->GetJointPosition(pGoalie->m_nHeadJointIndex);
        v3JointPos[1] = pGoalie->GetJointPosition(pGoalie->m_nRightHandJointIndex);
        v3JointPos[2] = pGoalie->GetJointPosition(pGoalie->m_nLeftHandJointIndex);

        float fSin;
        float fCos;
        nlSinCos(&fSin, &fCos, pGoalie->m_aActualFacingDirection);

        nlVector3* pJointPos = v3JointPos;
        float* pJointRadius = fJointRadius;
        u8 bMoved = 0;

        for (int i = 0; i < 3; i++, pJointPos++, pJointRadius++)
        {
            nlVector3 v3JointWorldPos;
            float x = pJointPos->f.x;
            float y = pJointPos->f.y;

            v3JointWorldPos.f.x = v3GoaliePos.f.x + ((fCos * x) - (fSin * y));
            v3JointWorldPos.f.y = v3GoaliePos.f.y + ((fCos * y) + (fSin * x));
            v3JointWorldPos.f.z = v3PostPos.f.z;

            nlVector3 v3PostToJoint;
            nlVec3Sub(v3PostToJoint, v3PostPos, v3JointWorldPos);
            float jointDistSq = v3PostToJoint.GetLengthSq2D();
            float fMinDist = postRadius + (*pJointRadius);

            if (i == 0)
            {
                if (jointDistSq < headDistLimitSq)
                {
                    nlVector3 v3Norm;
                    nlVec3Set(v3Norm, v3JointWorldPos.f.y - v3PrevHeadJointPos.f.y, v3PrevHeadJointPos.f.x - v3JointWorldPos.f.x, 0.0f);

                    if ((v3PostPos.f.x * v3Norm.f.x) < 0.0f)
                    {
                        v3Norm.f.x *= -1.0f;
                        v3Norm.f.y *= -1.0f;
                    }

                    nlVector4 v4Plane;
                    MakePerpendicularPlane(v3JointWorldPos, v3Norm, v4Plane, 0.0f);

                    float fCurDist = ((v3PostPos.f.x * v4Plane.f.x) + (v3PostPos.f.y * v4Plane.f.y) + (v3PostPos.f.z * v4Plane.f.z)) - v4Plane.f.w;
                    float fCurDistAbs = (float)fabs(fCurDist);

                    if (fCurDistAbs < fMinDist)
                    {
                        float fJointDist = nlSqrt(jointDistSq, true);
                        float fMoveDist = InterpolateRangeClamped(0.0f, fMinDist - fCurDistAbs, 1.0f + postRadius, 0.0f, fJointDist);

                        if ((fCurDist > 0.0f) || m_CanCollidedWithGoalLine)
                        {
                            fMoveDist *= -1.0f;
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
                    nlVector3 v3JointDist;
                    nlVec3Sub(v3JointDist, v3JointWorldPos, v3PostPos);

                    if (jointDistSq > 0.00001f)
                    {
                        float fCurDist = nlSqrt(jointDistSq, true);
                        float fScale = (fMinDist - fCurDist) / fCurDist;

                        nlVec3ScaleAdd(v3GoaliePos, fScale, v3JointDist, v3GoaliePos);
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
