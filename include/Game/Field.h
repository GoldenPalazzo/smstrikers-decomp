#ifndef _FIELD_H_
#define _FIELD_H_

#include "NL/nlMath.h"
#include "Game/Net.h"
#include "Game/Ball.h"

struct sCornerSegment
{
    /* 0x0 */ nlVector2 vCenter;
    /* 0x8 */ u16 thetaStart;
    /* 0xA */ u16 thetaEnd;
    /* 0xC */ f32 fRadius;
}; // total size: 0x10

struct sSideLinePlane
{
    /* 0x00 */ nlVector2 vNormal;
    /* 0x08 */ float fDistance;
}; // total size: 0xC

class cField
{
public:
    static void Init(cNet*, cNet*);
    static float GetGoalLineX(float);
    static float GetGoalLineX(unsigned int);
    static float GetSidelineY(unsigned int);
    static float GetCornerRadius();
    static float GetPenaltyBoxX(unsigned int);
    static float GetPenaltyBoxY();
    static cNet* GetNet(float);
    static bool IsOnField(const nlVector3&);
    static void FixOutOfBoundsPosition(nlVector3&, float);
    static void SetFieldDimensions(float, float, float);

    static inline const nlVector3& GetFieldPosition()
    {
        return mv3FieldPosition;
    }

    static nlVector3 mv3FieldPosition;
    static sSideLinePlane mSidelines[4];
    static sCornerSegment mCorners[4];
    // static Corner mCorners;
    static float mfPenaltyBoxX;
    static float mfPenaltyBoxY;
    static cNet* mpNet[2];
};

#endif // _FIELD_H_
