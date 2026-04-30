#ifndef _DRAWABLECHARACTER_H_
#define _DRAWABLECHARACTER_H_

#include "Game/Replay.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/CharacterEffects.h"
#include "Game/Render/Bowser.h"

void DrawableCharacterHeadTrackCallback(unsigned int, unsigned int, cPoseAccumulator*, unsigned int, int);

class cCharacter;
class cPoseNode;
class SkinAnimatedMovableNPC;
class SkinAnimatedMovableNPC;
class cPoseAccumulator;

class LoadFrame;
class LoadFrame;

class DrawableCharacter
{
public:
    template <typename T>
    void Replay(T&);

    DrawableCharacter();
    ~DrawableCharacter();

    void Free();
    cPN_SAnimController& GetAnimController() const;
    void Grab(cCharacter&);
    static void DrawableBowserHeadTrackCallback(unsigned int, unsigned int, cPoseAccumulator*, unsigned int, int);
    void BuildNodeMatrices();
    void Render(cCharacter&) const;
    void SendToGl(const cCharacter&) const;
    void Grab(SkinAnimatedMovableNPC& character);
    void Render(SkinAnimatedMovableNPC& character) const;
    void Blend(const float*, const DrawableCharacter&, const DrawableCharacter&);
    void EvaluateFrom(const cPoseNode&, const nlVector3&, unsigned short);
    nlVector3 GetBallPosition() const;
    nlQuaternion GetBallOrientation() const;

    static void RenderOnlyOneCharacter(const cCharacter&, bool);
    static void RenderAllCharacters();
    static cCharacter* OnlyRenderingOneCharacter();

    /* 0x0, */ bool mVisible;                       // offset 0x0, size 0x1
    /* 0x4, */ nlVector3 mPosition;                 // offset 0x4, size 0xC
    /* 0x10 */ nlVector3 mBip01Position;            // offset 0x10, size 0xC
    /* 0x1C */ nlVector3 mHeadPosition;             // offset 0x1C, size 0xC
    /* 0x28 */ float mHeight;                       // offset 0x28, size 0x4
    /* 0x2C */ nlVector3 mVelocity;                 // offset 0x2C, size 0xC
    /* 0x38 */ unsigned short mFacingDirection;     // offset 0x38, size 0x2
    /* 0x3A */ unsigned short mHeadSpin;            // offset 0x3A, size 0x2
    /* 0x3C */ unsigned short mHeadTilt;            // offset 0x3C, size 0x2
    /* 0x40 */ cPoseNode* mPoseTree;                // offset 0x40, size 0x4
    /* 0x44 */ cPoseAccumulator* mPoseAccumulator;  // offset 0x44, size 0x4
    /* 0x48 */ EffectsTexturing* mEffectsTexturing; // offset 0x48, size 0x4
    /* 0x4C */ cCharacter* mCharacter;              // offset 0x4C, size 0x4
    /* 0x50 */ Bowser* mBowser;                     // offset 0x50, size 0x4
    /* 0x54 */ unsigned char mDirt;                 // offset 0x54, size 0x1

    static bool sCameraRelativeLighting;
    static cCharacter* spRenderOnlyThisCharacter;
    static bool sbRenderOpposingGoalieToo;

}; // total size: 0x58

template <>
void DrawableCharacter::Replay<SaveFrame>(SaveFrame&);

template <>
void DrawableCharacter::Replay<LoadFrame>(LoadFrame&);

inline SkinAnimatedNPC_Type SkinAnimatedNPC::GetSkinAnimatedNPC_Type() const
{
    return SkinAnimatedNPC_BASE;
}
inline float SkinAnimatedMovableNPC::GetHeadSpin() const
{
    return 0.0f;
}
inline float SkinAnimatedMovableNPC::GetHeadTilt() const
{
    return 0.0f;
}

// class cPoseAccumulator
// {
// public:
//     void operator=(const cPoseAccumulator&);
//     cPoseAccumulator(const cPoseAccumulator&);
// };

// class cBuildNodeMatrixCallbackInfo
// {
// public:
//     cBuildNodeMatrixCallbackInfo();
// };

// class SkinAnimatedMovableNPC
// {
// public:
//     void GetHeadTilt() const;
//     void GetHeadSpin() const;
// };

// class SkinAnimatedNPC
// {
// public:
//     void GetSkinAnimatedNPC_Type() const;
// };

// class LoadFrame
// {
// public:
//     void ReplayablePolymorphicPtr<1, cPoseNode>(cPoseNode*&);
//     void ReplayablePolymorphicPtr<0, cPoseNode>(cPoseNode*&);
// };

// class cPN_SingleAxisBlender
// {
// public:
//     void Replay<LoadFrame>(LoadFrame&);
//     void Replay<SaveFrame>(SaveFrame&);
// };

// class cPN_SAnimController
// {
// public:
//     void Replay<LoadFrame>(LoadFrame&);
//     void Replay<SaveFrame>(SaveFrame&);
// };

// class cPN_Feather
// {
// public:
//     void Replay<LoadFrame>(LoadFrame&);
//     void Replay<SaveFrame>(SaveFrame&);
// };

// class cPN_Blender
// {
// public:
//     void Replay<LoadFrame>(LoadFrame&);
//     void Replay<SaveFrame>(SaveFrame&);
// };

// class FloatCompressor<0, 1, 15>
// {
// public:
//     void FloatCompressor(float&);
// };

// class FloatCompressor<0, 1, 7>
// {
// public:
//     void FloatCompressor(float&);
// };

#endif // _DRAWABLECHARACTER_H_
