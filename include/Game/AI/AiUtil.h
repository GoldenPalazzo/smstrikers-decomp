#ifndef _AIUTIL_H_
#define _AIUTIL_H_

#include "NL/nlMath.h"

nlVector3 GetClosestPointOnSidelines(const nlVector3&);
char* GetPowerupName(int powerup);
void SortToMinOrMaxTotalSum(unsigned int*, float (*)[4], bool);
nlVector3 GetClosestPointOnLineABFromPointC(const nlVector3&, const nlVector3&, const nlVector3&);
float AIsgn(float fValue);
float NormalizeVal(float fromVal, float fromMin, float fromMax);
float NormalizeVal(float fromVal, const nlVector2& fromExtrema);
float InterpolateRangeClamped(const nlVector2&, const nlVector2&, float);
float InterpolateRangeClamped(float, float, float, float, float);
float Interpolate(float fMin, float fMax, float fPercent);
void RotateVector(nlVector3&, const nlVector3&, nlQuaternion&);
void GetRotationBetweenVectors(nlQuaternion&, const nlVector3&, const nlVector3&);
void RotateVectorZAxis(nlVector3&, const nlVector3&, unsigned short);
void GetWorldPoint(nlVector3& v3WorldPointOut, const nlVector3& v3LocalPointIn, const nlVector3& v3RefPosition, unsigned short aRefAngle);
void GetLocalPoint(nlVector3& v3LocalPointOut, const nlVector3& v3WorldPointIn, const nlVector3& v3RefPosition, unsigned short aRefAngle);
float GetClosingSpeed(const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&);
float GetClosingSpeed2D(const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&);
float Exp(float);
bool TestCollision(float rp, const nlVector3& p1, const nlVector3& p2, float rq, const nlVector3& q1, const nlVector3& q2);
bool ClipPositionToSidelines(nlVector3&, float);
void CalcInterceptXY(const nlVector3&, float, float, const nlVector3&, const nlVector3&, int&, float*);
float SeekSpeed(float fCurrent, float fDesired, float fSeekAccel, float fSeekDecel, float fDeltaT);
unsigned short SeekDirection(unsigned short, unsigned short, float, float, float);
bool IsPointInCone(const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&);
void MakePerpendicularPlane(const nlVector3&, const nlVector3&, nlVector4&, float);
void MakePerpendicularPlane(const nlVector3&, unsigned short, nlVector4&, float);

#endif // _AIUTIL_H_
